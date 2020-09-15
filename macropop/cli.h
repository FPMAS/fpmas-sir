#include "../argtable/argtable3.h"
#include <iostream>

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
			struct arg_file* output_file_arg
				= arg_filen("o", "output-file", "<file>", 0, 1, "CSV output file (default: 'output.csv')");
			struct arg_int* max_step_arg
				= arg_intn("s", "max-step", "<n>", 0, 1, "Number of time steps to run (default: 1000)");
			struct arg_end* end = arg_end(20);

			void* argtable[10] = {
				help,
				city_count_arg,
				population_arg,
				infected_arg,
				alpha_arg,
				beta_arg,
				k_arg,
				output_file_arg,
				max_step_arg,
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
			std::string output_file = "output.csv";
			int max_step = 1000;

			Config(int argc, char** argv);

			~Config() {
				arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
			}

	};
}
