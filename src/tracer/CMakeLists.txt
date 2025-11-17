#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#

set(TRACER_SOURCES
    core/BitmapData.cpp
    core/IndexedBitmap.cpp
    core/TracingOptions.cpp
    core/ImageTracer.cpp
    
    quantization/ColorQuantizer.cpp
    quantization/ColorCube.cpp
    quantization/ColorNode.cpp
    
    processing/SelectiveBlur.cpp
    processing/PathScanner.cpp
    processing/PathTracer.cpp
    processing/PathSimplifier.cpp
    processing/PathHierarchy.cpp
    processing/GeometryDetector.cpp
    processing/BackgroundRemover.cpp
    processing/VisvalingamWhyatt.cpp
    processing/GradientDetector.cpp
    processing/RegionMerger.cpp
    processing/SharedEdgeRegistry.cpp
    
    output/SvgWriter.cpp
    
    utils/MathUtils.cpp
)

add_library(imagetracer
    ${TRACER_SOURCES}
    $<TARGET_OBJECTS:vendor_impl>
)

target_include_directories(imagetracer PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/quantization>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/processing>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/output>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/utils>
    $<BUILD_INTERFACE:${STB_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/imagetracer>
)

target_link_libraries(imagetracer PUBLIC Threads::Threads)

set_target_properties(imagetracer PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    OUTPUT_NAME "imagetracer"
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(imagetracer PRIVATE IMAGETRACER_EXPORTS)
    target_compile_definitions(imagetracer INTERFACE IMAGETRACER_SHARED)
endif()

add_library(hvif::tracer ALIAS imagetracer)
