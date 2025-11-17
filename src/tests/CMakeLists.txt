#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Tests and stress testing utilities
#

if(BUILD_STRESS_TEST)
    add_subdirectory(stress)
endif()

# Placeholder for future unit tests
if(HVIF_TOOLS_TESTS)
    message(STATUS "Unit tests framework not yet implemented")
    # add_subdirectory(unit) - when ready
endif()
