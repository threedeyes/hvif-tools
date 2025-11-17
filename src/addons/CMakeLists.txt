#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Platform-specific addons
#

if(WIN32 AND BUILD_HVIF4WIN)
    add_subdirectory(windows)
endif()
