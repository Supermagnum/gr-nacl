#[=======================================================================[.rst:
FindSodium
----------

Finds the libsodium cryptography library (minimum version 1.0.22).

Result variables
^^^^^^^^^^^^^^^^

``Sodium_FOUND``
  True if libsodium was found and meets the minimum version requirement.
``Sodium_VERSION``
  Detected libsodium version string.

Imported targets
^^^^^^^^^^^^^^^^

``sodium::sodium``
  Imported target for linking against libsodium with include directories set.

Cache variables
^^^^^^^^^^^^^^^

``SODIUM_USE_STATIC_LIBS``
  When ON, prefer the static ``libsodium.a`` archive over the shared library.

Legacy variables (backward compatibility)
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

``SODIUM_LIBRARIES``, ``SODIUM_INCLUDE_DIRS``, ``SODIUM_LIBRARY_DIRS``
#]=======================================================================]

include(FindPackageHandleStandardArgs)

set(_SODIUM_REQUIRED_VERSION "1.0.22")

option(SODIUM_USE_STATIC_LIBS "Prefer static libsodium over shared" OFF)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    if(SODIUM_USE_STATIC_LIBS)
        pkg_check_modules(PC_SODIUM QUIET IMPORTED_TARGET GLOBAL libsodium>=${_SODIUM_REQUIRED_VERSION})
        if(NOT PC_SODIUM_FOUND)
            pkg_check_modules(PC_SODIUM QUIET libsodium>=${_SODIUM_REQUIRED_VERSION})
        endif()
    else()
        pkg_check_modules(PC_SODIUM QUIET libsodium>=${_SODIUM_REQUIRED_VERSION})
    endif()
endif()

find_path(
    SODIUM_INCLUDE_DIR
    NAMES sodium.h
    HINTS
        ${PC_SODIUM_INCLUDEDIR}
        ${PC_SODIUM_INCLUDE_DIRS}
    PATHS
        /usr/local/include
        /usr/include
    PATH_SUFFIXES
        ""
)

if(SODIUM_USE_STATIC_LIBS)
    find_library(
        SODIUM_LIBRARY
        NAMES libsodium.a sodium
        HINTS
            ${PC_SODIUM_LIBDIR}
            ${PC_SODIUM_LIBRARY_DIRS}
        PATHS
            /usr/local/lib
            /usr/lib
            /usr/lib64
    )
else()
    find_library(
        SODIUM_LIBRARY
        NAMES sodium libsodium
        HINTS
            ${PC_SODIUM_LIBDIR}
            ${PC_SODIUM_LIBRARY_DIRS}
        PATHS
            /usr/local/lib
            /usr/lib
            /usr/lib64
    )
    if(NOT SODIUM_LIBRARY)
        find_library(
            SODIUM_LIBRARY
            NAMES libsodium.a sodium
            HINTS
                ${PC_SODIUM_LIBDIR}
                ${PC_SODIUM_LIBRARY_DIRS}
            PATHS
                /usr/local/lib
                /usr/lib
                /usr/lib64
        )
    endif()
endif()

set(Sodium_VERSION "${PC_SODIUM_VERSION}")
if(NOT Sodium_VERSION AND SODIUM_INCLUDE_DIR)
    if(EXISTS "${SODIUM_INCLUDE_DIR}/sodium/version.h")
        file(
            STRINGS
            "${SODIUM_INCLUDE_DIR}/sodium/version.h"
            _SODIUM_VERSION_LINE
            REGEX "^#define SODIUM_VERSION_STRING"
        )
        if(_SODIUM_VERSION_LINE)
            string(REGEX REPLACE ".*\"(.*)\"" "\\1" Sodium_VERSION "${_SODIUM_VERSION_LINE}")
        endif()
    endif()
endif()

find_package_handle_standard_args(
    Sodium
    REQUIRED_VARS SODIUM_LIBRARY SODIUM_INCLUDE_DIR
    VERSION_VAR Sodium_VERSION
)

if(Sodium_FOUND)
    if(Sodium_VERSION VERSION_LESS _SODIUM_REQUIRED_VERSION)
        set(Sodium_FOUND FALSE)
        set(SODIUM_FOUND FALSE)
        if(Sodium_FIND_REQUIRED)
            message(FATAL_ERROR
                "Found libsodium ${Sodium_VERSION}, but gr-nacl requires "
                "at least ${_SODIUM_REQUIRED_VERSION}.")
        else()
            message(STATUS
                "Found libsodium ${Sodium_VERSION}, but gr-nacl requires "
                "at least ${_SODIUM_REQUIRED_VERSION}.")
        endif()
    endif()
endif()

if(Sodium_FOUND AND NOT TARGET sodium::sodium)
    add_library(sodium::sodium UNKNOWN IMPORTED)
    set_target_properties(
        sodium::sodium
        PROPERTIES
            IMPORTED_LOCATION "${SODIUM_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${SODIUM_INCLUDE_DIR}"
    )
    if(PC_SODIUM_CFLAGS_OTHER)
        set_target_properties(
            sodium::sodium
            PROPERTIES
                INTERFACE_COMPILE_OPTIONS "${PC_SODIUM_CFLAGS_OTHER}"
        )
    endif()
    if(PC_SODIUM_LDFLAGS_OTHER)
        set_target_properties(
            sodium::sodium
            PROPERTIES
                INTERFACE_LINK_OPTIONS "${PC_SODIUM_LDFLAGS_OTHER}"
        )
    endif()
endif()

set(SODIUM_LIBRARIES ${SODIUM_LIBRARY})
set(SODIUM_INCLUDE_DIRS ${SODIUM_INCLUDE_DIR})
get_filename_component(SODIUM_LIBRARY_DIRS ${SODIUM_LIBRARY} PATH)

mark_as_advanced(SODIUM_LIBRARY SODIUM_INCLUDE_DIR)
