cmake_minimum_required(VERSION 2.8)

project(links++)

set(CMAKE_CXX_STANDARD 11)
set(BOOST_ROOT "D:\\Source\\library\\boost_1_65_1")
find_package(OpenMP)
find_package(Boost REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/ SOURCE)

add_executable(${PROJECT_NAME} ${SOURCE})

include_directories(include)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

if (Boost_FOUND)
	include_directories(${Boost_INCLUDE_DIRS})
	target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
endif()
