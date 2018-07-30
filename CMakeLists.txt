cmake_minimum_required( VERSION 3.25 )
project( ase )

# Determine if we're built as a subproject (using add_subdirectory)
# or if this is the master project.
set(MASTER_PROJECT OFF)
if( CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR )
	set(MASTER_PROJECT ON)
endif()

### Standard
set( CMAKE_CXX_STANDARD 20 )
set( CMAKE_CXX_STANDARD_REQUIRED ON )
set( CMAKE_CXX_EXTENSIONS OFF )

### Verbosity
set( CMAKE_COLOR_MAKEFILE ON )
set( CMAKE_VERBOSE_MAKEFILE ON )
set( CMAKE_EXPORT_COMPILE_COMMANDS ON )

### Optimizations
if( "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang|GNU" )
	add_compile_options( -Wall )
	add_compile_options( -Wextra )
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
	add_compile_options( /W4 )
endif()

### Targets
add_library(
	ase STATIC
	source/ase/ase.cpp
)
target_include_directories(
	ase
	PUBLIC
	include
)

### Tests
if( MASTER_PROJECT )
	enable_testing()

	# Copy Sample swatch file to build folder
	file(
		COPY test/SampleSwatch.ase
		DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
	)

	add_executable(
		Parse
		test/Parse.cpp
	)
	target_link_libraries(
		Parse
		PRIVATE
		ase
	)

	add_test(
		NAME Parse
		COMMAND Parse SampleSwatch.ase
	)
endif()