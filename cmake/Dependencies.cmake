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
# Haiku dependencies (iom2hvif)
# ============================================================================

if(BUILD_IOM2HVIF AND HAIKU)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(AGG REQUIRED libagg)
        if(AGG_FOUND)
            message(STATUS "Found libagg: ${AGG_INCLUDEDIR}")
        endif()
    else()
        message(FATAL_ERROR "pkg-config is required for iom2hvif on Haiku")
    endif()
    
    find_library(BE_LIBRARY be)
    if(NOT BE_LIBRARY)
        message(FATAL_ERROR "Haiku 'be' library not found")
    endif()
    message(STATUS "Found Haiku be library: ${BE_LIBRARY}")
endif()

if(BUILD_IOM2HVIF AND HAIKU AND AGG_FOUND)
    add_library(haiku_agg INTERFACE)
    target_include_directories(haiku_agg INTERFACE ${AGG_INCLUDEDIR})
    target_link_libraries(haiku_agg INTERFACE ${AGG_LIBRARIES})
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

if(BUILD_IOM2HVIF AND HAIKU AND NOT AGG_FOUND)
    message(WARNING "Disabling iom2hvif due to missing libagg")
    set(BUILD_IOM2HVIF OFF CACHE BOOL "" FORCE)
endif()
