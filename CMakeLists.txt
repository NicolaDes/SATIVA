cmake_minimum_required (VERSION 3.5.1)

set (PROJECT"CDCL")

project (${PROJECT} CXX)

set (CURRENT_VERSION 1.0)

include (cmake_files/params.cmake)
include (cmake_files/modes.cmake)
configure_file (
	"${PROJECT_SOURCE_DIR}/Config.h.in"
	"${PROJECT_BINARY_DIR}/Config.h"
	)

include (cmake_files/libraries.cmake)

include_directories(include)
add_subdirectory (src)

if(CDCL)
	add_executable (CDCL_test test/CDCL.cc)
	target_link_libraries(CDCL_test SOLVER)
endif()

if(CLI)
	add_executable(CLI_test test/CLI.cc)
	target_link_libraries(CLI_test SOLVER)
endif()

file (COPY "inputs" DESTINATION ".")

