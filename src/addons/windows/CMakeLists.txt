#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#

if(NOT WIN32)
    message(FATAL_ERROR "Windows thumbnail provider can only be built on Windows")
endif()

add_library(HVIFThumbnailProvider SHARED
    DllMain.cpp
    ClassFactory.cpp
    HVIFThumbnailProvider.cpp
    HVIFThumbnailProvider.def
)

target_include_directories(HVIFThumbnailProvider PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/import
    ${CMAKE_SOURCE_DIR}/src/export
    ${NANOSVG_DIR}
)

target_link_libraries(HVIFThumbnailProvider PRIVATE
    hvif::tools
    shlwapi
    gdi32
    uuid
)

set_target_properties(HVIFThumbnailProvider PROPERTIES
    OUTPUT_NAME "HVIFThumbnailProvider"
    SUFFIX ".dll"
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/addons"
)

if(HVIF_TOOLS_INSTALL)
    install(TARGETS HVIFThumbnailProvider
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/addons
        COMPONENT c_addons
    )
endif()

message(STATUS "Windows thumbnail provider DLL will be built")
message(STATUS "  Output: bin/addons/HVIFThumbnailProvider.dll")
message(STATUS "  Registration: regsvr32 HVIFThumbnailProvider.dll")
