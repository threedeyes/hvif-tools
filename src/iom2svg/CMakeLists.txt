#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# iom2svg: Icon-O-Matic to SVG converter
#

add_executable(iom2svg
    iom2svg.cpp
    IOMParser.cpp
    SVGRenderer.cpp
)

target_include_directories(iom2svg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(iom2svg PRIVATE
    hvif_common_includes
    hvif::bmessage
)

set_target_properties(iom2svg PROPERTIES
    OUTPUT_NAME "iom2svg"
)
