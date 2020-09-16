#include "fpmas/model/model.h"
#include "fpmas/model/serializer.h"

namespace macropop {
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

			City(
					const Population& population,
					double g_s, double g_i, double g_r
				)
				: population(population), g_s(g_s), g_i(g_i), g_r(g_r) {}

			void act() override;

			static void to_json(::nlohmann::json& j, const City* city);
			static City* from_json(const ::nlohmann::json& json);
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
			Disease(double alpha, double beta)
				: alpha(alpha), beta(beta) {}

			void act() override;

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
