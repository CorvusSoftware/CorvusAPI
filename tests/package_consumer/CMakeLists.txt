# CorvusAPI - CorvusSoftware
# Copyright (C) 2026 CorvusSoftware
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)
project(CorvusAPIInstalledConsumer LANGUAGES C)
find_package(CorvusAPI 0.1.0 EXACT CONFIG REQUIRED)
add_executable(corvusapi_installed_consumer main.c)
target_compile_features(corvusapi_installed_consumer PRIVATE c_std_11)
target_link_libraries(corvusapi_installed_consumer PRIVATE CorvusAPI::CorvusAPI)
enable_testing()
add_test(NAME installed_consumer COMMAND corvusapi_installed_consumer)
