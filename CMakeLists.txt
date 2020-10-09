cmake_minimum_required(VERSION 3.10)

project(fpmas-sir-model)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(MPI REQUIRED)

set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -g -O0")

set(SYNC_MODE "HardSyncMode" CACHE STRING "Graph Synchronization mode")
message("-- SYNC_MODE : ${SYNC_MODE}")

set(LOG_LEVEL 0 CACHE STRING "Log level")

include_directories(
	${CMAKE_PREFIX_PATH}/include
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}/argtable3/src)

find_library(FPMAS fpmas)
if(NOT FPMAS)
	message(FATAL_ERROR "Can't find fpmas library. Make sure to install it, "
	"and eventually provide the installation directory with -DCMAKE_PREFIX_PATH.")
else()
	message(STATUS "Found FPMAS library : ${FPMAS}")
endif()

find_library(ZOLTAN zoltan)
if(NOT ZOLTAN)
	message(FATAL_ERROR "Can't find zoltan library. Make sure to install it, "
	"and eventually provide the installation directory with -DCMAKE_PREFIX_PATH.")
else()
	message(STATUS "Found Zoltan library : ${ZOLTAN}")
endif()


set(DEPENDENCIES 
	${FPMAS}
	MPI::MPI_CXX
	${ZOLTAN}
	argtable3)

add_subdirectory(argtable3)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/macropop)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/micropop)

