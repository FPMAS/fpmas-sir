find_package(fpmas 1.0 REQUIRED)

add_executable(fpmas-sir-micropop
	main.cpp micropop.cpp output.cpp
	)
target_link_libraries(fpmas-sir-micropop fpmas::fpmas argtable3)

target_compile_definitions(fpmas-sir-micropop PUBLIC SYNC_MODE=${SYNC_MODE})
