#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# External dependencies detection and configuration
#

include(GNUInstallDirs)

find_package(Threads REQUIRED)

set(EXTERNAL_DIR "${CMAKE_SOURCE_DIR}/external")

set(NANOSVG_DIR "${EXTERNAL_DIR}/nanosvg_ext/src")
if(NOT EXISTS "${NANOSVG_DIR}/nanosvg.h")
    message(WARNING 
        "nanosvg.h not found in ${NANOSVG_DIR}. "
        "SVG conversion tools will not be available."
    )
    set(NANOSVG_FOUND FALSE)
else()
    set(NANOSVG_FOUND TRUE)
    message(STATUS "Found NanoSVG: ${NANOSVG_DIR}")
endif()

set(STB_DIR "${EXTERNAL_DIR}/stb")
if(NOT EXISTS "${STB_DIR}/stb_image.h")
    message(WARNING 
        "stb_image.h not found in ${STB_DIR}. "
        "img2svg will not be available."
    )
    set(STB_FOUND FALSE)
else()
    set(STB_FOUND TRUE)
    message(STATUS "Found STB: ${STB_DIR}")
endif()

if(BUILD_SVG2HVIF OR BUILD_SVG2IOM)
    if(NOT NANOSVG_FOUND)
        message(FATAL_ERROR 
            "NanoSVG is required for svg2hvif and svg2iom. "
            "Please place nanosvg.h in ${NANOSVG_DIR}"
        )
    endif()
endif()

if(BUILD_IMG2SVG)
    if(NOT STB_FOUND)
        message(FATAL_ERROR 
            "STB is required for img2svg. "
            "Please place stb_image.h in ${STB_DIR}"
        )
    endif()
endif()

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

add_library(hvif_common_includes INTERFACE)
target_include_directories(hvif_common_includes INTERFACE
    ${CMAKE_SOURCE_DIR}/src/common
)

if(NANOSVG_FOUND)
    add_library(nanosvg INTERFACE)
    target_include_directories(nanosvg INTERFACE ${NANOSVG_DIR})
endif()

if(STB_FOUND)
    add_library(stb INTERFACE)
    target_include_directories(stb INTERFACE ${STB_DIR})
endif()

if(BUILD_IOM2HVIF AND HAIKU AND AGG_FOUND)
    add_library(haiku_agg INTERFACE)
    target_include_directories(haiku_agg INTERFACE ${AGG_INCLUDEDIR})
    target_link_libraries(haiku_agg INTERFACE ${AGG_LIBRARIES})
endif()
