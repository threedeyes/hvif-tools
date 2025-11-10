#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# img2svg: Bitmap to SVG vectorization tool
#

set(IMG2SVG_SOURCES
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
    
    img2svg.cpp
)

add_executable(img2svg ${IMG2SVG_SOURCES})

target_include_directories(img2svg PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/core
    ${CMAKE_CURRENT_SOURCE_DIR}/quantization
    ${CMAKE_CURRENT_SOURCE_DIR}/processing
    ${CMAKE_CURRENT_SOURCE_DIR}/output
    ${CMAKE_CURRENT_SOURCE_DIR}/utils
)

target_link_libraries(img2svg PRIVATE
    hvif_common_includes
    stb
    Threads::Threads
)

set_target_properties(img2svg PROPERTIES
    OUTPUT_NAME "img2svg"
)
