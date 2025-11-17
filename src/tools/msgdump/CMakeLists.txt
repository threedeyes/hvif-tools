#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# msgdump: BMessage dump utility
#

add_executable(msgdump
    msgdump.cpp
)

target_include_directories(msgdump PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/common
)

# Only needs BMessage from common
target_sources(msgdump PRIVATE
    ${CMAKE_SOURCE_DIR}/src/common/BMessage.cpp
)

set_target_properties(msgdump PROPERTIES
    OUTPUT_NAME "msgdump"
)

if(HVIF_TOOLS_INSTALL)
    install(TARGETS msgdump
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        COMPONENT b_tools
    )
endif()
