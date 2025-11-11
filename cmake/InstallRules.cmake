#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Installation rules and packaging configuration
#

if(NOT HVIF_TOOLS_INSTALL)
    return()
endif()

include(GNUInstallDirs)

set(TOOLS_TO_INSTALL "")

if(TARGET hvif2svg)
    list(APPEND TOOLS_TO_INSTALL hvif2svg)
endif()

if(TARGET svg2hvif)
    list(APPEND TOOLS_TO_INSTALL svg2hvif)
endif()

if(TARGET iom2svg)
    list(APPEND TOOLS_TO_INSTALL iom2svg)
endif()

if(TARGET svg2iom)
    list(APPEND TOOLS_TO_INSTALL svg2iom)
endif()

if(TARGET img2svg)
    list(APPEND TOOLS_TO_INSTALL img2svg)
endif()

if(TARGET msg2txt)
    list(APPEND TOOLS_TO_INSTALL msg2txt)
endif()

if(TARGET iom2hvif)
    list(APPEND TOOLS_TO_INSTALL iom2hvif)
endif()

if(TOOLS_TO_INSTALL)
    install(TARGETS ${TOOLS_TO_INSTALL}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT a_tools
    )
endif()

if(WIN32 AND TARGET HVIFThumbnailProvider)
    install(FILES $<TARGET_FILE:HVIFThumbnailProvider>
        DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT b_windows
    )
endif()

if(WIN32)
    install(FILES
        "${CMAKE_SOURCE_DIR}/inkscape/hvif_input.py"
        "${CMAKE_SOURCE_DIR}/inkscape/hvif_input.inx"
        "${CMAKE_SOURCE_DIR}/inkscape/hvif_output.py"
        "${CMAKE_SOURCE_DIR}/inkscape/hvif_output.inx"
        "${CMAKE_SOURCE_DIR}/inkscape/iom_input.py"
        "${CMAKE_SOURCE_DIR}/inkscape/iom_input.inx"
        "${CMAKE_SOURCE_DIR}/inkscape/iom_output.py"
        "${CMAKE_SOURCE_DIR}/inkscape/iom_output.inx"
        DESTINATION share/inkscape
        COMPONENT c_inkscape
    )
endif()

# Install file type icons on Windows
if(WIN32)
    if(EXISTS "${CMAKE_SOURCE_DIR}/installer/hvif-file.ico")
        install(FILES "${CMAKE_SOURCE_DIR}/installer/hvif-file.ico"
            DESTINATION installer
            COMPONENT a_tools
        )
    endif()
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/installer/iom-file.ico")
        install(FILES "${CMAKE_SOURCE_DIR}/installer/iom-file.ico"
            DESTINATION installer
            COMPONENT a_tools
        )
    endif()
endif()

set(CPACK_PACKAGE_NAME "hvif-tools")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Haiku")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")

if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    
    include(NSISConfig)
    
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "hvif-tools developer")
    set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6")
endif()

# Include CPack and define components
include(CPack)

cpack_add_component(a_tools
    DISPLAY_NAME "Command-line Tools"
    DESCRIPTION "HVIF/IOM/SVG conversion utilities"
    REQUIRED)

if(WIN32 AND TARGET HVIFThumbnailProvider)
    cpack_add_component(b_windows
        DISPLAY_NAME "Thumbnail provider"
        DESCRIPTION "Thumbnail provider for HVIF and IOM files"
        DEPENDS a_tools)
endif()

if(WIN32)
    cpack_add_component(c_inkscape
        DISPLAY_NAME "Inkscape Extensions"
        DESCRIPTION "Import/Export extensions for Inkscape (requires Inkscape installed)"
        DISABLED
        DEPENDS a_tools)
endif()
