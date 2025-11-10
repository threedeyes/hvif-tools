#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#
# NSIS installer configuration
#

if(NOT WIN32)
    return()
endif()

set(CPACK_NSIS_PACKAGE_NAME "HVIF Tools")
set(CPACK_NSIS_DISPLAY_NAME "HVIF Tools ${PROJECT_VERSION}")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_MODIFY_PATH ON)

if(EXISTS "${CMAKE_SOURCE_DIR}/installer/hvif-tools.ico")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/installer/hvif-tools.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/installer/hvif-tools.ico")
    message(STATUS "Using custom installer icon")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/installer/header.bmp")
    set(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_SOURCE_DIR}/installer/header.bmp")
    message(STATUS "Using custom installer header")
endif()

set(CPACK_NSIS_INSTALLED_ICON_NAME "bin\\\\hvif2svg.exe")

set(CPACK_NSIS_COMPONENT_INSTALL ON)

set(CPACK_COMPONENT_TOOLS_DISPLAY_NAME "Command-line Tools")
set(CPACK_COMPONENT_TOOLS_DESCRIPTION "HVIF/IOM conversion utilities (hvif2svg, svg2hvif, iom2svg, svg2iom, img2svg)")
set(CPACK_COMPONENT_TOOLS_REQUIRED ON)

set(CPACK_COMPONENT_WINDOWS_DISPLAY_NAME "Windows Integration")
set(CPACK_COMPONENT_WINDOWS_DESCRIPTION "Thumbnail provider for HVIF files in Windows Explorer")
set(CPACK_COMPONENT_WINDOWS_REQUIRED OFF)

set(CPACK_COMPONENT_INKSCAPE_DISPLAY_NAME "Inkscape Extensions")
set(CPACK_COMPONENT_INKSCAPE_DESCRIPTION "Import/Export extensions for Inkscape (HVIF and IOM formats)")
set(CPACK_COMPONENT_INKSCAPE_REQUIRED OFF)

# Disable Start Menu folder creation
set(CPACK_NSIS_MENU_LINKS "")

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    WriteRegStr HKCR '.hvif' '' 'HVIFFile'
    WriteRegStr HKCR 'HVIFFile' '' 'Haiku Vector Icon Format'
    WriteRegStr HKCR 'HVIFFile\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\hvif2svg.exe,0'
    
    WriteRegStr HKCR '.iom' '' 'IOMFile'
    WriteRegStr HKCR 'IOMFile' '' 'Icon-O-Matic Format'
    WriteRegStr HKCR 'IOMFile\\\\DefaultIcon' '' '$INSTDIR\\\\bin\\\\iom2svg.exe,0'
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    DeleteRegKey HKCR '.hvif'
    DeleteRegKey HKCR 'HVIFFile'
    DeleteRegKey HKCR '.iom'
    DeleteRegKey HKCR 'IOMFile'
")

if(BUILD_HVIF4WIN)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
    
    IfFileExists '$INSTDIR\\\\bin\\\\HVIFThumbnailProvider.dll' 0 +2
        ExecWait 'regsvr32 /s \\\"$INSTDIR\\\\bin\\\\HVIFThumbnailProvider.dll\\\"'
    ")
    
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
    
    IfFileExists '$INSTDIR\\\\bin\\\\HVIFThumbnailProvider.dll' 0 +2
        ExecWait 'regsvr32 /u /s \\\"$INSTDIR\\\\bin\\\\HVIFThumbnailProvider.dll\\\"'
    ")
endif()

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
    
    IfFileExists '$INSTDIR\\\\share\\\\inkscape\\\\hvif_input.py' 0 skip_inkscape_install
    
    StrCpy \$1 '$APPDATA\\\\inkscape\\\\extensions'
    CreateDirectory '\$1'
    
    CopyFiles '$INSTDIR\\\\share\\\\inkscape\\\\*.py' '\$1'
    CopyFiles '$INSTDIR\\\\share\\\\inkscape\\\\*.inx' '\$1'
    
    WriteRegStr HKCU 'Software\\\\hvif-tools' 'InkscapeExtPath' '\$1'
    
    skip_inkscape_install:
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
    
    ReadRegStr \$1 HKCU 'Software\\\\hvif-tools' 'InkscapeExtPath'
    StrCmp '\$1' '' skip_uninst_inkscape
    
    Delete '\$1\\\\hvif_input.py'
    Delete '\$1\\\\hvif_input.inx'
    Delete '\$1\\\\hvif_output.py'
    Delete '\$1\\\\hvif_output.inx'
    Delete '\$1\\\\iom_input.py'
    Delete '\$1\\\\iom_input.inx'
    Delete '\$1\\\\iom_output.py'
    Delete '\$1\\\\iom_output.inx'
    
    DeleteRegKey HKCU 'Software\\\\hvif-tools'
    
    skip_uninst_inkscape:
")

set(CPACK_NSIS_DEFINES "RequestExecutionLevel admin")
