find_package(fpmas 1.1 REQUIRED)

add_executable(fpmas-sir-macropop
	macropop.cpp main.cpp output.cpp cli.cpp
	)
target_link_libraries(fpmas-sir-macropop fpmas::fpmas argtable3)
