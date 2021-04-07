#include "fpmas.h"
#include "output.h"
#include "cli.h"
#include "fpmas/random/generator.h"
#include "fpmas/random/distribution.h"
#include "fpmas/graph/graph_builder.h"

using fpmas::synchro::HardSyncMode;
using fpmas::synchro::GhostMode;
using namespace macropop;

// Configures json serialization
FPMAS_JSON_SET_UP(City, Disease)

int main(int argc, char** argv) {
	int rank;

	// Parses command line arguments
	Config config(argc, argv);

	// Initialize fpmas
	fpmas::init(argc, argv);
	{
		TimeOutput::init_probe.start();
		// Register user-defined agent types
		FPMAS_REGISTER_AGENT_TYPES(City, Disease)

			fpmas::api::model::Model* model;
		switch(config.sync_mode) {
			case GHOST:
				model = new fpmas::model::Model<GhostMode>;
				break;
			case HARD_SYNC:
				model = new fpmas::model::Model<HardSyncMode>;
				break;
		}
		rank = model->getMpiCommunicator().getRank();

		fpmas::model::Behavior<City> city_behavior {&City::migrate_population};
		auto& city_group = model->buildGroup(CITY, city_behavior);
		fpmas::model::Behavior<Disease> disease_behavior {&Disease::propagate_virus};
		auto& disease_group = model->buildGroup(DISEASE, disease_behavior);

		GraphSyncProbe graph_sync_probe(model->graph());
		city_group.agentExecutionJob().setEndTask(graph_sync_probe);

		// Model initialization
		{
			TimeOutput::builder_probe.start();

			// Initializes random distribution
			fpmas::random::DistributedGenerator<> rd;
			fpmas::random::PoissonDistribution<std::size_t> edge_distrib(config.k);

			// Agent builder that will build cities
			fpmas::model::DistributedAgentNodeBuilder city_builder(
					city_group, config.city_count,
					[&config] () {return new City(
							{config.average_population, config.initial_infected, 0},
							0.12, 0.12, 0.12);
					},
					[] () {return new City;},
					model->getMpiCommunicator()
					);
			
			switch(config.graph_mode) {
				case UNIFORM:
					{
						FPMAS_LOGI(model->getMpiCommunicator().getRank(), "MACROPOP", "Initializing uniform city graph...");
						// Automatic graph builder
						fpmas::graph::DistributedUniformGraphBuilder<fpmas::model::AgentPtr>
							graph_builder (rd, edge_distrib);

						// Automatically builds a graph using `city_builder` to
						// generate Agents
						graph_builder.build(city_builder, CITY_TO_CITY, model->graph());
						break;
					}

				case CLUSTERED:
					{
						FPMAS_LOGI(model->getMpiCommunicator().getRank(), "MACROPOP", "Initializing clustered city graph...");
						fpmas::random::UniformRealDistribution<double> location_dist(0, 1000);
						fpmas::graph::DistributedClusteredGraphBuilder<fpmas::model::AgentPtr> graph_builder(
								rd, edge_distrib, location_dist, location_dist
								);

						// Automatically builds a graph using `city_builder` to
						// generate Agents
						graph_builder.build(city_builder, CITY_TO_CITY, model->graph());
						break;
					}
			}
			TimeOutput::builder_probe.stop();

			TimeOutput::link_probe.start();
			//Associates a disease to each city
			for(auto city : city_group.localAgents()) {
				Disease* disease = new Disease(config.alpha, config.beta);
				disease_group.add(disease);
				model->link(disease, city, DISEASE_TO_CITY);
			}
			model->graph().synchronizationMode().getSyncLinker().synchronize();
			TimeOutput::link_probe.stop();
		}

		// Output job
		GlobalPopulationOutput model_output (
				config.output_dir + "output.csv", *model, model->getMpiCommunicator());

		fpmas::scheduler::detail::LambdaTask post_lb_task([&config, model] () {
				TimeOutput::lb_probe.stop();
				TimeOutput::init_probe.stop();
				TimeOutput::run_probe.start();
				});
		fpmas::scheduler::Job post_lb_job({post_lb_task});

		//fpmas::communication::MpiCommunicator comm;
		//fpmas::graph::ZoltanLoadBalancing<fpmas::model::AgentPtr> lb(comm);
		//model->graph().balance(lb, {});
		//model->graph().synchronize();

		// Performs load balancing at the beginning of the simulation
		model->scheduler().schedule(0, model->loadBalancingJob());
		model->scheduler().schedule(0.1, post_lb_job);

		// Schedules agents and output jobs
		model->scheduler().schedule(0.2, 1, city_group.jobs());
		model->scheduler().schedule(0.21, 1, disease_group.jobs());
		model->scheduler().schedule(0.22, 1, model_output.job());

		// Runs the model simulation
		TimeOutput::lb_probe.start(); // First task executed
		model->runtime().run(config.max_step);
		TimeOutput::run_probe.stop();

		ProbeOutput perf_output(config.output_dir + "perf.%r.csv", model->getMpiCommunicator().getRank());
		perf_output.dump();

		TimeOutput::monitor.commit(TimeOutput::builder_probe);
		TimeOutput::monitor.commit(TimeOutput::lb_probe);
		TimeOutput::monitor.commit(TimeOutput::link_probe);
		TimeOutput::monitor.commit(TimeOutput::init_probe);
		TimeOutput::monitor.commit(TimeOutput::run_probe);

		TimeOutput(
				config.output_dir + "time.csv",
				model->getMpiCommunicator()
				).dump();

		LbOutput(
				config.output_dir + "lb.%r.csv",
				model->getMpiCommunicator().getRank(), model->graph()
				).dump();
	}

	fpmas::finalize();
}
