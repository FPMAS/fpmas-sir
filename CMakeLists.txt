cmake_minimum_required(VERSION 3.10)

project(fpmas-sir-model)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(MPI REQUIRED)

set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -g -O0")

set(SYNC_MODE "HardSyncMode" CACHE STRING "Graph Synchronization mode")
message("-- SYNC_MODE : ${SYNC_MODE}")

include_directories(
	${CMAKE_PREFIX_PATH}/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/argtable3/src)

add_subdirectory(argtable3)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/macropop)
