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
