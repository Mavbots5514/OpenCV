cmake_minimum_required(VERSION 2.8)

set (CMAKE_CXX_STANDARD 11)
project(hello)

set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS} .)

add_executable(cv_hello opencv.cpp PracticalSocket.cpp)

target_link_libraries(cv_hello ${OpenCV_LIBS})
