#include "fpmas/model/model.h"
#include "fpmas/model/serializer.h"
#include "fpmas/utils/perf.h"
#include "fpmas/graph/random_load_balancing.h"
#include "config.h"

namespace macropop {
	template<template<typename> class SyncMode>
		class ModelConfig {
			protected:
				fpmas::model::detail::AgentGraph<SyncMode> graph {fpmas::communication::WORLD};

				fpmas::scheduler::Scheduler scheduler;
				fpmas::runtime::Runtime runtime {scheduler};

				fpmas::graph::ZoltanLoadBalancing<fpmas::model::AgentPtr> zoltan {
					fpmas::communication::WORLD
				};
				fpmas::graph::ScheduledLoadBalancing<fpmas::model::AgentPtr> scheduled_lb {zoltan, scheduler, runtime};
				fpmas::graph::RandomLoadBalancing<fpmas::model::AgentPtr> random_lb {
					fpmas::communication::WORLD
				};

				fpmas::api::graph::LoadBalancing<fpmas::model::AgentPtr>* lb;
				ModelConfig(LbMethod lb_method) {
					switch(lb_method) {
						case ZOLTAN:
							lb = &scheduled_lb;
							break;
						case RANDOM:
							lb = &random_lb;
							break;
					}
				}
		};

	template<template<typename> class SyncMode>
		class Model : private ModelConfig<SyncMode>, public fpmas::model::detail::Model {
			public:
				Model(LbMethod lb) :
					ModelConfig<SyncMode>(lb),
					fpmas::model::detail::Model(
						this->ModelConfig<SyncMode>::graph,
						this->ModelConfig<SyncMode>::scheduler,
						this->ModelConfig<SyncMode>::runtime,
						*this->lb) {}
		};

	FPMAS_DEFINE_GROUPS(CITY,DISEASE);

	FPMAS_DEFINE_LAYERS(CITY_TO_CITY, DISEASE_TO_CITY);

	struct Population {
		double S;
		double I;
		double R;

		Population(double S, double I, double R)
			: S(S), I(I), R(R) {}
		Population()
			: Population(0, 0, 0) {}

		double N() const {
			return S + I + R;
		}
	};

	void to_json(nlohmann::json& j, const Population& population);

	void from_json(const nlohmann::json& j, Population& population);

	Population operator+(const Population& p1, const Population& p2);
	Population operator+=(Population& p, const Population& p2);
	Population operator-=(Population& p, const Population& p2);
	Population operator*(const double& h, const Population& population);


	/**
	 * City Agent.
	 *
	 * Cities are connected to others to migrate their population.
	 */
	class City : public fpmas::model::AgentBase<City> {
		private:
			void migrate(double m, City*);

		public:
			static fpmas::utils::perf::Monitor monitor;
			static std::string BEHAVIOR_PROBE;
			static std::string COMM_PROBE;
			static std::string DISTANT_COMM_PROBE;
			static std::string SYNC_PROBE;
			static fpmas::utils::perf::Probe behavior_probe;
			static fpmas::utils::perf::Probe comm_probe;
			static fpmas::utils::perf::Probe sync_probe;

			/**
			 * Current city population
			 */
			Population population;
			/**
			 * Susceptible people migration rate
			 */
			double g_s;
			/**
			 * Infected people migration rate
			 */
			double g_i;
			/**
			 * Removed people migration rate
			 */
			double g_r;

			/**
			 * Default constructor used for "light_json" edge transmission
			 * optimization.
			 */
			City() {}

			City(
					const Population& population,
					double g_s, double g_i, double g_r
				)
				: population(population), g_s(g_s), g_i(g_i), g_r(g_r) {}

			void migrate_population();

			static void to_json(::nlohmann::json& j, const City* city);
			static City* from_json(const ::nlohmann::json& json);
	};

	class GraphSyncProbe : public fpmas::api::scheduler::Task {
		private:
			fpmas::model::detail::SynchronizeGraphTask sync_graph_task;

		public:
			GraphSyncProbe(fpmas::api::model::AgentGraph& graph)
				: sync_graph_task(graph) {}
			void run() override;
	};

	/**
	 * Disease Agent.
	 *
	 * Each Disease is connected to exactly one City, and each City is
	 * connected to a Disease agent.
	 * The Disease update the S / I / R population of the city according to
	 * epidemiological parameters.
	 */
	 class Disease : public fpmas::model::AgentBase<Disease> {
		private:
			/**
			 * SIR model alpha parameter (remission probability of infected
			 * people)
			 */
			double alpha;
			/**
			 * SIR model beta parameter (average number of people contamined
			 * by ont person at each time step)
			 */
			double beta;
			/**
			 * Integration step
			 */
			static const double delta_t;
		public:
			/**
			 * Default constructor used for "light_json" edge transmission
			 * optimization.
			 */
			Disease() {}

			Disease(double alpha, double beta)
				: alpha(alpha), beta(beta) {}

			void propagate_virus();

			static void to_json(::nlohmann::json& j, const Disease* disease);
			static Disease* from_json(const ::nlohmann::json& json);
	};

	 /**
	  * Runge-Kutta 4 method used to solve the SIR equations system.
	  */
	class RK4 {
		private:
			static Population f(double alpha, double beta, const Population&);

		public:
			/**
			 * Returns the updated population (i.e. with updated S/I/R
			 * populations) according to the SIR equations and the initial
			 * population.
			 *
			 * @param alpha SIR alpha parameter
			 * @param beta SIR beta parameter
			 * @param h integration step
			 */
			static Population solve(double alpha, double beta, double h, const Population& population);
	};
}
