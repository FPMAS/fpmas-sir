cmake_minimum_required(VERSION 3.10)

project(fpmas-sir-model)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(MPI REQUIRED)

set(CMAKE_CXX_DEBUG_FLAGS "${CMAKE_CXX_DEBUG_FLAGS} -g -O0")

set(SYNC_MODE "HardSyncMode" CACHE STRING "Graph Synchronization mode")
message("-- SYNC_MODE : ${SYNC_MODE}")

set(LOG_LEVEL 0 CACHE STRING "Log level")
add_compile_definitions(LOG_LEVEL=${LOG_LEVEL})

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(fpmas-sir-macropop macropop.cpp main.cpp output.cpp)
target_link_libraries(fpmas-sir-macropop fpmas MPI::MPI_CXX zoltan)
