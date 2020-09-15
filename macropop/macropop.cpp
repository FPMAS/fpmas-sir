#include "macropop.h"
#include "fpmas/model/guards.h"

namespace macropop {

	Population operator*(const double& h, const Population& population) {
		return {h * population.S, h * population.I, h * population.R};
	}
	Population operator+(const Population& p1, const Population& p2) {
		return {p1.S + p2.S, p1.I + p2.I, p1.R + p2.R};
	}
	Population operator+=(Population& p, const Population& p2) {
		p.S += p2.S;
		p.I += p2.I;
		p.R += p2.R;
		return p;
	}
	Population operator-=(Population& p, const Population& p2) {
		p.S -= p2.S;
		p.I -= p2.I;
		p.R -= p2.R;
		return p;
	}

	void to_json(nlohmann::json& j, const Population& population) {
		j["S"] = population.S;
		j["I"] = population.I;
		j["R"] = population.R;
	}

	void from_json(const nlohmann::json& j, Population& population) {
		population.S = j.at("S").get<double>();
		population.I = j.at("I").get<double>();
		population.R = j.at("R").get<double>();
	}

	void City::migrate(double m, City* neighbor_city) {
		Population migration;
		{
			fpmas::model::LockGuard lock(this);
			migration = {
				g_s * m * this->population.S,
				g_i * m * this->population.I,
				g_r * m * this->population.R
			};
			this->population -= migration;
		}
		fpmas::model::AcquireGuard acquire(neighbor_city);
		neighbor_city->population += migration;
	}

	void City::act() {
		auto neighbors = outNeighbors<City>(CITY_TO_CITY);
		double m = 1. / neighbors.count();

		for(auto neighbor_city : neighbors) {
			migrate(m, neighbor_city);
		}

		FPMAS_LOGI(this->model()->graph().getMpiCommunicator().getRank(),
				"CITY", "Updated city population : %f",
				this->population.N());
	}

	void City::to_json(::nlohmann::json& j, const City* city) {
		j["pop"] = city->population;
		j["g_s"] = city->g_s;
		j["g_i"] = city->g_i;
		j["g_r"] = city->g_r;
	}

	City* City::from_json(const ::nlohmann::json& json) {
		return new City(
				json.at("pop").get<Population>(),
				json.at("g_s").get<double>(),
				json.at("g_i").get<double>(),
				json.at("g_r").get<double>());
	}

	const double Disease::delta_t {0.1};

	void Disease::act() {
		auto city = *outNeighbors<City>(DISEASE_TO_CITY).begin();

		fpmas::model::AcquireGuard guard (city);

		city->population = RK4::solve(alpha, beta, delta_t, city->population);
		FPMAS_LOGI(this->model()->graph().getMpiCommunicator().getRank(),
				"DISEASE", "Updated city population : %f, %f, %f",
				city->population.S, city->population.I, city->population.R);
	}

	void Disease::to_json(::nlohmann::json& j, const Disease* disease) {
		j["alpha"] = disease->alpha;
		j["beta"] = disease->beta;
	}

	Disease* Disease::from_json(const ::nlohmann::json& json) {
		return new Disease(
			json.at("alpha").get<double>(),
			json.at("beta").get<double>()
		);
	}

	Population RK4::f(double alpha, double beta, const Population& population) {
		double x = beta * population.I * population.S / population.N();
		double y = alpha * population.I;
		return {-x, x - y, y};
	}

	Population RK4::solve(double alpha, double beta, double h, const Population &population) {
		Population k1 = f(alpha, beta, population);
		Population k2 = f(alpha, beta, population + (h / 2) * k1);
		Population k3 = f(alpha, beta, population + (h / 2) * k2);
		Population k4 = f(alpha, beta, population + h * k3);

		return population + (h / 6) * (k1 + 2 * k2 + 2 * k3 + k4);
	}
}
