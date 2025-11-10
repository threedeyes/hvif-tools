#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# svg2hvif: SVG to HVIF converter
#

add_executable(svg2hvif
    svg2hvif.cpp
    HVIFWriter.cpp
    SVGParser.cpp
)

target_include_directories(svg2hvif PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(svg2hvif PRIVATE
    hvif_common_includes
    nanosvg
)

set_target_properties(svg2hvif PROPERTIES
    OUTPUT_NAME "svg2hvif"
)
