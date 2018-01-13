cmake_minimum_required (VERSION 3.5.1)

set (PROJECT"CDCL")

project (${PROJECT} CXX)

set (CURRENT_VERSION 0.2)

if (DECAY)
	set (DECAY_VAL ${DECAY})
else()
	set (DECAY_VAL 2)
endif()
if (BONUS)
	set (BONUS_VAL ${BONUS})
else()
	set (BONUS_VAL ${DECAY_VAL})
endif()

set(GRAPHICS_VAL 0)

if (VERBOSE)
	set (VERBOSE_ON 1)
else()
	set (VERBOSE_ON 0)
endif()

if(V)
	set (VERIFICA_ON 1)
else()
	set (VERIFICA_ON 0)
endif()


if(GRAPH)
	message (Inserted binary location for graphics: ${GNUPLOT})
	file (COPY "graphics" DESTINATION ".")
endif()

configure_file (
	"${PROJECT_SOURCE_DIR}/Config.h.in"
	"${PROJECT_BINARY_DIR}/Config.h"
	)

# EXTERNAL LIBRARIES
include (CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
	add_compile_options("-std=c++11")
else()
	message(FATAL_ERROR "Compiler ${CMAKE_CXX_COMPILER} has no C++11 support.")
endif()

if(OMP)
	find_package(OpenMP)
	if (OPENMP_FOUND)
		set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
		set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
	endif()
endif()

# LIBRARIES AND DIRECTORIES OF PROJECT
include_directories ("${PROJECT_BINARY_DIR}")

include_directories ("lib/")
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

