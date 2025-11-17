#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#

if(NOT HVIF_TOOLS_INSTALL)
    return()
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# ============================================================================
# Library: libhviftools
# ============================================================================

if(TARGET hviftools)
    if(BUILD_SHARED_LIBS)
        set(_hviftools_component a_libraries)
    else()
        set(_hviftools_component e_devel)
    endif()
    
    install(TARGETS hviftools
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT a_libraries
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT a_libraries
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT e_devel
    )
    
    install(FILES
        ${CMAKE_SOURCE_DIR}/src/common/HaikuIcon.h
        ${CMAKE_SOURCE_DIR}/src/common/IconConverter.h
        ${CMAKE_SOURCE_DIR}/src/common/IconAdapter.h
        ${CMAKE_SOURCE_DIR}/src/common/HVIFStructures.h
        ${CMAKE_SOURCE_DIR}/src/common/IOMStructures.h
        ${CMAKE_SOURCE_DIR}/src/common/Utils.h
        ${CMAKE_SOURCE_DIR}/src/common/BMessage.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hviftools/common
        COMPONENT e_devel
    )
    
    install(FILES
        ${CMAKE_SOURCE_DIR}/src/import/HVIFParser.h
        ${CMAKE_SOURCE_DIR}/src/import/IOMParser.h
        ${CMAKE_SOURCE_DIR}/src/import/SVGParser.h
        ${CMAKE_SOURCE_DIR}/src/import/PNGParser.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hviftools/import
        COMPONENT e_devel
    )
    
    install(FILES
        ${CMAKE_SOURCE_DIR}/src/export/HVIFWriter.h
        ${CMAKE_SOURCE_DIR}/src/export/IOMWriter.h
        ${CMAKE_SOURCE_DIR}/src/export/SVGWriter.h
        ${CMAKE_SOURCE_DIR}/src/export/PNGWriter.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/hviftools/export
        COMPONENT e_devel
    )
    
    configure_package_config_file(
        "${CMAKE_SOURCE_DIR}/cmake/hviftools-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/hviftools-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hviftools
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
    )
    
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/hviftools-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/hviftools-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/hviftools-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/hviftools
        COMPONENT e_devel
    )
endif()

# ============================================================================
# Library: libimagetracer
# ============================================================================

if(TARGET imagetracer)
    install(TARGETS imagetracer
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT a_libraries
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT a_libraries
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT e_devel
    )
    
    install(FILES
        ${CMAKE_SOURCE_DIR}/src/tracer/core/ImageTracer.h
        ${CMAKE_SOURCE_DIR}/src/tracer/core/TracingOptions.h
        ${CMAKE_SOURCE_DIR}/src/tracer/core/BitmapData.h
        ${CMAKE_SOURCE_DIR}/src/tracer/core/IndexedBitmap.h
        ${CMAKE_SOURCE_DIR}/src/tracer/core/VectorizationProgress.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imagetracer/core
        COMPONENT e_devel
    )
    
    install(FILES
        ${CMAKE_SOURCE_DIR}/src/tracer/output/SvgWriter.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imagetracer/output
        COMPONENT e_devel
    )
    
    install(FILES
        ${CMAKE_SOURCE_DIR}/src/tracer/quantization/ColorQuantizer.h
        ${CMAKE_SOURCE_DIR}/src/tracer/quantization/ColorCube.h
        ${CMAKE_SOURCE_DIR}/src/tracer/quantization/ColorNode.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imagetracer/quantization
        COMPONENT e_devel
    )
    
    install(FILES
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/BackgroundRemover.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/GeometryDetector.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/GradientDetector.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/PathHierarchy.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/PathScanner.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/PathSimplifier.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/PathTracer.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/RegionMerger.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/SelectiveBlur.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/SharedEdgeRegistry.h
        ${CMAKE_SOURCE_DIR}/src/tracer/processing/VisvalingamWhyatt.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imagetracer/processing
        COMPONENT e_devel
    )
    
    install(FILES
        ${CMAKE_SOURCE_DIR}/src/tracer/utils/MathUtils.h
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/imagetracer/utils
        COMPONENT e_devel
    )
    
    configure_package_config_file(
        "${CMAKE_SOURCE_DIR}/cmake/imagetracer-config.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/imagetracer-config.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imagetracer
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
    )
    
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/imagetracer-config-version.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )
    
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/imagetracer-config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/imagetracer-config-version.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/imagetracer
        COMPONENT e_devel
    )
endif()

# ============================================================================
# Windows-specific files
# ============================================================================

if(WIN32)
    if(EXISTS "${CMAKE_SOURCE_DIR}/installer/hvif-file.ico")
        install(FILES "${CMAKE_SOURCE_DIR}/installer/hvif-file.ico"
            DESTINATION installer
            COMPONENT b_tools
        )
    endif()
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/installer/iom-file.ico")
        install(FILES "${CMAKE_SOURCE_DIR}/installer/iom-file.ico"
            DESTINATION installer
            COMPONENT b_tools
        )
    endif()
    
    if(EXISTS "${CMAKE_SOURCE_DIR}/installer/hvif-tools.ico")
        install(FILES "${CMAKE_SOURCE_DIR}/installer/hvif-tools.ico"
            DESTINATION installer
            COMPONENT b_tools
        )
    endif()
    
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
        COMPONENT d_inkscape
    )
    
    if(BUILD_SHARED_LIBS)
        set(CMAKE_INSTALL_SYSTEM_RUNTIME_COMPONENT a_libraries)
        include(InstallRequiredSystemLibraries)
    endif()
endif()

# ============================================================================
# Packaging
# ============================================================================

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

include(CPack)

cpack_add_component(b_tools
    DISPLAY_NAME "Command-line Tools"
    DESCRIPTION "icon2icon, img2svg, msgdump utilities"
    REQUIRED
)

if(BUILD_SHARED_LIBS)
    cpack_add_component(a_libraries
        DISPLAY_NAME "Runtime Libraries"
        DESCRIPTION "libhviftools and libimagetracer runtime DLLs"
        REQUIRED
    )
    
    if(TARGET HVIFThumbnailProvider)
        cpack_add_component(c_addons
            DISPLAY_NAME "Thumbnail Provider"
            DESCRIPTION "Windows Explorer thumbnail provider for HVIF/IOM"
            DEPENDS a_libraries
        )
    endif()
else()
    if(TARGET HVIFThumbnailProvider)
        cpack_add_component(c_addons
            DISPLAY_NAME "Thumbnail Provider"
            DESCRIPTION "Windows Explorer thumbnail provider for HVIF/IOM"
        )
    endif()
endif()

cpack_add_component(d_inkscape
    DISPLAY_NAME "Inkscape Extensions"
    DESCRIPTION "Import/Export extensions for Inkscape"
    DISABLED
)

cpack_add_component(e_devel
    DISPLAY_NAME "Development Files"
    DESCRIPTION "Headers and import libraries for development"
    DISABLED
)

cpack_add_component(f_tests
    DISPLAY_NAME "Testing Utilities"
    DESCRIPTION "Stress testing and diagnostic tools"
    DISABLED
)
