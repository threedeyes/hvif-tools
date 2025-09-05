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

set(NANOSVG_DIR ${EXTERNAL_DIR}/nanosvg/src)
if(NOT EXISTS "${NANOSVG_DIR}/nanosvg.h")
  message(WARNING "nanosvg.h not found in ${NANOSVG_DIR}. Place nanosvg.h there or update NANOSVG_DIR.")
endif()

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

include(GNUInstallDirs)
install(TARGETS hvif2svg svg2hvif
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
