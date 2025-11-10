#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# Common libraries build configuration
#

add_library(bmessage OBJECT
    BMessage.cpp
)

target_include_directories(bmessage PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

add_library(hvif::bmessage ALIAS bmessage)
