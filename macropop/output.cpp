#include "output.h"
#include <fstream>

namespace macropop {

	ProbeOutput::ProbeOutput(std::string file_name, int rank)
		: FileOutput(file_name, rank), CsvOutput(
				this->file,
				{"behavior_time", [] () {
				return std::chrono::duration_cast<time_unit>(
						City::monitor.totalDuration(City::BEHAVIOR_PROBE));
				}},
				{"comm_time", [] () {
				return std::chrono::duration_cast<time_unit>(
						City::monitor.totalDuration(City::COMM_PROBE));
				}},
				{"distant_comm_time", [] () {
				return std::chrono::duration_cast<time_unit>(
						City::monitor.totalDuration(City::DISTANT_COMM_PROBE));
				}},
				{"sync_time", [] () {
				return std::chrono::duration_cast<time_unit>(
						City::monitor.totalDuration(City::SYNC_PROBE));
				}},
				{"comm_count", [] () {
				return City::monitor.callCount(City::COMM_PROBE);
				}},
				{"distant_comm_count", [] () {
				return City::monitor.callCount(City::DISTANT_COMM_PROBE);
			 }}
	)
	{
	}

	const Population& GlobalPopulationOutput::total_population() {
		if(model.runtime().currentDate() >= buffer_date) {
			buffer_date = model.runtime().currentDate();
			this->total_population_buffer = Population(0, 0, 0);
			auto& city_group = model.getGroup(CITY);
			for(auto city : city_group.localAgents()) {
				this->total_population_buffer += dynamic_cast<City*>(city)->population;
			}
		}
		return this->total_population_buffer;
	}

	GlobalPopulationOutput::GlobalPopulationOutput(
			std::string output_file,
			fpmas::api::model::Model& model,
			fpmas::api::communication::MpiCommunicator& comm) :
		model(model),
		FileOutput(output_file),
		DistributedCsvOutput(comm, 0, this->file,
				{"T", [&model] () {return model.runtime().currentDate();}},
				{"S", [this, &model] () {
				return total_population().S;
				}},
				{"I", [this, &model] () {
				return total_population().I;
				}},
				{"R", [this, &model] () {
				return total_population().R;
				}},
				{"N", [this, &model] () {
				return total_population().N();
				}}) {
		}
}
