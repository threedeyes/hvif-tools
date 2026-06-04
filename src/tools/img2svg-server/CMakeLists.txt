#
# Copyright 2025-2026, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#

set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "web/index.html"
    "web/tailwindcss.js"
    "web/haikon.js"
)

file(READ "web/index.html" HTML_CONTENT)
file(READ "web/tailwindcss.js" TAILWIND_CONTENT)
file(READ "web/haikon.js" HAIKON_CONTENT)

file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/embedded_ui.h"
"/* Auto-generated file. Do not edit. */\n"
"#pragma once\n\n"
"static const char* embedded_index_html = R\"=====(\n"
"${HTML_CONTENT}\n"
")=====\";\n\n"
"static const char* embedded_tailwindcss_js = R\"=====(\n"
"${TAILWIND_CONTENT}\n"
")=====\";\n\n"
"static const char* embedded_haikon_js = R\"=====(\n"
"${HAIKON_CONTENT}\n"
")=====\";\n"
)

include_directories("${CMAKE_CURRENT_BINARY_DIR}")


add_executable(img2svg-server
    server.cpp
)

target_include_directories(img2svg-server PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/import
    ${CMAKE_SOURCE_DIR}/src/export
    ${CMAKE_SOURCE_DIR}/src/tracer
    ${CMAKE_SOURCE_DIR}/src/tracer/core
    ${CMAKE_SOURCE_DIR}/src/tracer/output
    ${STB_DIR}
    ${HTTPLIB_DIR}
)

target_link_libraries(img2svg-server PRIVATE
    hvif::tracer
    hvif::tools
)

if(UNIX AND NOT APPLE AND NOT HAIKU)
    target_link_libraries(img2svg-server PRIVATE pthread)
endif()

if(HAIKU)
    target_link_libraries(img2svg-server PRIVATE be translation)
endif()

set_target_properties(img2svg-server PROPERTIES
    OUTPUT_NAME "img2svg-server"
)

if(HVIF_TOOLS_INSTALL)
    install(TARGETS img2svg-server
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT b_tools
    )
endif()
