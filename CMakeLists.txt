#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# hvif-tools: Main CMake configuration
#

cmake_minimum_required(VERSION 3.16)

project(hvif-tools
    VERSION 2.1.2
    DESCRIPTION "Tools and libraries for Haiku Vector Icon Format"
    LANGUAGES CXX
)

# ============================================================================
# Load required CMake modules early
# ============================================================================

include(CMakeDependentOption)

# ============================================================================
# Build options: Libraries
# ============================================================================

option(BUILD_HVIFTOOLS_LIB "Build libhviftools library" ON)
option(BUILD_IMAGETRACER_LIB "Build libimagetracer library" ON)
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" OFF)

# ============================================================================
# Build options: CLI Tools
# ============================================================================

cmake_dependent_option(BUILD_ICON2ICON
    "Build icon2icon: Universal icon format converter"
    ON "BUILD_HVIFTOOLS_LIB" OFF)

cmake_dependent_option(BUILD_IMG2SVG
    "Build img2svg: Bitmap to SVG vectorizer"
    ON "BUILD_IMAGETRACER_LIB" OFF)

option(BUILD_IOM2HVIF "Build iom2hvif: Icon-O-Matic to HVIF converter (Haiku only)" OFF)

option(BUILD_MSGDUMP "Build msgdump: BMessage dump utility (debug tool)" OFF)

# ============================================================================
# Build options: Platform addons
# ============================================================================

if(WIN32)
    cmake_dependent_option(BUILD_HVIF4WIN
        "Build Windows thumbnail provider for HVIF/IOM files"
        ON "BUILD_HVIFTOOLS_LIB" OFF)
else()
    set(BUILD_HVIF4WIN OFF CACHE BOOL "Windows thumbnail provider (Windows only)" FORCE)
endif()

# ============================================================================
# Build options: Integration
# ============================================================================

option(BUILD_INKSCAPE_EXTENSIONS "Install Inkscape import/export extensions" OFF)

# ============================================================================
# Development options
# ============================================================================

option(HVIF_TOOLS_WARNINGS    "Enable extra compiler warnings" OFF)
option(HVIF_TOOLS_WERROR      "Treat warnings as errors" OFF)
option(HVIF_TOOLS_TESTS       "Build unit tests" OFF)
option(HVIF_TOOLS_SANITIZERS  "Enable sanitizers (ASan, UBSan)" OFF)
option(HVIF_TOOLS_INSTALL     "Enable installation targets" ON)

# Stress testing
cmake_dependent_option(BUILD_STRESS_TEST
    "Build stress testing utility for icon converters"
    OFF "BUILD_HVIFTOOLS_LIB" OFF)

# ============================================================================
# Build type
# ============================================================================

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo"
    )
endif()

# ============================================================================
# CMake modules
# ============================================================================

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(CompilerSettings)
include(Dependencies)

# ============================================================================
# Subdirectories
# ============================================================================

add_subdirectory(external)
add_subdirectory(src)

if(BUILD_INKSCAPE_EXTENSIONS)
    add_subdirectory(inkscape)
endif()

# ============================================================================
# Installation and packaging
# ============================================================================

if(HVIF_TOOLS_INSTALL)
    include(InstallRules)
endif()

# ============================================================================
# Build summary
# ============================================================================

include(cmake/BuildInfo.cmake)
