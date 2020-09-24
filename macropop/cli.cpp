#include "cli.h"

namespace macropop {

	Config::Config(int argc, char** argv) {
		errors = arg_parse(argc, argv, argtable);
		if(help->count > 0) {
			std::cout << "Usage : fpmas-sir-macropop ";
			arg_print_syntax(stdout, argtable, "\n");
			arg_print_glossary(stdout, argtable, "  %-25s %s\n");

			arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
			std::exit(EXIT_SUCCESS);
		}
		if(errors > 0) {
			arg_print_errors(stdout, end, "fpmas-sir-macropop");
			printf("Try 'fpmas-sir-macropop --help' for more information.\n");

			arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
			std::exit(EXIT_FAILURE);
		}


		if(city_count_arg->count > 0)
			city_count = city_count_arg->ival[0];
		if(population_arg->count > 0)
			average_population = population_arg->ival[0];
		if(infected_arg->count > 0)
			initial_infected = infected_arg->ival[0];
		if(alpha_arg->count > 0)
			alpha = alpha_arg->dval[0];
		if(beta_arg->count > 0)
			beta = beta_arg->dval[0];
		if(k_arg->count > 0)
			k = k_arg->ival[0];
		if(mode_arg->count > 0) {
			std::string mode_str(mode_arg->sval[0]);
			if(mode_str == "clustered" || mode_str == "CLUSTERED")
				graph_mode = CLUSTERED;
			else if (mode_str == "uniform" || mode_str == "UNIFORM")
				graph_mode = UNIFORM;
			else {
				std::cout << "Unknown graph mode : " << mode_str << std::endl;
				printf("Try 'fpmas-sir-macropop --help' for more information.\n");

				arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
				std::exit(EXIT_FAILURE);
			}
		}
		if(output_file_arg->count > 0)
			output_file = std::string(output_file_arg->filename[0]);
		if(max_step_arg->count > 0)
			max_step = max_step_arg->ival[0];
	}

}
