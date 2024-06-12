
project(ngraph.native LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)

find_package(OpenMP REQUIRED)

add_executable(ngraph.native main.cpp src/layout.cpp src/quadTree.cpp)
target_link_libraries(ngraph.native PUBLIC OpenMP::OpenMP_CXX)
target_include_directories(ngraph.native PUBLIC src)
