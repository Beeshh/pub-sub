cmake_minimum_required(VERSION 3.10)
project(SomeIpPubSub)

find_package(Threads REQUIRED)
find_package(vsomeip3 REQUIRED)

include_directories(${VSOMEIP_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(window_ecu window_ecu_main.cpp)
target_link_libraries(window_ecu vsomeip3 Threads::Threads)

add_executable(climate_ecu climate_ecu_main.cpp)
target_link_libraries(climate_ecu vsomeip3 Threads::Threads)
