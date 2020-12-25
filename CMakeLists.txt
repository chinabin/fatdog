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


function(ragelmaker src_rl outputlist outputdir)
    #Create a custom build step that will call ragel on the provided src_rl file.
    #The output .cpp file will be appended to the variable name passed in outputlist.

    get_filename_component(src_file ${src_rl} NAME_WE)

    set(rl_out ${outputdir}/${src_file}.rl.cc)

    #adding to the list inside a function takes special care, we cannot use list(APPEND...)
    #because the results are local scope only
    set(${outputlist} ${${outputlist}} ${rl_out} PARENT_SCOPE)

    #Warning: The " -S -M -l -C -T0  --error-format=msvc" are added to match existing window invocation
    #we might want something different for mac and linux
    add_custom_command(
        OUTPUT ${rl_out}
        COMMAND cd ${outputdir}
        COMMAND ragel ${CMAKE_CURRENT_SOURCE_DIR}/${src_rl} -o ${rl_out} -l -C -G2  --error-format=msvc
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${src_rl}
        )
    set_source_files_properties(${rl_out} PROPERTIES GENERATED TRUE)
endfunction(ragelmaker)


set(LIB_SRC
    fatdog/macro.h
    fatdog/singleton.h
    fatdog/noncopyable.h
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
    fatdog/endian.h
    fatdog/address.h
    fatdog/address.cpp
    fatdog/fd_manager.h
    fatdog/fd_manager.cpp
    fatdog/hook.h
    fatdog/hook.cpp
    fatdog/bytearray.h
    fatdog/bytearray.cpp
    fatdog/socket.h
    fatdog/socket.cpp
    fatdog/http/http.h
    fatdog/http/http.cpp
    fatdog/http/http_parser.h
    fatdog/http/http_parser.cpp
    fatdog/tcp_server.h
    fatdog/tcp_server.cpp
    fatdog/socket_stream.h
    fatdog/socket_stream.cpp
    fatdog/stream.h
    fatdog/stream.cpp
    fatdog/http/http_session.h
    fatdog/http/http_session.cpp
    fatdog/http/http_server.h
    fatdog/http/http_server.cpp
    fatdog/http/servlet.h
    fatdog/http/servlet.cpp
    )

set(LIBS
        pthread
        yaml-cpp
        dl)

ragelmaker(fatdog/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/fatdog/http)
ragelmaker(fatdog/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/fatdog/http)
ragelmaker(fatdog/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/fatdog)

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

add_executable(test_address tests/test_address.cpp ${LIB_SRC})
target_link_libraries(test_address ${LIBS})

add_executable(test_sleep tests/test_sleep.cpp)
target_link_libraries(test_sleep dl)

add_executable(test_hook tests/test_hook.cpp ${LIB_SRC})
target_link_libraries(test_hook ${LIBS})

add_executable(test_bytearray tests/test_bytearray.cpp ${LIB_SRC})
target_link_libraries(test_bytearray ${LIBS})

add_executable(test_socket tests/test_socket.cpp ${LIB_SRC})
target_link_libraries(test_socket ${LIBS})

add_executable(test_http tests/test_http.cpp ${LIB_SRC})
target_link_libraries(test_http ${LIBS})

add_executable(test_http_parser tests/test_http_parser.cpp ${LIB_SRC})
target_link_libraries(test_http_parser ${LIBS})

add_executable(test_tcp_server tests/test_tcp_server.cpp ${LIB_SRC})
target_link_libraries(test_tcp_server ${LIBS})

add_executable(echo_server examples/echo_server.cpp ${LIB_SRC})
target_link_libraries(echo_server ${LIBS})

add_executable(test_http_server tests/test_http_server.cpp ${LIB_SRC})
target_link_libraries(test_http_server ${LIBS})

add_executable(test_uri tests/test_uri.cpp ${LIB_SRC})
target_link_libraries(test_uri ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
