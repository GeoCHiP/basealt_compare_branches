cmake_minimum_required(VERSION 3.20)

project(compare_branches VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 17)

find_package(CURL REQUIRED)
find_package(nlohmann_json REQUIRED)

add_library(branch_operations SHARED src/branch_operations.cpp)
target_include_directories(branch_operations PRIVATE ${CURL_INCLUDE_DIRS})
target_link_libraries(branch_operations ${CURL_LIBRARIES} nlohmann_json::nlohmann_json)

add_executable(compare_branches src/main.cpp)
target_link_libraries(compare_branches branch_operations)

set(BRANCH_OPERATIONS_VERSION_MAJOR 1)
set(BRANCH_OPERATIONS_VERSION_MINOR 0)
set(BRANCH_OPERATIONS_VERSION_PATCH 0)
set(BRANCH_OPERATIONS_VERSION_STRING "${BRANCH_OPERATIONS_VERSION_MAJOR}.${BRANCH_OPERATIONS_VERSION_MINOR}.${BRANCH_OPERATIONS_VERSION_PATCH}")
set_target_properties(branch_operations PROPERTIES VERSION ${BRANCH_OPERATIONS_VERSION_STRING}
                                                   SOVERSION ${BRANCH_OPERATIONS_VERSION_MAJOR})

install(TARGETS branch_operations compare_branches
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib64)

install(FILES src/branch_operations.hpp DESTINATION include/branch_operations)

