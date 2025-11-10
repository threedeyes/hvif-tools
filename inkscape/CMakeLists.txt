#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Inkscape extensions installation
#

if(NOT BUILD_INKSCAPE_EXTENSIONS)
    return()
endif()

# For Windows, extensions are packaged in share/inkscape and installed by NSIS
# For other platforms, install directly to Inkscape extensions directory
if(WIN32)
    message(STATUS "Inkscape extensions will be packaged for NSIS installer")
    return()
endif()

if(APPLE)
    set(_default_ext_dir "$ENV{HOME}/Library/Application Support/org.inkscape.Inkscape/config/inkscape/extensions")
elseif(HAIKU)
    set(_default_ext_dir "$ENV{HOME}/config/settings/inkscape/extensions")
else()
    set(_default_ext_dir "$ENV{HOME}/.config/inkscape/extensions")
endif()

set(INKSCAPE_EXT_DIR "${_default_ext_dir}" CACHE PATH 
    "Inkscape extensions directory"
)

message(STATUS "Inkscape extensions directory: ${INKSCAPE_EXT_DIR}")

set(HVIF_EXTENSION_FILES
    hvif_input.py
    hvif_input.inx
    hvif_output.py
    hvif_output.inx
)

set(IOM_EXTENSION_FILES
    iom_input.py
    iom_input.inx
    iom_output.py
    iom_output.inx
)

install(FILES
    ${HVIF_EXTENSION_FILES}
    ${IOM_EXTENSION_FILES}
    DESTINATION ${INKSCAPE_EXT_DIR}
    COMPONENT inkscape
)

if(UNIX)
    install(CODE "
        file(CHMOD 
            \${CMAKE_INSTALL_PREFIX}/${INKSCAPE_EXT_DIR}/hvif_input.py
            \${CMAKE_INSTALL_PREFIX}/${INKSCAPE_EXT_DIR}/hvif_output.py
            \${CMAKE_INSTALL_PREFIX}/${INKSCAPE_EXT_DIR}/iom_input.py
            \${CMAKE_INSTALL_PREFIX}/${INKSCAPE_EXT_DIR}/iom_output.py
            PERMISSIONS 
                OWNER_READ OWNER_WRITE OWNER_EXECUTE
                GROUP_READ GROUP_EXECUTE
                WORLD_READ WORLD_EXECUTE
        )
    " COMPONENT inkscape)
endif()

message(STATUS "Inkscape extensions require the following CLI tools:")
message(STATUS "  hvif2svg, svg2hvif (for HVIF format)")
message(STATUS "  iom2svg, svg2iom (for Icon-O-Matic format)")
message(STATUS "")
message(STATUS "Make sure these tools are installed and available in PATH.")
