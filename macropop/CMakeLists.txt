add_executable(fpmas-sir-macropop
	macropop.cpp main.cpp output.cpp cli.cpp
	../argtable/argtable3.c)
target_link_libraries(fpmas-sir-macropop fpmas MPI::MPI_CXX zoltan)
