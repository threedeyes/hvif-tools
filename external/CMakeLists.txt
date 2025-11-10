#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# External dependencies submodules initialization
#

find_package(Git QUIET)
if(GIT_FOUND)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        RESULT_VARIABLE GIT_SUBMOD_RESULT
        OUTPUT_QUIET
        ERROR_QUIET
    )
    
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(WARNING 
            "git submodule update failed with ${GIT_SUBMOD_RESULT}. "
            "External dependencies may not be available."
        )
    endif()
endif()
