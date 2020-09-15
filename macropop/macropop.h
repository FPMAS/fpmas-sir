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


	class City : public fpmas::model::AgentBase<City> {
		private:
		//void migrate(double coef, double& from, double& to);
		void migrate(double m, City*);

		public:
			Population population;
			double g_s;
			double g_i;
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

	class Disease : public fpmas::model::AgentBase<Disease> {
		private:
			double alpha;
			double beta;
			static const double delta_t;
		public:
			Disease(double alpha, double beta)
				: alpha(alpha), beta(beta) {}

			void act() override;

			static void to_json(::nlohmann::json& j, const Disease* disease);
			static Disease* from_json(const ::nlohmann::json& json);
	};

	class RK4 {
		private:
			static Population f(double alpha, double beta, const Population&);

		public:
			static Population solve(double alpha, double beta, double h, const Population& population);
	};
}
