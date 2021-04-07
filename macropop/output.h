#include "fpmas/model/model.h"
#include "fpmas/io/output.h"
#include "fpmas/io/csv_output.h"

#include "macropop.h"

namespace macropop {
	using namespace fpmas::io;
	typedef std::chrono::milliseconds time_unit;

	class ProbeOutput : public FileOutput, public CsvOutput<
						time_unit,
						time_unit,
						time_unit,
						time_unit,
						std::size_t,
						std::size_t>
	{
		public:
			ProbeOutput(std::string file_name, int rank);
	};

	class TimeOutput : public FileOutput, public DistributedCsvOutput<
					   Local<time_unit>,
					   Local<time_unit>,
					   Local<time_unit>,
					   Local<time_unit>,
					   Local<time_unit>
					   >
	{
		public:
			static fpmas::utils::perf::Probe builder_probe;
			static fpmas::utils::perf::Probe link_probe;
			static fpmas::utils::perf::Probe lb_probe;
			static fpmas::utils::perf::Probe init_probe;
			static fpmas::utils::perf::Probe run_probe;
			static fpmas::utils::perf::Monitor monitor;

			TimeOutput(std::string file_name, fpmas::api::communication::MpiCommunicator& comm);
	};

	class LbOutput : public FileOutput, public CsvOutput<
					 std::size_t,
					 std::size_t,
					 std::size_t,
					 std::size_t>
	{
		public:
			LbOutput(
					std::string file_format, int rank,
					fpmas::api::graph::DistributedGraph<fpmas::model::AgentPtr>& graph
					);
	};

	class GlobalPopulationOutput :
		public FileOutput,
		public DistributedCsvOutput<
		Local<fpmas::scheduler::TimeStep>,
		Reduce<double>,
		Reduce<double>,
		Reduce<double>,
		Reduce<double>
		> {
			private:
				fpmas::api::model::Model& model;
				fpmas::scheduler::Date buffer_date {};
				Population total_population_buffer;

				const Population& total_population();

			public:
				GlobalPopulationOutput(
						std::string output_file,
						fpmas::api::model::Model& model,
						fpmas::api::communication::MpiCommunicator& comm);
		};
}
