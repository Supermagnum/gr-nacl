# Copyright 2011 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# GNU Radio is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# GNU Radio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with GNU Radio; see the file COPYING.  If not, write to
# the Free Software Foundation, Inc., 51 Franklin Street,
# Boston, MA 02110-1301, USA.

########################################################################
# Python bindings using pybind11
########################################################################
find_package(Python3 COMPONENTS Interpreter Development)
find_package(pybind11 QUIET)

if(NOT Python3_FOUND OR NOT pybind11_FOUND)
    message(STATUS "Python3 or pybind11 not found - skipping Python bindings")
    return()
endif()

# Create Python bindings
pybind11_add_module(nacl_python
    nacl_python.cc
)

target_include_directories(nacl_python PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${GNURADIO_ALL_INCLUDE_DIRS}
)

target_link_libraries(nacl_python PRIVATE
    gnuradio-nacl
    gnuradio-runtime
    gnuradio-pmt
)

# Install Python module
install(TARGETS nacl_python
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python3/dist-packages
)

# Install Python wrapper module to gnuradio package
install(FILES gnuradio_nacl_init.py
    DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/python3/dist-packages/gnuradio
    RENAME nacl.py
)

########################################################################
# Handle the unit tests
########################################################################
include(GrTest)

set(GR_TEST_TARGET_DEPS gnuradio-nacl)
set(GR_TEST_PYTHON_DIRS ${CMAKE_BINARY_DIR}/swig)
GR_ADD_TEST(qa_encrypt_public ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_encrypt_public.py)
GR_ADD_TEST(qa_decrypt_public ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_decrypt_public.py)
GR_ADD_TEST(qa_encrypt_secret ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_encrypt_secret.py)
GR_ADD_TEST(qa_decrypt_secret ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_decrypt_secret.py)
GR_ADD_TEST(qa_crypt_tagged_stream ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/qa_crypt_tagged_stream.py)
