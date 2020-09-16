#include "fpmas.h"

namespace micropop {
	FPMAS_DEFINE_GROUPS(AGENT, CLUSTER);
	FPMAS_DEFINE_LAYERS(AGENT_TO_AGENT, AGENT_TO_CLUSTER);

	enum State {
		S, I, R
	};

	NLOHMANN_JSON_SERIALIZE_ENUM(State, {
			{S, "S"},
			{I, "I"},
			{R, "R"}
			});

	class Agent : public fpmas::model::AgentBase<Agent> {
		private:
			fpmas::random::UniformRealDistribution<float> remission {0, 1};
			fpmas::random::UniformRealDistribution<float> infection {0, 1};

			State state;

		public:
			Agent(State state)
				: state(state) {}

			State getState() const;
			void setState(State state);

			void act() override;

			static void to_json(nlohmann::json&, const Agent*);
			static Agent* from_json(const nlohmann::json&);
	};

	class Cluster : public fpmas::model::AgentBase<Cluster> {
		public:
			std::size_t k;
			double alpha;
			double beta;

			Cluster(std::size_t k, double alpha, double beta)
				: k(k), alpha(alpha), beta(beta) {}

			void act() override;

			static void to_json(nlohmann::json&, const Cluster*);
			static Cluster* from_json(const nlohmann::json&);
	};
}
