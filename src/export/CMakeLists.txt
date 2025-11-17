#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Export writers for libhviftools
#

add_library(hvif_export OBJECT
    HVIFWriter.cpp
    IOMWriter.cpp
    SVGWriter.cpp
    PNGWriter.cpp
)

target_include_directories(hvif_export PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    ${CMAKE_CURRENT_SOURCE_DIR}/../common
    ${CMAKE_CURRENT_SOURCE_DIR}/../import
    ${CMAKE_CURRENT_SOURCE_DIR}/../export
    ${NANOSVG_DIR}
    ${STB_DIR}
)

if(HAIKU)
    target_link_libraries(hvif_import PUBLIC be translation)
endif()
