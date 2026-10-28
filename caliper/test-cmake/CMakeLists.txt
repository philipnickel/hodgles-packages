cmake_minimum_required(VERSION 3.20)
project(caliper_consumer LANGUAGES CXX)

find_package(caliper REQUIRED CONFIG)

add_executable(region_smoke region_smoke.cpp)
target_link_libraries(region_smoke PRIVATE caliper)
