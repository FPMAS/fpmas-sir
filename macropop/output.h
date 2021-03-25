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

	class GlobalPopulationOutput : public fpmas::api::scheduler::Task {
		private:
			std::string output_file;
			fpmas::api::model::Model& model;
			fpmas::api::communication::MpiCommunicator& comm;
			fpmas::communication::TypedMpi<Population> mpi;
		public:
			GlobalPopulationOutput(
					std::string output_file,
					fpmas::api::model::Model& model,
					fpmas::api::communication::MpiCommunicator& comm);

			void run() override;
	};
}
