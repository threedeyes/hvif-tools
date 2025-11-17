#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# CLI tools
#

if(BUILD_ICON2ICON)
    add_subdirectory(icon2icon)
endif()

if(BUILD_IMG2SVG)
    add_subdirectory(img2svg)
endif()

if(BUILD_MSGDUMP)
    add_subdirectory(msgdump)
endif()
