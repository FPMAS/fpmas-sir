#include "fpmas.h"
#include "output.h"
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
		FPMAS_REGISTER_AGENT_TYPES(City, Disease)

		static std::size_t city_count = 100;
		fpmas::random::mt19937 rd;
		fpmas::random::PoissonDistribution<std::size_t> distrib(6);

		fpmas::model::DefaultModel<SYNC_MODE> model;
		fpmas::graph::FixedDegreeDistributionRandomGraph<fpmas::model::AgentPtr>
			generator {rd, distrib};

		auto& city_group = model.buildGroup(CITY);
		auto& disease_group = model.buildGroup(DISEASE);

		FPMAS_ON_PROC(model.getMpiCommunicator(), 0) {

			fpmas::model::AgentNodeBuilder agent_builder(city_group);
			for(std::size_t i = 0; i < city_count; i++)
				agent_builder.push(new City({40000, 1, 0}, 0.12, 0.12, 0.12));

			generator.build(agent_builder, CITY_TO_CITY, model.graph());

			for(auto city : city_group.agents()) {
				Disease* disease = new Disease(.1, .7);
				disease_group.add(disease);
				model.graph().link(
						disease->node(), city->get()->node(), DISEASE_TO_CITY);
			}
		}

		fpmas::scheduler::Job output_job;
		GlobalPopulationOutput output_task (model, model.getMpiCommunicator());
		output_job.add(output_task);
		model.scheduler().schedule(0, model.loadBalancingJob());
		model.scheduler().schedule(0, 1, city_group.job());
		model.scheduler().schedule(0, 1, disease_group.job());
		model.scheduler().schedule(0, 1, output_job);

		model.runtime().run(1000);
	}
	fpmas::finalize();
}
