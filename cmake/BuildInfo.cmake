#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Build information summary output
#

message(STATUS "")
message(STATUS "Build Summary")
message(STATUS "=============")
message(STATUS "Version:        ${PROJECT_VERSION}")
message(STATUS "Build type:     ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "")

# ============================================================================
# Libraries
# ============================================================================

message(STATUS "Libraries:")

if(TARGET hviftools)
    set(_hviftools_type "static")
    if(BUILD_SHARED_LIBS)
        set(_hviftools_type "shared")
    endif()
    message(STATUS "  libhviftools:     ${_hviftools_type}")
else()
    message(STATUS "  libhviftools:     OFF")
endif()

if(TARGET imagetracer)
    set(_tracer_type "static")
    if(BUILD_SHARED_LIBS)
        set(_tracer_type "shared")
    endif()
    message(STATUS "  libimagetracer:   ${_tracer_type}")
else()
    message(STATUS "  libimagetracer:   OFF")
endif()

message(STATUS "")

# ============================================================================
# CLI Tools
# ============================================================================

message(STATUS "Tools to build:")

if(TARGET icon2icon)
    message(STATUS "  icon2icon:        ON")
else()
    message(STATUS "  icon2icon:        OFF")
endif()

if(TARGET img2svg)
    message(STATUS "  img2svg:          ON")
else()
    message(STATUS "  img2svg:          OFF")
endif()

if(TARGET msgdump)
    message(STATUS "  msgdump:          ON")
else()
    message(STATUS "  msgdump:          OFF")
endif()

message(STATUS "")

# ============================================================================
# Platform Integration
# ============================================================================

message(STATUS "Integration:")

if(TARGET HVIFThumbnailProvider)
    message(STATUS "  Windows addon:    ON")
elseif(WIN32)
    message(STATUS "  Windows addon:    OFF")
endif()

if(BUILD_INKSCAPE_EXTENSIONS)
    message(STATUS "  Inkscape ext:     ON")
else()
    message(STATUS "  Inkscape ext:     OFF")
endif()

message(STATUS "")

# ============================================================================
# Testing & Development
# ============================================================================

if(BUILD_STRESS_TEST OR HVIF_TOOLS_TESTS OR HVIF_TOOLS_SANITIZERS OR HVIF_TOOLS_WARNINGS)
    message(STATUS "Development options:")
    
    if(TARGET hvif-stress-test)
        message(STATUS "  Stress test:      ON")
    elseif(BUILD_STRESS_TEST)
        message(STATUS "  Stress test:      FAILED")
    endif()
    
    if(HVIF_TOOLS_TESTS)
        message(STATUS "  Tests:            ${HVIF_TOOLS_TESTS}")
    endif()
    
    if(HVIF_TOOLS_WARNINGS)
        message(STATUS "  Warnings:         ${HVIF_TOOLS_WARNINGS}")
    endif()
    
    if(HVIF_TOOLS_WERROR)
        message(STATUS "  Werror:           ${HVIF_TOOLS_WERROR}")
    endif()
    
    if(HVIF_TOOLS_SANITIZERS)
        message(STATUS "  Sanitizers:       ${HVIF_TOOLS_SANITIZERS}")
    endif()
    
    message(STATUS "")
endif()

# ============================================================================
# Dependencies
# ============================================================================

message(STATUS "Dependencies:")

if(NANOSVG_FOUND)
    message(STATUS "  NanoSVG:          found")
else()
    message(STATUS "  NanoSVG:          NOT FOUND")
endif()

if(STB_FOUND)
    message(STATUS "  STB:              found")
else()
    message(STATUS "  STB:              NOT FOUND")
endif()

message(STATUS "")

# ============================================================================
# Build targets summary
# ============================================================================

set(_targets_list "")

if(TARGET hviftools)
    list(APPEND _targets_list "hviftools")
endif()

if(TARGET imagetracer)
    list(APPEND _targets_list "imagetracer")
endif()

if(TARGET icon2icon)
    list(APPEND _targets_list "icon2icon")
endif()

if(TARGET img2svg)
    list(APPEND _targets_list "img2svg")
endif()

if(TARGET msgdump)
    list(APPEND _targets_list "msgdump")
endif()

if(TARGET hvif-stress-test)
    list(APPEND _targets_list "hvif-stress-test")
endif()

if(TARGET HVIFThumbnailProvider)
    list(APPEND _targets_list "HVIFThumbnailProvider")
endif()

list(LENGTH _targets_list _targets_count)

if(_targets_count GREATER 0)
    message(STATUS "Configured targets (${_targets_count}):")
    foreach(_target ${_targets_list})
        message(STATUS "  ${_target}")
    endforeach()
    message(STATUS "")
    
    message(STATUS "Build commands:")
    message(STATUS "  All targets:      cmake --build .")
    message(STATUS "  Specific target:  cmake --build . --target <name>")
    message(STATUS "")
    
    if(HVIF_TOOLS_INSTALL)
        message(STATUS "Installation:")
        message(STATUS "  cmake --install .")
        message(STATUS "  cmake --install . --component <name>")
        message(STATUS "")
    endif()
    
    message(STATUS "Packaging:")
    message(STATUS "  cmake --build . --target package")
    message(STATUS "")
else()
    message(WARNING "No targets configured! Enable at least one BUILD_* option.")
endif()
