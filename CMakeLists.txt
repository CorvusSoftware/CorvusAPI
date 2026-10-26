# CorvusAPI - CorvusSoftware
# Copyright (C) 2026 CorvusSoftware
# SPDX-License-Identifier: MIT

cmake_minimum_required(VERSION 3.16)
project(CorvusAPI VERSION 0.1.0 LANGUAGES C)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# The public API is header-only; cores implement its entrypoints themselves.
add_library(corvusapi INTERFACE)
add_library(CorvusAPI::CorvusAPI ALIAS corvusapi)
set_target_properties(corvusapi PROPERTIES EXPORT_NAME CorvusAPI)
target_include_directories(corvusapi INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

set(CORVUSAPI_INSTALL_CONFIG_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/CorvusAPI")
configure_package_config_file(
    cmake/CorvusAPIConfig.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/CorvusAPIConfig.cmake"
    INSTALL_DESTINATION "${CORVUSAPI_INSTALL_CONFIG_DIR}"
)
# ABI 0 is experimental: do not promise compatibility across package versions.
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/CorvusAPIConfigVersion.cmake"
    VERSION "${PROJECT_VERSION}"
    COMPATIBILITY ExactVersion
)

install(DIRECTORY include/corvus DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(FILES include/corvusapi.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(TARGETS corvusapi EXPORT CorvusAPITargets)
install(EXPORT CorvusAPITargets
    NAMESPACE CorvusAPI::
    DESTINATION "${CORVUSAPI_INSTALL_CONFIG_DIR}"
)
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/CorvusAPIConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/CorvusAPIConfigVersion.cmake"
    DESTINATION "${CORVUSAPI_INSTALL_CONFIG_DIR}"
)

option(CORVUSAPI_BUILD_VERIFIER "Build the standalone native ABI verifier" OFF)
option(CORVUSAPI_BUILD_EXAMPLES "Build the independent minimal C example core" OFF)
option(CORVUSAPI_BUILD_TESTS "Test the example core and public ABI" OFF)

function(corvusapi_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /WX)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Werror -pedantic)
    endif()
endfunction()

if(CORVUSAPI_BUILD_VERIFIER)
    add_executable(corvusapi-verify tools/corvusapi_verify.c)
    target_compile_features(corvusapi-verify PRIVATE c_std_11)
    target_link_libraries(corvusapi-verify PRIVATE CorvusAPI::CorvusAPI ${CMAKE_DL_LIBS})
    corvusapi_set_warnings(corvusapi-verify)
endif()

if(CORVUSAPI_BUILD_EXAMPLES)
    add_library(corvusapi_example_core SHARED examples/minimal_core.c)
    target_compile_features(corvusapi_example_core PRIVATE c_std_11)
    target_link_libraries(corvusapi_example_core PRIVATE CorvusAPI::CorvusAPI)
    corvusapi_set_warnings(corvusapi_example_core)
    set_target_properties(corvusapi_example_core PROPERTIES C_VISIBILITY_PRESET hidden)
endif()

if(CORVUSAPI_BUILD_TESTS)
    if(NOT CORVUSAPI_BUILD_VERIFIER OR NOT CORVUSAPI_BUILD_EXAMPLES)
        message(FATAL_ERROR
            "CORVUSAPI_BUILD_TESTS needs CORVUSAPI_BUILD_VERIFIER and CORVUSAPI_BUILD_EXAMPLES")
    endif()
    enable_testing()
    add_test(NAME corvusapi_verify_example
        COMMAND corvusapi-verify "$<TARGET_FILE:corvusapi_example_core>")

    add_executable(corvusapi_lifecycle_test
        tests/minimal_core_test.c examples/minimal_core.c)
    target_compile_features(corvusapi_lifecycle_test PRIVATE c_std_11)
    target_link_libraries(corvusapi_lifecycle_test PRIVATE CorvusAPI::CorvusAPI)
    corvusapi_set_warnings(corvusapi_lifecycle_test)
    add_test(NAME corvusapi_lifecycle COMMAND corvusapi_lifecycle_test)
endif()
