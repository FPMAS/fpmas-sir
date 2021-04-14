#include "argtable3.h"
#include <iostream>
#include "config.h"

namespace macropop {
	class Config {
		private:
			struct arg_lit* help
				= arg_litn("h", "help", 0, 1, "Display help and exit");
			struct arg_int* city_count_arg
				= arg_intn("n", "city-count", "<n>", 0, 1, "Number of city to generate (default: 100)");
			struct arg_int* population_arg
				= arg_intn("p", "population", "<n>", 0, 1, "Average population of each city (default: 40000)");
			struct arg_int* infected_arg
				= arg_intn("i", "infected", "<n>", 0, 1, "Average initial number of infected people by city (default: 1)");
			struct arg_dbl* alpha_arg
				= arg_dbln("a", "alpha", "<f>", 0, 1, "Alpha parameter of the SIR model (default: 0.2)");
			struct arg_dbl* beta_arg
				= arg_dbln("b", "beta", "<f>", 0, 1, "Beta parameter of the SIR model (default: 0.5)");
			struct arg_int* k_arg
				= arg_intn("k", "graph-degree", "<n>", 0, 1, "Average count of target cities of each city (default: 6)");
			struct arg_str* graph_mode_arg
				= arg_strn("m", "graph-mode", "<graph-mode>", 0, 1, "Graph builder mode : 'clustered' or 'uniform' (default: clustered)");
			struct arg_file* output_dir_arg
				= arg_filen("o", "output-dir", "<dir>", 0, 1, "Output directory (default: current directory)");
			struct arg_int* max_step_arg
				= arg_intn("s", "max-step", "<n>", 0, 1, "Number of time steps to run (default: 1000)");
			struct arg_str* sync_mode_arg
				= arg_strn("S", "sync-mode", "<sync-mode>", 0, 1, "Synchronization mode: 'ghost' or 'hard_sync' (default: hard_sync)");
			struct arg_str* lb_method_arg
				= arg_strn("l", "lb-method", "<lb-method>", 0, 1, "Load-balancing method: 'zoltan' or 'random' (default: zoltan)");
			struct arg_end* end = arg_end(20);

			void* argtable[13] = {
				help,
				city_count_arg,
				population_arg,
				infected_arg,
				alpha_arg,
				beta_arg,
				k_arg,
				graph_mode_arg,
				output_dir_arg,
				max_step_arg,
				sync_mode_arg,
				lb_method_arg,
				end
			};


		public:
			int errors;
			std::size_t city_count = 100;
			double average_population = 40000;
			double initial_infected = 1; 
			double alpha = 0.2;
			double beta = 0.5;
			std::size_t k = 6;
			GraphMode graph_mode = CLUSTERED;
			std::string output_dir = "";
			int max_step = 1000;
			SyncMode sync_mode = HARD_SYNC;
			LbMethod lb_method = ZOLTAN;

			Config(int argc, char** argv);

			~Config() {
				arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
			}

	};
}
