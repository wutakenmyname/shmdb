cmake_minimum_required(VERSION 3.2)
PROJECT(vmdb)
include(config.cmake)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(INCLUDE_FILES vmdb.h list.h mbool.h status.h db_base.h customization_vm.h)
set(SOURCE_FILES vmdb.c)
set(EXPORT_HEAD_FILES status.h mbool.h db_base.h vmdb.h customization_vm.h byte_types.h)
set(LIB_NAME vmdb)
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
    message(FATAL_ERROR "INSTALL_DIR is not defined")
endif()

if(DEFINED EXTERN_LIB_DIR)
    message(STATUS "EXTERN_LIB_DIR is set to ${EXTERN_LIB_DIR}")
    link_directories(${EXTERN_LIB_DIR})
else()
    message(FATAL_ERROR "EXTERN_LIB_DIR is not defined!")
endif()

add_library(${LIB_NAME} SHARED ${SOURCE_FILES})
target_link_libraries(${LIB_NAME} basedb)

install(TARGETS ${LIB_NAME}
        LIBRARY DESTINATION ${MY_INSTALL_DIR}/output/lib/
        )
install(FILES ${EXPORT_HEAD_FILES}
        DESTINATION ${MY_INSTALL_DIR}/output/include/)
