#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Windows thumbnail provider for HVIF files
#

if(NOT WIN32)
    message(WARNING "hvif4win can only be built on Windows")
    return()
endif()

add_library(HVIFThumbnailProvider SHARED
    DllMain.cpp
    ClassFactory.cpp
    HVIFThumbnailProvider.cpp
    
    ${CMAKE_SOURCE_DIR}/src/hvif2svg/HVIFParser.cpp
    ${CMAKE_SOURCE_DIR}/src/hvif2svg/SVGRenderer.cpp
    ${CMAKE_SOURCE_DIR}/src/iom2svg/IOMParser.cpp
    ${CMAKE_SOURCE_DIR}/src/iom2svg/SVGRenderer.cpp
    
    HVIFThumbnailProvider.def
)

target_include_directories(HVIFThumbnailProvider PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/hvif2svg
    ${CMAKE_SOURCE_DIR}/src/iom2svg
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(HVIFThumbnailProvider PRIVATE
    hvif_common_includes
    hvif::bmessage
    nanosvg
    shlwapi
    gdi32
    uuid
)

set_target_properties(HVIFThumbnailProvider PROPERTIES
    OUTPUT_NAME "HVIFThumbnailProvider"
    SUFFIX ".dll"
)

message(STATUS "Building Windows thumbnail provider DLL")
message(STATUS "COM registration must be done by installer or manually:")
message(STATUS "  regsvr32 HVIFThumbnailProvider.dll")
