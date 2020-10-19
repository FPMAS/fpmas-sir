#include "output.h"
#include <fstream>

namespace macropop {

	GlobalPopulationOutput::GlobalPopulationOutput(
			std::string output_file,
			fpmas::api::model::Model& model,
			fpmas::api::communication::MpiCommunicator& comm)
		: output_file(output_file), model(model), comm(comm), mpi(comm) {
			FPMAS_ON_PROC(comm, 0) {
				std::ofstream outfile(output_file, std::ios::out | std::ios::trunc);
				outfile << "T,S,I,R,N" << std::endl;
			}
		}

	void GlobalPopulationOutput::run() {
		Population total_population;
		auto& city_group = model.getGroup(CITY);
		for(auto city : city_group.localAgents()) {
			total_population += static_cast<City*>(city)->population;
		}

		FPMAS_ON_PROC(comm, 0) {
			std::ofstream outfile(output_file, std::ios::out | std::ios::app);

			auto total_population_vec = mpi.gather(total_population, 0);

			Population total_sum = std::accumulate(
					total_population_vec.begin(),
					total_population_vec.end(),
					Population());

			outfile <<
				model.runtime().currentDate() << "," <<
				total_sum.S << "," <<
				total_sum.I << "," <<
				total_sum.R << "," <<
				total_sum.S + total_sum.I + total_sum.R << std::endl;
		} else {
			mpi.gather(total_population, 0);
		}
	}
}
