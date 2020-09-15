#include "fpmas/model/model.h"
#include "macropop.h"

namespace macropop {
	class GlobalPopulationOutput : public fpmas::api::scheduler::Task {
		private:
			fpmas::api::model::Model& model;
			fpmas::api::communication::MpiCommunicator& comm;
			fpmas::communication::TypedMpi<Population> mpi;
		public:
			GlobalPopulationOutput(
					fpmas::api::model::Model& model,
					fpmas::api::communication::MpiCommunicator& comm);

			void run() override;
	};
}
