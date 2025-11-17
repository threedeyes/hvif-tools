#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# External dependencies detection and configuration
#

include(GNUInstallDirs)

find_package(Threads REQUIRED)

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

# ============================================================================
# NanoSVG (required for SVG parsing/writing)
# ============================================================================

set(NANOSVG_DIR "${EXTERNAL_DIR}/nanosvg_ext/src")
if(NOT EXISTS "${NANOSVG_DIR}/nanosvg.h")
    if(BUILD_HVIFTOOLS_LIB)
        message(FATAL_ERROR 
            "NanoSVG is required for libhviftools. "
            "Please initialize submodules: git submodule update --init --recursive"
        )
    else()
        message(WARNING "NanoSVG not found, libhviftools will not be available")
        set(NANOSVG_FOUND FALSE)
    endif()
else()
    set(NANOSVG_FOUND TRUE)
    message(STATUS "Found NanoSVG: ${NANOSVG_DIR}")
    
    add_library(nanosvg INTERFACE)
    target_include_directories(nanosvg INTERFACE ${NANOSVG_DIR})
endif()

# ============================================================================
# STB (required for image loading)
# ============================================================================

set(STB_DIR "${EXTERNAL_DIR}/stb")
if(NOT EXISTS "${STB_DIR}/stb_image.h")
    if(BUILD_IMAGETRACER_LIB)
        message(FATAL_ERROR 
            "STB is required for libimagetracer. "
            "Please initialize submodules: git submodule update --init --recursive"
        )
    else()
        message(WARNING "STB not found, libimagetracer will not be available")
        set(STB_FOUND FALSE)
    endif()
else()
    set(STB_FOUND TRUE)
    message(STATUS "Found STB: ${STB_DIR}")
    
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${STB_DIR})
endif()

# ============================================================================
# Disable builds if dependencies are missing
# ============================================================================

if(BUILD_HVIFTOOLS_LIB AND NOT NANOSVG_FOUND)
    message(WARNING "Disabling libhviftools due to missing NanoSVG")
    set(BUILD_HVIFTOOLS_LIB OFF CACHE BOOL "" FORCE)
    set(BUILD_ICON2ICON OFF CACHE BOOL "" FORCE)
    set(BUILD_HVIF4WIN OFF CACHE BOOL "" FORCE)
endif()

if(BUILD_IMAGETRACER_LIB AND NOT STB_FOUND)
    message(WARNING "Disabling libimagetracer due to missing STB")
    set(BUILD_IMAGETRACER_LIB OFF CACHE BOOL "" FORCE)
    set(BUILD_IMG2SVG OFF CACHE BOOL "" FORCE)
endif()
