#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#

add_subdirectory(vendor)

if(BUILD_HVIFTOOLS_LIB)
    add_subdirectory(common)
    add_subdirectory(import)
    add_subdirectory(export)
    
    add_library(hviftools
        $<TARGET_OBJECTS:vendor_impl>
        $<TARGET_OBJECTS:hvif_common>
        $<TARGET_OBJECTS:hvif_import>
        $<TARGET_OBJECTS:hvif_export>
    )
    
    target_include_directories(hviftools PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/common>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/import>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/export>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/tracer/core>
        $<BUILD_INTERFACE:${NANOSVG_DIR}>
        $<BUILD_INTERFACE:${STB_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/hviftools>
    )
    
    set_target_properties(hviftools PROPERTIES
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
        OUTPUT_NAME "hviftools"
    )
    
    if(BUILD_SHARED_LIBS)
        target_compile_definitions(hviftools PRIVATE HVIFTOOLS_EXPORTS)
        target_compile_definitions(hviftools INTERFACE HVIFTOOLS_SHARED)
    endif()
    
    if(BUILD_IMAGETRACER_LIB)
        target_link_libraries(hviftools PUBLIC hvif::tracer)
		if(HAIKU)
    		target_link_libraries(hviftools PUBLIC be translation)
		endif()
    endif()

    add_library(hvif::tools ALIAS hviftools)
    
    message(STATUS "Configured library: hviftools")
endif()

if(BUILD_IMAGETRACER_LIB)
    add_subdirectory(tracer)
    
    message(STATUS "Configured library: imagetracer")
endif()

if(BUILD_ICON2ICON OR BUILD_IMG2SVG OR BUILD_MSGDUMP)
    add_subdirectory(tools)
endif()

if(BUILD_HVIF4WIN)
    add_subdirectory(addons)
endif()

if(HVIF_TOOLS_TESTS OR BUILD_STRESS_TEST)
    add_subdirectory(tests)
endif()
