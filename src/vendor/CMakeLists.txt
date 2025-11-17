#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#

add_library(vendor_impl OBJECT
    nanosvg_impl.cpp
    nanosvgrast_impl.cpp
    stb_image_impl.cpp
    stb_image_write_impl.cpp
)

target_include_directories(vendor_impl PUBLIC
    ${NANOSVG_DIR}
    ${STB_DIR}
)
