#include "fpmas.h"
#include "output.h"
#include "cli.h"
#include "fpmas/random/generator.h"
#include "fpmas/random/distribution.h"
#include "fpmas/graph/graph_builder.h"

#include <chrono>

#ifndef SYNC_MODE
#define SYNC_MODE HardSyncMode
#endif

using fpmas::synchro::HardSyncMode;
using fpmas::synchro::GhostMode;
using namespace macropop;

// Configures json serialization
FPMAS_JSON_SET_UP(City, Disease)

int main(int argc, char** argv) {
	auto begin = std::chrono::system_clock::now();
	int rank;

	// Parses command line arguments
	Config config(argc, argv);

	// Initialize fpmas
	fpmas::init(argc, argv);
	{
		// Register user-defined agent types
		FPMAS_REGISTER_AGENT_TYPES(City, Disease)

		fpmas::model::DefaultModel<SYNC_MODE> model;
		rank = model.getMpiCommunicator().getRank();

		auto& city_group = model.buildGroup(CITY);
		auto& disease_group = model.buildGroup(DISEASE);

		// Initializes the model on proc 0
		FPMAS_ON_PROC(model.getMpiCommunicator(), 0) {
			// Initializes random distribution
			fpmas::random::mt19937 rd;
			fpmas::random::PoissonDistribution<std::size_t> distrib(config.k);

			// Automatic graph builder
			fpmas::graph::RandomDegreeGraphBuilder<fpmas::model::AgentPtr>
				generator {rd, distrib};


			// Agent builder that will build cities
			fpmas::model::AgentNodeBuilder city_builder(city_group);
			for(std::size_t i = 0; i < config.city_count; i++) {
				// Initializes a new city
				city_builder.push(new City(
							{config.average_population, config.initial_infected, 0},
							0.12, 0.12, 0.12));
			}

			// Automatically builds a graph from the cities provided to
			// `city_builder`
			generator.build(city_builder, CITY_TO_CITY, model.graph());

			// Associates a disease to each city
			for(auto city : city_group.agents()) {
				Disease* disease = new Disease(config.alpha, config.beta);
				disease_group.add(disease);
				model.graph().link(
						disease->node(), city->get()->node(), DISEASE_TO_CITY);
			}
		}

		// Executed on all procs

		// Output job
		fpmas::scheduler::Job output_job;
		GlobalPopulationOutput output_task (
				config.output_file, model, model.getMpiCommunicator());
		output_job.add(output_task);

		// Performs load balancing at the beginning of the simulation
		model.scheduler().schedule(0, model.loadBalancingJob());

		// Schedules agents and output jobs
		model.scheduler().schedule(0, 1, city_group.job());
		model.scheduler().schedule(0, 1, disease_group.job());
		model.scheduler().schedule(0, 1, output_job);

		// Runs the model simulation
		model.runtime().run(config.max_step);
	}

	fpmas::finalize();
	auto end = std::chrono::system_clock::now();
	if(rank==0) {
		std::chrono::duration<double> diff = end - start;
		std::cout << "Execution time : " << diff.count() << " s" << std::endl;
	}
}
