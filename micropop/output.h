#include "micropop.h"

namespace micropop {
	class GlobalPopulationOutput : public fpmas::api::scheduler::Task {
		private:
			std::string output_file;
			fpmas::api::model::Model& model;
			fpmas::api::communication::MpiCommunicator& comm;
			fpmas::communication::TypedMpi<std::array<std::size_t, 3>> mpi;
		public:
			GlobalPopulationOutput(
					std::string output_file,
					fpmas::api::model::Model& model,
					fpmas::api::communication::MpiCommunicator& comm);

			void run() override;
	};
}
