cmake_minimum_required(VERSION 3.2.2)

project(libase)

include_directories(include)

file(GLOB_RECURSE SOURCE_FILES source/*.cpp)

set(CMAKE_CXX_STANDARD 11)

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME}
                      PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(${PROJECT_NAME}_INCLUDE_DIRS 
    ${PROJECT_SOURCE_DIR}/ase
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)
