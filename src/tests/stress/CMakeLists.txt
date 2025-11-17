#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Stress testing utility for icon converters
#
# This tool performs intensive conversion testing between all supported
# formats (HVIF ↔ SVG ↔ IOM) to verify stability and correctness.
#

if(NOT BUILD_HVIFTOOLS_LIB)
    message(FATAL_ERROR "Stress test requires libhviftools to be built")
endif()

add_executable(hvif-stress-test
    stress.cpp
)

target_include_directories(hvif-stress-test PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/import
    ${CMAKE_SOURCE_DIR}/src/export
)

target_link_libraries(hvif-stress-test PRIVATE
    hvif::tools
)

# Enable all warnings for test code
if(MSVC)
    target_compile_options(hvif-stress-test PRIVATE /W4)
else()
    target_compile_options(hvif-stress-test PRIVATE -Wall -Wextra -Wpedantic)
endif()

# In Debug builds, enable extra diagnostics
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(hvif-stress-test PRIVATE
        DEBUG_STRESS_TEST
        VERBOSE_OUTPUT
    )
endif()

set_target_properties(hvif-stress-test PROPERTIES
    OUTPUT_NAME "hvif-stress-test"
    EXCLUDE_FROM_ALL OFF
)

# Optional installation (disabled by default for test utilities)
if(HVIF_TOOLS_INSTALL)
    install(TARGETS hvif-stress-test
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT tests
        EXCLUDE_FROM_ALL
    )
endif()

message(STATUS "Configured stress test: hvif-stress-test")
message(STATUS "  Build: cmake --build . --target hvif-stress-test")
