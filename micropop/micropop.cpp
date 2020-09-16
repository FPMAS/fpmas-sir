#include "micropop.h"

namespace micropop {
	void Agent::act() {
		if(state == I) {
			fpmas::random::random_device rd;

			auto cluster = outNeighbors<Cluster>(AGENT_TO_CLUSTER)[0];
			fpmas::model::ReadGuard read (cluster);

			if(remission(rd) < cluster->alpha) {
				this->state = R;
				return;
			}
			fpmas::random::PoissonDistribution<std::size_t> contacts (cluster->beta);
			auto neighbors = outNeighbors<Agent>(AGENT_TO_AGENT).shuffle();
			std::vector<fpmas::model::Neighbor<Agent>> infected;
			for(auto i = 0; i < contacts(rd); i++) {
				auto neighbor = neighbors[i];

				if(infection(rd) < cluster->beta / cluster->k) {
					fpmas::model::AcquireGuard acquire (neighbor);
					if(neighbor->getState() == S) {
						neighbor->setState(I);
						infected.push_back(neighbor);
					}
					for(auto neighbor : infected)
						this->model()->graph().link(
								neighbor->node(), cluster->node(),
								AGENT_TO_CLUSTER);
				}
			}
		}
	}

	State Agent::getState() const {
		return state;
	}
	
	void Agent::setState(State state) {
		this->state = state;
	}

	void Agent::to_json(nlohmann::json& j, const Agent* agent) {
		j["s"] = agent->state;
	}

	Agent* Agent::from_json(const nlohmann::json& j) {
		return new Agent(j.at("s").get<State>());
	}

	void Cluster::act() {
		// Die is no more infected
	}

	void Cluster::to_json(nlohmann::json & j, const Cluster * cluster) {
		j["k"] = cluster->k;
		j["a"] = cluster->alpha;
		j["b"] = cluster->beta;
	}

	Cluster* Cluster::from_json(const nlohmann::json & j) {
		return new Cluster(
				j.at("k").get<std::size_t>(),
				j.at("a").get<double>(),
				j.at("b").get<double>()
				);
	}
}
