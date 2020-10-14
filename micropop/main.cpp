#include "output.h"
#include "fpmas/random/generator.h"
#include "fpmas/random/distribution.h"
#include "fpmas/graph/graph_builder.h"

#ifndef SYNC_MODE
#define SYNC_MODE HardSyncMode
#endif

using fpmas::synchro::HardSyncMode;
using fpmas::synchro::GhostMode;
using namespace micropop;

FPMAS_JSON_SET_UP(Agent, Cluster);

int main(int argc, char** argv) {
	FPMAS_REGISTER_AGENT_TYPES(Agent, Cluster);

	std::size_t agent_count = 1000;
	std::size_t average_neighbors = 15;
	std::size_t init_infected = 10;
	std::size_t k = 5;
	double alpha = .2;
	double beta = .5;
	unsigned long max_step = 100;
	std::string out_file = "micropop.csv";

	fpmas::init(argc, argv);
	{
		fpmas::model::Model<SYNC_MODE> model;

		auto& agent_group = model.buildGroup(AGENT);
		auto& cluster_group = model.buildGroup(CLUSTER);

		FPMAS_ON_PROC(model.getMpiCommunicator(), 0) {
			fpmas::model::AgentNodeBuilder node_builder (agent_group);
			// Initializes infected agents
			std::vector<Agent*> infected_agents;
			for(std::size_t i = 0; i < init_infected; i++) {
				auto* agent = new Agent(I);
				node_builder.push(agent);
				infected_agents.push_back(agent);
			}

			// Initializes susceptible agents
			for(std::size_t i = 0; i < agent_count-init_infected; i++)
				node_builder.push(new Agent(S));

			fpmas::random::mt19937 generator;
			fpmas::random::PoissonDistribution<std::size_t> distribution (average_neighbors);
			fpmas::graph::UniformGraphBuilder<fpmas::api::model::AgentPtr>
				graph_builder (generator, distribution);

			graph_builder.build(node_builder, AGENT_TO_AGENT, model.graph());

			for(auto agent : infected_agents) {
				auto* cluster = new Cluster(k, alpha, beta);
				cluster_group.add(cluster);
				model.graph().link(agent->node(), cluster->node(), AGENT_TO_CLUSTER);
			}
		}

		GlobalPopulationOutput output_task (out_file, model, model.getMpiCommunicator());
		fpmas::scheduler::Job output_job;
		output_job.add(output_task);
		model.scheduler().schedule(0, model.loadBalancingJob());
		model.scheduler().schedule(0, 1, agent_group.job());
		model.scheduler().schedule(0, 1, cluster_group.job());
		model.scheduler().schedule(0, 1, output_job);

		model.runtime().run(max_step);

	}
	fpmas::finalize();
}
