#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#

add_executable(img2svg
    img2svg.cpp
)

target_include_directories(img2svg PRIVATE
    ${CMAKE_SOURCE_DIR}/src/tracer
    ${CMAKE_SOURCE_DIR}/src/tracer/core
    ${CMAKE_SOURCE_DIR}/src/tracer/output
    ${STB_DIR}
)

target_link_libraries(img2svg PRIVATE
    hvif::tracer
)

if(HAIKU)
    target_link_libraries(img2svg PRIVATE be translation)
endif()

set_target_properties(img2svg PROPERTIES
    OUTPUT_NAME "img2svg"
)

if(HVIF_TOOLS_INSTALL)
    install(TARGETS img2svg
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT b_tools
    )
endif()
