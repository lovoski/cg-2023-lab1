cmake_minimum_required(VERSION 3.20)

project(tests)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR})

find_package(glm CONFIG REQUIRED)

add_executable(test_quat test_quat.cpp)
target_link_libraries(test_quat PRIVATE glm::glm)