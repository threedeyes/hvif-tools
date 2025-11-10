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
        COMPONENT tools
    )
endif()

if(WIN32 AND TARGET HVIFThumbnailProvider)
    install(TARGETS HVIFThumbnailProvider
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        COMPONENT windows
    )
    
    message(STATUS "Windows thumbnail provider will be installed.")
    message(STATUS "Use an installer (NSIS/WiX) to register the COM component.")
endif()

install(FILES
    "${CMAKE_SOURCE_DIR}/README.md"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT documentation
    OPTIONAL
)

if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    install(FILES
        "${CMAKE_SOURCE_DIR}/LICENSE"
        DESTINATION ${CMAKE_INSTALL_DOCDIR}
        COMPONENT documentation
    )
endif()

set(CPACK_PACKAGE_NAME "hvif-tools")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_VENDOR "Haiku")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_COMPONENTS_ALL tools documentation)

if(EXISTS "${CMAKE_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/README.md")
    set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/README.md")
endif()

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
    list(APPEND CPACK_COMPONENTS_ALL windows)
    
    set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_DISPLAY_NAME "HVIF Tools")
    set(CPACK_NSIS_PACKAGE_NAME "HVIF Tools")
    
elseif(HAIKU)
    set(CPACK_GENERATOR "TGZ;HPKG")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
    
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "hvif-tools developers")
    set(CPACK_DEBIAN_PACKAGE_SECTION "graphics")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6, libstdc++6")
endif()

include(CPack)
