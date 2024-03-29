cmake_minimum_required(VERSION 3.2)
PROJECT(vmutils)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(INCLUDE_FILES customization.h)
set(EXPORT_HEAD_FILES "")
add_executable(vmget vmget.c)
add_executable(vmset vmset.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -O2 -fpic")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fpic")

# 设置交叉编译器
if(DEFINED CROSS_COMPILE)
    message(STATUS "CROSS_COMPILE is set to ${CROSS_COMPILE}")
    set(MY_COMPILE_C ${CROSS_COMPILE}gcc)
    set(MY_COMPILE_C++ ${CROSS_COMPILE}g++)
else()
    message(STATUS "CROSS_COMPILE is not set")
    set(MY_COMPILE_C gcc)
    set(MY_COMPILE_C++ g++)
endif()

if(DEFINED MY_SYSROOT)
    set(CMAKE_SYSROOT ${MY_SYSROOT})
endif()

if(DEFINED MY_C_FLAGS)
    message(STATUS "MY_C_FLAGS ${MY_C_FLAGS}")
    set(CMAKE_C_FLAGS ${MY_C_FLAGS})
endif()

if(DEFINED MY_CXX_FLAGS)
    message(STATUS "MY_CXX_FLAGS ${MY_CXX_FLAGS}")
    set(CMAKE_CXX_FLAGS ${MY_CXX_FLAGS})
endif()

if(DEFINED MY_DEBUG)
    message(STATUS "set DEBUG FLAG")
    set(CMAKE_C_FLAGS "${MY_C_FLAGS} -g")
    set(CMAKE_CXX_FLAGS "${MY_CXX_FLAGS} -g")
endif()

message(STATUS "CMAKE_BINARY_DIR is ${CMAKE_BINARY_DIR}")
if(DEFINED INSTALL_DIR)
    message(STATUS "CMAKE_INSTALL_PREFIX is set to : ${INSTALL_DIR}")
    set(MY_INSTALL_DIR ${INSTALL_DIR})
else()
    message(STATUS "CMAKE_INSTALL_PREFIX is set to: ${CMAKE_BINARY_DIR}")
    set(MY_INSTALL_DIR ${CMAKE_BINARY_DIR})
endif()

if(DEFINED EXTERN_INCLUDE_DIR)
    message(STATUS "EXTERN_INCLUDE_DIR is set to ${EXTERN_INCLUDE_DIR}")
    include_directories(${EXTERN_INCLUDE_DIR})
endif()

if(DEFINED EXTERN_LIB_DIR)
    message(STATUS "EXTERN_LIB_DIR is set to ${EXTERN_LIB_DIR}")
    link_directories(${EXTERN_LIB_DIR})
else()
    message(FATAL_ERROR "EXTERN_LIB_DIR is not defined!")
endif()

target_link_libraries(vmget vmdb)

install(TARGETS vmget vmset
        RUNTIME DESTINATION ${MY_INSTALL_DIR}/output/bin/
        )
install(FILES ${EXPORT_HEAD_FILES}
        DESTINATION ${MY_INSTALL_DIR}/output/include/)