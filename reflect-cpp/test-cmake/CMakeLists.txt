cmake_minimum_required(VERSION 3.23)
project(reflect_cpp_consumer LANGUAGES CXX)

# Versioned, as the solver asks for it (needs reflectcpp-config-version.cmake).
find_package(reflectcpp 0.26 REQUIRED CONFIG)

add_executable(schema_smoke schema_smoke.cpp)
target_compile_features(schema_smoke PRIVATE cxx_std_20)
target_link_libraries(schema_smoke PRIVATE reflectcpp::reflectcpp)
