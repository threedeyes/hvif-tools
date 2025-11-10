#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# hvif2svg: HVIF to SVG converter
#

add_executable(hvif2svg
    hvif2svg.cpp
    HVIFParser.cpp
    SVGRenderer.cpp
)

target_include_directories(hvif2svg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(hvif2svg PRIVATE
    hvif_common_includes
)

set_target_properties(hvif2svg PROPERTIES
    OUTPUT_NAME "hvif2svg"
)
