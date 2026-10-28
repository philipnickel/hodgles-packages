cmake_minimum_required(VERSION 3.20)
project(mfem_hodgles_consumer LANGUAGES CXX)

find_package(MFEM 4.10 REQUIRED CONFIG)

foreach(smoke parmesh_smoke hdf5_smoke)
  add_executable(${smoke} ${smoke}.cpp)
  target_include_directories(${smoke} PRIVATE ${MFEM_INCLUDE_DIRS})
  target_link_libraries(${smoke} PRIVATE ${MFEM_LIBRARIES})
endforeach()
