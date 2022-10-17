cmake_minimum_required(VERSION 3.20)

project(compare_branches)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(branch_operations SHARED src/branch_operations.cpp)
target_include_directories(branch_operations PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(branch_operations ${CURL_LIBRARIES} nlohmann_json::nlohmann_json)

add_executable(compare_branches src/main.cpp)
target_link_libraries(compare_branches branch_operations)

install(TARGETS branch_operations compare_branches
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib)

install(FILES src/branch_operations.hpp DESTINATION include/branch_operations)

