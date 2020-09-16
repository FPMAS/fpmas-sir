#include "output.h"
#include <fstream>

namespace micropop {

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
		std::array<std::size_t, 3> total_population {0, 0, 0};
		auto& agent_group = model.getGroup(AGENT);
		for(auto agent : agent_group.localAgents()) {
			fpmas::model::SharedLockGuard lock (*agent);
			switch(static_cast<Agent*>(agent->get())->getState()) {
				case S:
					total_population[0]++;
					break;
				case I:
					total_population[1]++;
					break;
				case R:
					total_population[2]++;
			}
		}

		FPMAS_ON_PROC(comm, 0) {
			std::ofstream outfile(output_file, std::ios::out | std::ios::app);

			auto total_population_vec = mpi.gather(total_population, 0);

			std::array<std::size_t, 3> total_sum {0, 0, 0};
			for(auto array : total_population_vec)
				for(int i = 0; i < 3; i++)
					total_sum[i] += array[i];

			outfile <<
				model.runtime().currentDate() << "," <<
				total_sum[0] << "," <<
				total_sum[1] << "," <<
				total_sum[2] << "," <<
				total_sum[0] + total_sum[1] + total_sum[2] << std::endl;
		} else {
			mpi.gather(total_population, 0);
		}
	}
}
