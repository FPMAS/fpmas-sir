#include "fpmas/model/model.h"
#include "macropop.h"

namespace macropop {
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
