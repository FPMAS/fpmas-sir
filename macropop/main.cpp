#include "fpmas.h"
#include "output.h"
#include "cli.h"
#include "fpmas/random/generator.h"
#include "fpmas/random/distribution.h"
#include "fpmas/graph/graph_builder.h"

#ifndef SYNC_MODE
#define SYNC_MODE HardSyncMode
#endif

using fpmas::synchro::HardSyncMode;
using fpmas::synchro::GhostMode;
using namespace macropop;

FPMAS_JSON_SET_UP(City, Disease)

int main(int argc, char** argv) {
	fpmas::init(argc, argv);
	{
		Config config(argc, argv);

		FPMAS_REGISTER_AGENT_TYPES(City, Disease)

		fpmas::random::mt19937 rd;
		fpmas::random::PoissonDistribution<std::size_t> distrib(config.k);

		fpmas::model::DefaultModel<SYNC_MODE> model;
		fpmas::graph::FixedDegreeDistributionRandomGraph<fpmas::model::AgentPtr>
			generator {rd, distrib};

		auto& city_group = model.buildGroup(CITY);
		auto& disease_group = model.buildGroup(DISEASE);

		FPMAS_ON_PROC(model.getMpiCommunicator(), 0) {

			fpmas::model::AgentNodeBuilder agent_builder(city_group);
			for(std::size_t i = 0; i < config.city_count; i++)
				agent_builder.push(new City(
							{config.average_population, config.initial_infected, 0},
							0.12, 0.12, 0.12));

			generator.build(agent_builder, CITY_TO_CITY, model.graph());

			for(auto city : city_group.agents()) {
				Disease* disease = new Disease(config.alpha, config.beta);
				disease_group.add(disease);
				model.graph().link(
						disease->node(), city->get()->node(), DISEASE_TO_CITY);
			}
		}

		fpmas::scheduler::Job output_job;
		GlobalPopulationOutput output_task (
				config.output_file, model, model.getMpiCommunicator());
		output_job.add(output_task);
		model.scheduler().schedule(0, model.loadBalancingJob());
		model.scheduler().schedule(0, 1, city_group.job());
		model.scheduler().schedule(0, 1, disease_group.job());
		model.scheduler().schedule(0, 1, output_job);

		model.runtime().run(config.max_step);
	}
	fpmas::finalize();
}
