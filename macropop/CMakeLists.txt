add_executable(fpmas-sir-macropop
	macropop.cpp main.cpp output.cpp cli.cpp
	../argtable/argtable3.c)
target_link_libraries(fpmas-sir-macropop ${DEPENDENCIES})

target_compile_definitions(fpmas-sir-macropop PUBLIC LOG_LEVEL=${LOG_LEVEL} SYNC_MODE=${SYNC_MODE})
