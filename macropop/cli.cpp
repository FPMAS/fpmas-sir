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
		if(graph_mode_arg->count > 0) {
			std::string mode_str(graph_mode_arg->sval[0]);
			if(mode_str == "clustered" || mode_str == "CLUSTERED")
				graph_mode = CLUSTERED;
			else if (mode_str == "uniform" || mode_str == "UNIFORM")
				graph_mode = UNIFORM;
			else {
				std::cout << "Unknown graph mode: " << mode_str << std::endl;
				printf("Try 'fpmas-sir-macropop --help' for more information.\n");

				arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
				std::exit(EXIT_FAILURE);
			}
		}
		if(output_dir_arg->count > 0)
			output_dir = std::string(output_dir_arg->filename[0]);
		if(max_step_arg->count > 0)
			max_step = max_step_arg->ival[0];
		if(sync_mode_arg->count > 0) {
			std::string mode_str(sync_mode_arg->sval[0]);
			if(mode_str == "ghost" || mode_str == "GHOST")
				sync_mode = GHOST;
			else if (mode_str == "hard_sync" || mode_str == "HARD_SYNC")
				sync_mode = HARD_SYNC;
			else {
				std::cout << "Unknown sync mode: " << mode_str << std::endl;
				printf("Try 'fpmas-sir-macropop --help' for more information.\n");

				arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
				std::exit(EXIT_FAILURE);
			}
		}
		if(lb_method_arg->count > 0) {
			std::string lb_str(lb_method_arg->sval[0]);
			if(lb_str == "zoltan" || lb_str == "ZOLTAN")
				lb_method = ZOLTAN;
			else if (lb_str == "random" || lb_str == "RANDOM")
				lb_method = RANDOM;
			else {
				std::cout << "Unknown LB method: " << lb_str << std::endl;

				printf("Try 'fpmas-sir-macropop --help' for more information.\n");

				arg_freetable(argtable, sizeof(argtable) / sizeof(argtable[0]));
				std::exit(EXIT_FAILURE);
			}
		}
	}
}
