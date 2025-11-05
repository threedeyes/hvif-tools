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
# img2svg
# -----------------------

set(IMG2SVG_DIR ${SRC_DIR}/img2svg)
set(IMG2SVG_CORE_DIR ${IMG2SVG_DIR}/core)
set(IMG2SVG_QUANTIZATION_DIR ${IMG2SVG_DIR}/quantization)
set(IMG2SVG_PROCESSING_DIR ${IMG2SVG_DIR}/processing)
set(IMG2SVG_OUTPUT_DIR ${IMG2SVG_DIR}/output)

set(IMG2SVG_CORE_SOURCES
    ${IMG2SVG_CORE_DIR}/BitmapData.cpp
    ${IMG2SVG_CORE_DIR}/IndexedBitmap.cpp
    ${IMG2SVG_CORE_DIR}/TracingOptions.cpp
    ${IMG2SVG_CORE_DIR}/ImageTracer.cpp
)

set(IMG2SVG_QUANTIZATION_SOURCES
    ${IMG2SVG_QUANTIZATION_DIR}/ColorQuantizer.cpp
    ${IMG2SVG_QUANTIZATION_DIR}/ColorCube.cpp
    ${IMG2SVG_QUANTIZATION_DIR}/ColorNode.cpp
)

set(IMG2SVG_PROCESSING_SOURCES
    ${IMG2SVG_PROCESSING_DIR}/SelectiveBlur.cpp
    ${IMG2SVG_PROCESSING_DIR}/PathScanner.cpp
    ${IMG2SVG_PROCESSING_DIR}/PathTracer.cpp
    ${IMG2SVG_PROCESSING_DIR}/PathSimplifier.cpp
    ${IMG2SVG_PROCESSING_DIR}/GeometryDetector.cpp
    ${IMG2SVG_PROCESSING_DIR}/BackgroundRemover.cpp
    ${IMG2SVG_PROCESSING_DIR}/VisvalingamWhyatt.cpp
    ${IMG2SVG_PROCESSING_DIR}/GradientDetector.cpp
)

set(IMG2SVG_OUTPUT_SOURCES
    ${IMG2SVG_OUTPUT_DIR}/SvgWriter.cpp
)

set(IMG2SVG_ALL_SOURCES
    ${IMG2SVG_CORE_SOURCES}
    ${IMG2SVG_QUANTIZATION_SOURCES}
    ${IMG2SVG_PROCESSING_SOURCES}
    ${IMG2SVG_OUTPUT_SOURCES}
    ${IMG2SVG_DIR}/img2svg.cpp
)

add_executable(img2svg ${IMG2SVG_ALL_SOURCES})

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

target_link_libraries(img2svg PRIVATE Threads::Threads)

if(HVIF_TOOLS_WARNINGS)
  if(MSVC)
    target_compile_options(img2svg PRIVATE /W4)
  else()
    target_compile_options(img2svg PRIVATE -Wall -Wextra -Wpedantic)
  endif()
endif()

# ---------------------------
# Windows Thumbnail Provider
# ---------------------------
if(WIN32)
  option(BUILD_THUMBNAIL_PROVIDER "Build Windows thumbnail provider for HVIF files" ON)

  if(BUILD_THUMBNAIL_PROVIDER)
    message(STATUS "Building Windows thumbnail provider for HVIF files")

    set(HVIF4WIN_DIR ${SRC_DIR}/hvif4win)

    set(HVIF4WIN_SOURCES
      ${HVIF4WIN_DIR}/DllMain.cpp
      ${HVIF4WIN_DIR}/ClassFactory.cpp
      ${HVIF4WIN_DIR}/HVIFThumbnailProvider.cpp
      ${SRC_DIR}/hvif2svg/HVIFParser.cpp
      ${SRC_DIR}/hvif2svg/SVGRenderer.cpp
    )

    set(HVIF4WIN_HEADERS
      ${HVIF4WIN_DIR}/Globals.h
      ${HVIF4WIN_DIR}/ClassFactory.h
      ${HVIF4WIN_DIR}/HVIFThumbnailProvider.h
    )

    add_library(HVIFThumbnailProvider SHARED
      ${HVIF4WIN_SOURCES}
      ${HVIF4WIN_HEADERS}
      ${HVIF4WIN_DIR}/HVIFThumbnailProvider.def
    )

    target_include_directories(HVIFThumbnailProvider
      PRIVATE
        ${HVIF4WIN_DIR}
        ${SRC_DIR}/common
        ${SRC_DIR}/hvif2svg
        ${NANOSVG_DIR}
    )

    target_link_libraries(HVIFThumbnailProvider
      shlwapi
      gdi32
      uuid
    )

    target_compile_definitions(HVIFThumbnailProvider PRIVATE
      UNICODE
      _UNICODE
      WIN32_LEAN_AND_MEAN
    )

    set_target_properties(HVIFThumbnailProvider PROPERTIES
      OUTPUT_NAME "HVIFThumbnailProvider"
      SUFFIX ".dll"
    )

    if(HVIF_TOOLS_WARNINGS)
      if(MSVC)
        target_compile_options(HVIFThumbnailProvider PRIVATE /W4)
      endif()
    endif()

    # Optional: Create registration/unregistration batch scripts
    if(CMAKE_CONFIGURATION_TYPES)
      set(DLL_OUTPUT_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/$<CONFIG>")
    else()
      set(DLL_OUTPUT_PATH "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
    endif()

    # Generate register script
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/register_provider.bat"
      "@echo off\n"
      "echo Registering HVIF Thumbnail Provider...\n"
      "echo.\n"
      "echo This requires Administrator privileges!\n"
      "echo.\n"
      "regsvr32 /s \"${DLL_OUTPUT_PATH}/HVIFThumbnailProvider.dll\"\n"
      "if %ERRORLEVEL% EQU 0 (\n"
      "    echo Successfully registered!\n"
      "    echo Restarting Windows Explorer...\n"
      "    taskkill /f /im explorer.exe >nul 2>&1\n"
      "    start explorer.exe\n"
      "    echo Done!\n"
      ") else (\n"
      "    echo Registration failed! Please run as Administrator.\n"
      ")\n"
      "pause\n"
    )

    # Generate unregister script
    file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/unregister_provider.bat"
      "@echo off\n"
      "echo Unregistering HVIF Thumbnail Provider...\n"
      "echo.\n"
      "regsvr32 /u /s \"${DLL_OUTPUT_PATH}/HVIFThumbnailProvider.dll\"\n"
      "if %ERRORLEVEL% EQU 0 (\n"
      "    echo Successfully unregistered!\n"
      "    echo Restarting Windows Explorer...\n"
      "    taskkill /f /im explorer.exe >nul 2>&1\n"
      "    start explorer.exe\n"
      "    echo Done!\n"
      ") else (\n"
      "    echo Unregistration failed!\n"
      ")\n"
      "pause\n"
    )

  endif()
endif()

# -----------------------
# Installation
# -----------------------
include(GNUInstallDirs)

# Install executables
install(TARGETS hvif2svg svg2hvif img2svg
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install Windows thumbnail provider
if(WIN32 AND BUILD_THUMBNAIL_PROVIDER)
  install(TARGETS HVIFThumbnailProvider
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  )

  install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/register_provider.bat"
    "${CMAKE_CURRENT_BINARY_DIR}/unregister_provider.bat"
    DESTINATION ${CMAKE_INSTALL_BINDIR}
  )
endif()

# Optional: Add custom target to build all utilities
if(WIN32 AND BUILD_THUMBNAIL_PROVIDER)
  add_custom_target(all-tools
    DEPENDS hvif2svg svg2hvif img2svg HVIFThumbnailProvider
    COMMENT "Building all hvif-tools utilities including Windows thumbnail provider"
  )
else()
  add_custom_target(all-tools
    DEPENDS hvif2svg svg2hvif img2svg
    COMMENT "Building all hvif-tools utilities"
  )
endif()

# Optional: Print build information
message(STATUS "Building hvif-tools with the following utilities:")
message(STATUS "  - hvif2svg: Convert HVIF to SVG")
message(STATUS "  - svg2hvif: Convert SVG to HVIF")
message(STATUS "  - img2svg:  Convert bitmap images to SVG")
if(WIN32 AND BUILD_THUMBNAIL_PROVIDER)
  message(STATUS "  - HVIFThumbnailProvider: Windows thumbnail provider for HVIF files")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
