#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Import parsers for libhviftools
#

add_library(hvif_import OBJECT
    HVIFParser.cpp
    IOMParser.cpp
    SVGParser.cpp
    PNGParser.cpp
)

target_include_directories(hvif_import PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../import
    ${CMAKE_CURRENT_SOURCE_DIR}/../export
    ${CMAKE_CURRENT_SOURCE_DIR}/../tracer
    ${CMAKE_CURRENT_SOURCE_DIR}/../tracer/core
    ${CMAKE_CURRENT_SOURCE_DIR}/../tracer/processing
    ${CMAKE_CURRENT_SOURCE_DIR}/../tracer/quantization
    ${CMAKE_CURRENT_SOURCE_DIR}/../tracer/output
    ${NANOSVG_DIR}
    ${STB_DIR}
)

if(HAIKU)
    target_link_libraries(hvif_import PUBLIC be translation)
endif()
