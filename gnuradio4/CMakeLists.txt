# SPDX-License-Identifier: GPL-3.0-or-later
cmake_minimum_required(VERSION 3.22)
project(
  gr-nacl4
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "GNU Radio 4.0 out-of-tree libsodium (NaCl-style) blocks")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(GR_NACL4_BUILD_TESTS "Build Boost.UT tests" ON)

find_package(gnuradio4 REQUIRED CONFIG)
find_package(PkgConfig REQUIRED)
pkg_check_modules(libsodium REQUIRED IMPORTED_TARGET libsodium)

add_library(gnuradio4-gr-nacl INTERFACE)
add_library(gnuradio4::gr-nacl ALIAS gnuradio4-gr-nacl)
target_include_directories(
  gnuradio4-gr-nacl
  INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>)
target_link_libraries(gnuradio4-gr-nacl INTERFACE gnuradio4::gnuradio-core gnuradio4::gnuradio-blocklib-core
                                                  PkgConfig::libsodium)

install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/include/" DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(
  TARGETS gnuradio4-gr-nacl
  EXPORT gr-nacl4-targets
  INCLUDES
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}")

install(
  EXPORT gr-nacl4-targets
  FILE gr-nacl4-targets.cmake
  NAMESPACE gnuradio4::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gr-nacl4")

include(CMakePackageConfigHelpers)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/gr-nacl4-config.cmake.in" "${CMAKE_CURRENT_BINARY_DIR}/gr-nacl4-config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gr-nacl4")

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/gr-nacl4-config-version.cmake"
  VERSION "${PROJECT_VERSION}"
  COMPATIBILITY SameMajorVersion)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gr-nacl4-config.cmake" "${CMAKE_CURRENT_BINARY_DIR}/gr-nacl4-config-version.cmake"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/gr-nacl4")

configure_file("${CMAKE_CURRENT_SOURCE_DIR}/cmake/gnuradio4-gr-nacl.pc.in" "${CMAKE_CURRENT_BINARY_DIR}/gnuradio4-gr-nacl.pc"
               @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/gnuradio4-gr-nacl.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

if(GR_NACL4_BUILD_TESTS)
  include(FetchContent)
  set(BOOST_UT_DISABLE_MODULE ON CACHE BOOL "Disable UT Module Support" FORCE)
  FetchContent_Declare(
    ut
    GIT_REPOSITORY https://github.com/boost-ext/ut.git
    GIT_TAG 53e17f25119598c6458d30351b260193096ba67e
    EXCLUDE_FROM_ALL)
  FetchContent_MakeAvailable(ut)
  enable_testing()
  add_subdirectory(test)
endif()
