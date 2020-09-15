add_executable(fpmas-sir-micropop
	main.cpp
	)
target_link_libraries(fpmas-sir-micropop fpmas MPI::MPI_CXX zoltan)
