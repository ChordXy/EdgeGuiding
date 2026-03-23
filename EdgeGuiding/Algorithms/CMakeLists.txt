cmake_minimum_required(VERSION 4.1)
project(Algorithms)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(Demo)
include_directories(TestCode)

add_executable(Algorithms
        Demo/AlgorithmBlock.cpp
        Demo/AlgorithmLine.cpp
        Demo/Detection.h
        TestCode/ColorBlockTracker.cpp
        TestCode/Detection.h
        TestCode/LineTracker.cpp
        TestCode/main.cpp
        alg_detection.h
        AlgorithmBlock.cpp
        AlgorithmLine.cpp)
