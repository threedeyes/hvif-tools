cmake_minimum_required(VERSION 3.16)

project(hvif-tools VERSION 0.1.0 LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

option(HVIF_TOOLS_WARNINGS "Enable extra warnings" OFF)
if(HVIF_TOOLS_WARNINGS)
  if(MSVC)
    add_compile_options(/W4)
  else()
    add_compile_options(-Wall -Wextra -Wpedantic)
  endif()
endif()

set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR       ${PROJECT_ROOT}/src)
set(EXTERNAL_DIR  ${PROJECT_ROOT}/external)

set(NANOSVG_DIR ${EXTERNAL_DIR}/nanosvg_ext/src)
if(NOT EXISTS "${NANOSVG_DIR}/nanosvg.h")
  message(WARNING "nanosvg.h not found in ${NANOSVG_DIR}. Place nanosvg.h there or update NANOSVG_DIR.")
endif()

set(STB_DIR ${EXTERNAL_DIR}/stb)
if(NOT EXISTS "${STB_DIR}/stb_image.h")
  message(WARNING "stb_image.h not found in ${STB_DIR}. Place stb_image.h there or update STB_DIR.")
endif()

# Find required packages
find_package(Threads REQUIRED)

# -----------------------
# hvif2svg
# -----------------------
add_executable(hvif2svg
  ${SRC_DIR}/hvif2svg/hvif2svg.cpp
  ${SRC_DIR}/hvif2svg/HVIFParser.cpp
  ${SRC_DIR}/hvif2svg/SVGRenderer.cpp
)

target_include_directories(hvif2svg
  PRIVATE
    ${SRC_DIR}/common
    ${SRC_DIR}/hvif2svg
)

# -----------------------
# svg2hvif
# -----------------------
add_executable(svg2hvif
  ${SRC_DIR}/svg2hvif/svg2hvif.cpp
  ${SRC_DIR}/svg2hvif/HVIFWriter.cpp
  ${SRC_DIR}/svg2hvif/SVGParser.cpp
)

target_include_directories(svg2hvif
  PRIVATE
    ${SRC_DIR}/common
    ${SRC_DIR}/svg2hvif
    ${NANOSVG_DIR}
)

# -----------------------
# img2svg (ImageTracer)
# -----------------------

# Define source directories for img2svg
set(IMG2SVG_DIR ${SRC_DIR}/img2svg)
set(IMG2SVG_CORE_DIR ${IMG2SVG_DIR}/core)
set(IMG2SVG_QUANTIZATION_DIR ${IMG2SVG_DIR}/quantization)
set(IMG2SVG_PROCESSING_DIR ${IMG2SVG_DIR}/processing)
set(IMG2SVG_OUTPUT_DIR ${IMG2SVG_DIR}/output)

# Core sources
set(IMG2SVG_CORE_SOURCES
    ${IMG2SVG_CORE_DIR}/BitmapData.cpp
    ${IMG2SVG_CORE_DIR}/IndexedBitmap.cpp
    ${IMG2SVG_CORE_DIR}/TracingOptions.cpp
    ${IMG2SVG_CORE_DIR}/ImageTracer.cpp
)

# Quantization sources
set(IMG2SVG_QUANTIZATION_SOURCES
    ${IMG2SVG_QUANTIZATION_DIR}/ColorQuantizer.cpp
    ${IMG2SVG_QUANTIZATION_DIR}/ColorCube.cpp
    ${IMG2SVG_QUANTIZATION_DIR}/ColorNode.cpp
)

# Processing sources
set(IMG2SVG_PROCESSING_SOURCES
    ${IMG2SVG_PROCESSING_DIR}/SelectiveBlur.cpp
    ${IMG2SVG_PROCESSING_DIR}/PathScanner.cpp
    ${IMG2SVG_PROCESSING_DIR}/PathTracer.cpp
    ${IMG2SVG_PROCESSING_DIR}/PathSimplifier.cpp
    ${IMG2SVG_PROCESSING_DIR}/GeometryDetector.cpp
    ${IMG2SVG_PROCESSING_DIR}/BackgroundRemover.cpp
)

# Output sources
set(IMG2SVG_OUTPUT_SOURCES
    ${IMG2SVG_OUTPUT_DIR}/SvgWriter.cpp
)

# Combine all sources
set(IMG2SVG_ALL_SOURCES
    ${IMG2SVG_CORE_SOURCES}
    ${IMG2SVG_QUANTIZATION_SOURCES}
    ${IMG2SVG_PROCESSING_SOURCES}
    ${IMG2SVG_OUTPUT_SOURCES}
    ${IMG2SVG_DIR}/img2svg.cpp
)

# Create img2svg executable
add_executable(img2svg ${IMG2SVG_ALL_SOURCES})

# Set include directories for img2svg
target_include_directories(img2svg
  PRIVATE
    ${SRC_DIR}/common
    ${IMG2SVG_CORE_DIR}
    ${IMG2SVG_QUANTIZATION_DIR}
    ${IMG2SVG_PROCESSING_DIR}
    ${IMG2SVG_OUTPUT_DIR}
    ${IMG2SVG_DIR}
    ${STB_DIR}
)

# Link libraries for img2svg
target_link_libraries(img2svg PRIVATE Threads::Threads)

# Compiler specific options for img2svg
if(HVIF_TOOLS_WARNINGS)
  if(MSVC)
    target_compile_options(img2svg PRIVATE /W4)
  else()
    target_compile_options(img2svg PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endif()

# -----------------------
# Installation
# -----------------------
include(GNUInstallDirs)
install(TARGETS hvif2svg svg2hvif img2svg
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Optional: Add custom target to build all utilities
add_custom_target(all-tools
  DEPENDS hvif2svg svg2hvif img2svg
  COMMENT "Building all hvif-tools utilities"
)

# Optional: Print build information
message(STATUS "Building hvif-tools with the following utilities:")
message(STATUS "  - hvif2svg: Convert HVIF to SVG")
message(STATUS "  - svg2hvif: Convert SVG to HVIF")
message(STATUS "  - img2svg:  Convert bitmap images to SVG")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
