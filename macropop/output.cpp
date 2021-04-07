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
				}}) {
	}

	fpmas::utils::perf::Probe TimeOutput::builder_probe {"builder"};
	fpmas::utils::perf::Probe TimeOutput::link_probe {"link"};
	fpmas::utils::perf::Probe TimeOutput::lb_probe {"lb"};
	fpmas::utils::perf::Probe TimeOutput::init_probe {"init"};
	fpmas::utils::perf::Probe TimeOutput::run_probe {"run"};
	fpmas::utils::perf::Monitor TimeOutput::monitor;

	TimeOutput::TimeOutput(std::string file_name, fpmas::api::communication::MpiCommunicator& comm)
		: FileOutput(file_name),
		DistributedCsvOutput<Local<time_unit>, Local<time_unit>, Local<time_unit>, Local<time_unit>, Local<time_unit>>(comm, 0, this->file,
				{builder_probe.label(), [this] () {
				return std::chrono::duration_cast<time_unit>(
						monitor.totalDuration(builder_probe.label()));
				}},
				{link_probe.label(), [this] () {
				return std::chrono::duration_cast<time_unit>(
						monitor.totalDuration(link_probe.label())
						);
				}},
				{lb_probe.label(), [this] () {
				return std::chrono::duration_cast<time_unit>(
						monitor.totalDuration(lb_probe.label())
						);
				}},
				{init_probe.label(), [this] () {
				return std::chrono::duration_cast<time_unit>(
						monitor.totalDuration(init_probe.label())
						);
				}},
				{run_probe.label(), [this] () {
				return std::chrono::duration_cast<time_unit>(
						monitor.totalDuration(run_probe.label())
						);
				}}) {
	}

	LbOutput::LbOutput(
			std::string file_format, int rank,
			fpmas::api::graph::DistributedGraph<fpmas::model::AgentPtr>& graph
			)
		: FileOutput(file_format, rank), CsvOutput<std::size_t, std::size_t, std::size_t, std::size_t>(this->file,
				{"LOCAL_NODES", [&graph] () {
				return graph.getLocationManager().getLocalNodes().size();
				}},
				{"DISTANT_NODES", [&graph] () {
				return graph.getLocationManager().getDistantNodes().size();
				}},
				{"DISTANT_CITY_TO_CITY", [&graph] () {
				std::size_t c_to_c = 0;
				for(auto node : graph.getLocationManager().getDistantNodes())
						c_to_c += node.second->getOutgoingEdges(CITY_TO_CITY).size();
				return c_to_c;
				}},
				{"DISTANT_DISEASE_TO_CITY", [&graph] () {
				std::size_t d_to_c = 0;
				for(auto node : graph.getLocationManager().getDistantNodes())
						d_to_c += node.second->getOutgoingEdges(DISEASE_TO_CITY).size();
				return d_to_c;
				}}) {
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
