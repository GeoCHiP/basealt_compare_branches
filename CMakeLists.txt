cmake_minimum_required(VERSION 3.24)

project(compare_branches)

set(CMAKE_CXX_STANDARD 17)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

find_package(CURL REQUIRED)

add_library(branch_operations SHARED src/branch_operations.cpp)
target_include_directories(branch_operations PUBLIC "vendor/json" PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(branch_operations ${CURL_LIBRARIES})

add_executable(compare_branches src/main.cpp)
target_link_libraries(compare_branches branch_operations)

