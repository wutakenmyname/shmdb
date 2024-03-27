cmake_minimum_required(VERSION 3.2)
PROJECT(base_db)

set(INCLUDE_FILES db_base.h list.h mbool.h status.h)
set(SOURCE_FILES db_base.c)
set(EXPORT_HEAD_FILES status.h mbool.h db_base.h)
set(LIB_NAME basedb)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(STATUS "CMAKE_INSTALL_PREFIX is set to default: ${CMAKE_INSTALL_PREFIX}")
    set(MY_INSTALL_DIR ${CMAKE_BINARY_DIR})
else()
    message(STATUS "CMAKE_INSTALL_PREFIX is set to: ${CMAKE_INSTALL_PREFIX}")
    set(MY_INSTALL_DIR ${CMAKE_INSTALL_PREFIX})
endif()

add_library(${LIB_NAME} SHARED ${SOURCE_FILES})

install(TARGETS ${LIB_NAME}
        LIBRARY DESTINATION ${MY_INSTALL_DIR}/output/lib/
        )
install(FILES ${EXPORT_HEAD_FILES}
        DESTINATION ${MY_INSTALL_DIR}/output/include/)