#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# icon2icon: Universal icon format converter (HVIF ↔ SVG ↔ IOM)
#

add_executable(icon2icon
    icon2icon.cpp
)

target_include_directories(icon2icon PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/import
    ${CMAKE_SOURCE_DIR}/src/export
)

target_link_libraries(icon2icon PRIVATE
    hvif::tools
)

set_target_properties(icon2icon PROPERTIES
    OUTPUT_NAME "icon2icon"
)

if(HVIF_TOOLS_INSTALL)
    install(TARGETS icon2icon
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT b_tools
    )
endif()
