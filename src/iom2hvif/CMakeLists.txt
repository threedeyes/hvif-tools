#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# iom2hvif: Icon-O-Matic to HVIF converter (Haiku native)
#

if(NOT HAIKU)
    message(WARNING "iom2hvif can only be built on Haiku")
    return()
endif()

set(IOM2HVIF_SOURCES
    iom2hvif.cpp
    
    core/Icon.cpp
    core/Style.cpp
    core/VectorPath.cpp
    core/Shape.cpp
    core/Transformable.cpp
    
    format/LittleEndianBuffer.cpp
    format/FlatIconFormat.cpp
    format/PathCommandQueue.cpp
    
    transform/Transformers.cpp
    
    io/MessageImporter.cpp
    io/FlatIconExporter.cpp
)

add_executable(iom2hvif ${IOM2HVIF_SOURCES})

target_include_directories(iom2hvif PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/format
    ${CMAKE_CURRENT_SOURCE_DIR}/transform
    ${CMAKE_CURRENT_SOURCE_DIR}/io
)

target_link_libraries(iom2hvif PRIVATE
    ${BE_LIBRARY}
    haiku_agg
)

set_target_properties(iom2hvif PROPERTIES
    OUTPUT_NAME "iom2hvif"
)

if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(iom2hvif PRIVATE -O3)
endif()
