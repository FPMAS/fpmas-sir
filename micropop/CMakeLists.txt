add_executable(fpmas-sir-micropop
	main.cpp micropop.cpp output.cpp
	)
target_link_libraries(fpmas-sir-micropop
	${DEPENDENCIES})

target_compile_definitions(fpmas-sir-micropop PUBLIC LOG_LEVEL=${LOG_LEVEL} SYNC_MODE=${SYNC_MODE})
