#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# hvif-tools: Main CMake configuration
#

cmake_minimum_required(VERSION 3.16)

project(hvif-tools 
    VERSION 0.1.0 
    DESCRIPTION "Tools for working with Haiku Vector Icon Format"
    LANGUAGES CXX
)

option(BUILD_ALL_TOOLS "Build all CLI tools" ON)

option(BUILD_HVIF2SVG "Build hvif2svg: HVIF to SVG converter" ${BUILD_ALL_TOOLS})
option(BUILD_SVG2HVIF "Build svg2hvif: SVG to HVIF converter" ${BUILD_ALL_TOOLS})
option(BUILD_IOM2SVG  "Build iom2svg: Icon-O-Matic to SVG converter" ${BUILD_ALL_TOOLS})
option(BUILD_SVG2IOM  "Build svg2iom: SVG to Icon-O-Matic converter" ${BUILD_ALL_TOOLS})
option(BUILD_IMG2SVG  "Build img2svg: Bitmap to SVG vectorizer" ${BUILD_ALL_TOOLS})
option(BUILD_MSG2TXT  "Build msg2txt: BMessage dump utility (debug tool)" OFF)

if(HAIKU)
    option(BUILD_IOM2HVIF "Build iom2hvif: Icon-O-Matic to HVIF converter (Haiku only)" ${BUILD_ALL_TOOLS})
else()
    set(BUILD_IOM2HVIF OFF CACHE BOOL "Icon-O-Matic to HVIF converter (Haiku only)" FORCE)
endif()

if(WIN32)
    option(BUILD_HVIF4WIN "Build Windows thumbnail provider for HVIF files" ON)
else()
    set(BUILD_HVIF4WIN OFF CACHE BOOL "Windows thumbnail provider (Windows only)" FORCE)
endif()

option(BUILD_INKSCAPE_EXTENSIONS "Install Inkscape import/export extensions" OFF)

option(HVIF_TOOLS_WARNINGS    "Enable extra compiler warnings" OFF)
option(HVIF_TOOLS_WERROR      "Treat warnings as errors" OFF)
option(HVIF_TOOLS_TESTS       "Build unit tests" OFF)
option(HVIF_TOOLS_SANITIZERS  "Enable sanitizers (ASan, UBSan)" OFF)
option(HVIF_TOOLS_INSTALL     "Enable installation targets" ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS 
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CompilerSettings)
include(Dependencies)

add_subdirectory(src/common)
add_subdirectory(external)

if(BUILD_HVIF2SVG)
    add_subdirectory(src/hvif2svg)
endif()

if(BUILD_SVG2HVIF)
    add_subdirectory(src/svg2hvif)
endif()

if(BUILD_IOM2SVG)
    add_subdirectory(src/iom2svg)
endif()

if(BUILD_SVG2IOM)
    add_subdirectory(src/svg2iom)
endif()

if(BUILD_IMG2SVG)
    add_subdirectory(src/img2svg)
endif()

if(BUILD_MSG2TXT)
    add_subdirectory(src/msg2txt)
endif()

if(BUILD_IOM2HVIF AND HAIKU)
    add_subdirectory(src/iom2hvif)
endif()

if(BUILD_HVIF4WIN AND WIN32)
    add_subdirectory(src/hvif4win)
endif()

if(BUILD_INKSCAPE_EXTENSIONS)
    add_subdirectory(inkscape)
endif()

if(HVIF_TOOLS_INSTALL)
    include(InstallRules)
endif()

include(cmake/BuildInfo.cmake)
