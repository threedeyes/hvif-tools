#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# svg2iom: SVG to Icon-O-Matic converter
#

add_executable(svg2iom
    svg2iom.cpp
    IOMWriter.cpp
    SVGParser.cpp
)

target_include_directories(svg2iom PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(svg2iom PRIVATE
    hvif_common_includes
    hvif::bmessage
    nanosvg
)

set_target_properties(svg2iom PROPERTIES
    OUTPUT_NAME "svg2iom"
)
