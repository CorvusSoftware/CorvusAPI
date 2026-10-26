# CorvusAPI - CorvusSoftware
# Copyright (C) 2026 CorvusSoftware
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)
project(CorvusAPI VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)

add_library(corvusapi INTERFACE)
add_library(CorvusAPI::CorvusAPI ALIAS corvusapi)

target_include_directories(corvusapi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

install(DIRECTORY include/corvus DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES include/corvusapi.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS corvusapi EXPORT CorvusAPITargets)
install(EXPORT CorvusAPITargets
    NAMESPACE CorvusAPI::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CorvusAPI
)

# Conformance verifier is a host tool; headers remain a standalone C ABI.
option(CORVUSAPI_BUILD_VERIFIER "Build the CorvusAPI native module ABI verifier" OFF)
if(CORVUSAPI_BUILD_VERIFIER)
    add_executable(corvusapi-verify tools/corvusapi_verify.c)
    target_compile_features(corvusapi-verify PRIVATE c_std_11)
    target_link_libraries(corvusapi-verify PRIVATE CorvusAPI::CorvusAPI ${CMAKE_DL_LIBS})
    if(MSVC)
        target_compile_options(corvusapi-verify PRIVATE /W4)
    else()
        target_compile_options(corvusapi-verify PRIVATE -Wall -Wextra -Werror -pedantic)
    endif()
endif()
