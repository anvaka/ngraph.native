cmake_minimum_required(VERSION 2.8)

project(links++)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "-Wall -Wextra")
	set(CMAKE_CXX_FLAGS_DEBUG "-g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

set(BOOST_ROOT "D:\\Source\\library\\boost_1_65_1")
set(Boost_USE_STATIC_LIBS FALSE)
set(Boost_USE_STATIC_RUNTIME FALSE)

find_package(OpenMP REQUIRED)
find_package(Boost REQUIRED
	COMPONENTS
	system filesystem
)

FILE(GLOB MY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
FILE(GLOB MY_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h
					 ${CMAKE_CURRENT_SOURCE_DIR}/include/*.inl)

add_executable(${PROJECT_NAME} ${MY_SOURCES} ${MY_HEADERS})

include_directories(include)

if (OPENMP_FOUND)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

if (Boost_FOUND)
	message(status "** Boost Include: ${Boost_INCLUDE_DIR}")
	message(status "** Boost Library Dirs: ${Boost_LIBRARY_DIRS}")
	message(status "** Boost Libraries: ${Boost_LIBRARIES}")
	
	if (WIN32)
	  add_definitions( -DBOOST_ALL_NO_LIB )
	  add_definitions( -DBOOST_ALL_DYN_LINK )
	endif()

	include_directories(${Boost_INCLUDE_DIRS})
	link_libraries(${Boost_LIBRARY_DIRS})
endif()

target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES})
