cmake_minimum_required(VERSION 2.8)

set (CMAKE_CXX_STANDARD 11)
project(openCV)

set(CUDA_USE_STATIC_CUDA_RUNTIME OFF)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

add_executable(openCV openCV.cpp)

target_link_libraries(openCV ${OpenCV_LIBS})
