cmake_minimum_required(VERSION 3.0.0)
project(fatdog 
    VERSION 0.1.0
    DESCRIPTION "A practice for CPP"
    LANGUAGES CXX)

include(CTest)
enable_testing()

# add_library(fatdog fatdog.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
