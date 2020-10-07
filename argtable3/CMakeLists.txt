# Configures argtable3-download project
# (implicitely creates argtable3-download directory)
configure_file(CMakeLists.txt.in argtable3-download/CMakeLists.txt)

# Configures and build argtable3-download at global project configuration
execute_process(
	COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/argtable3-download)
# Argtable3 sources are extracted in the argtable3 dir
execute_process(
	COMMAND ${CMAKE_COMMAND} --build .
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/argtable3-download)

add_library(argtable3 src/argtable3.c)
