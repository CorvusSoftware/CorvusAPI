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
install(TARGETS corvusapi EXPORT CorvusAPITargets)
install(EXPORT CorvusAPITargets
    NAMESPACE CorvusAPI::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/CorvusAPI
)
