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
	${CMAKE_CURRENT_SOURCE_DIR}/argtable)

find_library(FPMAS fpmas)
if(FPMAS-NOTFOUND)
	message(FATAL_ERROR "Can't find fpmas library. Make sure to install it, and
	eventually provides the installation directory with -DCMAKE_PREFIX_PATH.")
else()
	message(STATUS "Found FPMAS library : ${FPMAS}")
endif()

find_library(ZOLTAN zoltan)
if(ZOLTAN-NOTFOUND)
	message(FATAL_ERROR "Can't find zoltan library. Make sure to install it, and
	eventually provides the installation directory with -DCMAKE_PREFIX_PATH.")
else()
	message(STATUS "Found Zoltan library : ${ZOLTAN}")
endif()


set(DEPENDENCIES 
	${FPMAS}
	MPI::MPI_CXX
	${ZOLTAN})

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/macropop)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/micropop)

