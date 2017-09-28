cmake_minimum_required(VERSION 2.8)

project(links++)

set (CMAKE_CXX_STANDARD 11)

find_package(OpenMP)
if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

include_directories(include)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/ SOURCE)

add_executable(${PROJECT_NAME} ${SOURCE})