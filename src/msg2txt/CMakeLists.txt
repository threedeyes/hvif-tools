#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# msg2txt: BMessage dump utility
#

add_executable(msg2txt
    msg2txt.cpp
)

target_include_directories(msg2txt PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(msg2txt PRIVATE
    hvif_common_includes
    hvif::bmessage
)

set_target_properties(msg2txt PROPERTIES
    OUTPUT_NAME "msg2txt"
)
