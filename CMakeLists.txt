cmake_minimum_required (VERSION 2.8.12.2)

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

if(PIG)
	add_executable(PIG_test test/PIG.cc)
	target_link_libraries(PIG_test SOLVER)
endif()

if(CSV)
	add_executable(CSV_test test/CSV.cc)
	target_link_libraries(CSV_test SOLVER)
endif()

add_executable(SATIVA release/SATIVA.cc)
target_link_libraries(SATIVA SOLVER)

file (COPY "inputs" DESTINATION ".")

