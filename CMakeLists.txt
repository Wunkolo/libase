cmake_minimum_required( VERSION 3.2.2 )
project( ase )

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
	add_compile_options( -O2 )
	add_compile_options( -Wall )
	add_compile_options( -Wextra )
elseif( "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC" )
	add_compile_options( /W4 )
endif()

include_directories( include )

### Targets
add_library(
	ase STATIC
	source/ase/ase.cpp
)

set_target_properties(
	${PROJECT_NAME}
	PROPERTIES POSITION_INDEPENDENT_CODE ON
)

set(
	${PROJECT_NAME}_INCLUDE_DIRS 
    ${PROJECT_SOURCE_DIR}/include
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE
)


### Tests
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
