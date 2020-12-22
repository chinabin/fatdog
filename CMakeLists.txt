cmake_minimum_required(VERSION 3.0.0)
project(fatdog 
    VERSION 0.1.0
    DESCRIPTION "A practice for CPP"
    LANGUAGES CXX)
    
# SET(CMAKE_VERBOSE_MAKEFILE ON)      # 显示详细的原始编译信息
# CXXFLAGS 用于 C++ 编译器的选项，$ENV 表示 CMAKE 的环境变量，CMAKE_CXX_FLAGS 指定编译标志
# -rdynamic 用于通知链接器将所有符号添加到动态符号表中
# -o3 将优化程序调到最高，-Wall 显示警告
# gcc 中的有些警告信息可以使用 -Wwarning-name 打开，也可以用 -Wno-warning-name 来关闭，所以
# -Wno-deprecated 表示，使用过时的特性时不要给出警告
# -Werror 把所有警告转换为错误，并在警告发生时终止编译过程
# -Wno-unused-function 表示未使用的函数不要给出警告
# set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -ggdb -std=c++11")

INCLUDE_DIRECTORIES(.)
INCLUDE_DIRECTORIES(/usr/local/include)

LINK_DIRECTORIES(/usr/local/lib)

find_package(yaml-cpp REQUIRED)
find_package(Boost REQUIRED)

include(CTest)
enable_testing()

set(LIB_SRC
    fatdog/macro.h
    fatdog/log.h
    fatdog/log.cpp
    fatdog/config.h
    fatdog/config.cpp
    fatdog/util.h
    fatdog/thread.h
    fatdog/thread.cpp
    fatdog/util.cpp
    fatdog/fiber.h
    fatdog/fiber.cpp
    fatdog/scheduler.h
    fatdog/scheduler.cpp
    fatdog/iomanager.h
    fatdog/iomanager.cpp
    fatdog/timer.h
    fatdog/timer.cpp
    )

set(LIBS
        pthread
        yaml-cpp)

# add_library 将目录中的源文件生成链接库
ADD_LIBRARY(fatdog SHARED ${LIB_SRC})

# add_library(fatdog fatdog.cpp)
add_executable(test_log tests/test_log.cpp ${LIB_SRC})
target_link_libraries(test_log ${LIBS})

add_executable(test_config tests/test_config.cpp ${LIB_SRC})
target_link_libraries(test_config ${LIBS})

add_executable(test_thread tests/test_thread.cpp ${LIB_SRC})
target_link_libraries(test_thread ${LIBS})

add_executable(test_fibel tests/test_fibel.cpp ${LIB_SRC})
target_link_libraries(test_fibel ${LIBS})

add_executable(test_scheduler tests/test_scheduler.cpp ${LIB_SRC})
target_link_libraries(test_scheduler ${LIBS})

add_executable(test_iomanager tests/test_iomanager.cpp ${LIB_SRC})
target_link_libraries(test_iomanager ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
