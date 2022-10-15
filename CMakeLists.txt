cmake_minimum_required(VERSION 3.24)

project(compare_branches)

set(CMAKE_CXX_STANDARD 17)

add_executable(compare_branches main.cpp)

find_package(CURL REQUIRED)

include_directories(${CURL_INCLUDE_DIRS})

target_link_libraries(compare_branches ${CURL_LIBRARIES})

