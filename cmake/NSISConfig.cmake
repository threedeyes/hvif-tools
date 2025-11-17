#
# Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
# Distributed under the terms of the MIT License.
#

if(NOT WIN32)
    return()
endif()

set(CPACK_PACKAGE_INSTALL_DIRECTORY "HVIF-Tools")

set(CPACK_NSIS_PACKAGE_NAME "HVIF Tools")
set(CPACK_NSIS_DISPLAY_NAME "HVIF Tools ${PROJECT_VERSION}")
set(CPACK_NSIS_INSTALL_ROOT "$PROGRAMFILES64")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)
set(CPACK_NSIS_DEFINES "RequestExecutionLevel admin")
set(CPACK_NSIS_MODIFY_PATH OFF)

if(EXISTS "${CMAKE_SOURCE_DIR}/installer/hvif-tools.ico")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_SOURCE_DIR}/installer/hvif-tools.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_SOURCE_DIR}/installer/hvif-tools.ico")
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/installer/header.bmp")
    set(CPACK_NSIS_MUI_HEADERIMAGE "${CMAKE_SOURCE_DIR}/installer/header.bmp")
endif()

set(CPACK_NSIS_INSTALLED_ICON_NAME "installer\\\\hvif-tools.ico")

set(CPACK_NSIS_COMPONENT_INSTALL ON)

set(CPACK_NSIS_MENU_LINKS "")

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "
    DetailPrint 'Adding to system PATH...'
    ReadRegStr \$R0 HKLM 'SYSTEM\\\\CurrentControlSet\\\\Control\\\\Session Manager\\\\Environment' 'Path'
    StrCpy \$R1 '\$INSTDIR\\\\bin'
    
    StrLen \$R3 \$R1
    StrCpy \$R4 0
    StrCpy \$R5 ''
    
    path_search_loop:
        StrCpy \$R5 \$R0 \$R3 \$R4
        StrCmp \$R5 '' path_not_found
        StrCmp \$R5 \$R1 path_already_added
        IntOp \$R4 \$R4 + 1
        Goto path_search_loop
    
    path_not_found:
    StrCpy \$R0 '\$R0;\$R1'
    WriteRegExpandStr HKLM 'SYSTEM\\\\CurrentControlSet\\\\Control\\\\Session Manager\\\\Environment' 'Path' '\$R0'
    DetailPrint 'Added to PATH: \$R1'
    SendMessage 0xFFFF 0x001A 0 'STR:Environment' \$0 /TIMEOUT=5000
    Goto path_done
    
    path_already_added:
    DetailPrint 'Already in PATH: \$R1'
    
    path_done:
    
    DetailPrint 'Registering HVIF file type...'
    ReadRegStr \$R0 HKCR '.hvif' ''
    StrCmp \$R0 '' hvif_no_backup_needed
        StrCmp \$R0 'HVIFFile' hvif_no_backup_needed
        WriteRegStr HKCR '.hvif' 'hvif_tools_backup' '\$R0'
    hvif_no_backup_needed:
    WriteRegStr HKCR '.hvif' '' 'HVIFFile'
    WriteRegStr HKCR '.hvif' 'Content Type' 'image/x-hvif'
    WriteRegStr HKCR '.hvif' 'PerceivedType' 'image'
    WriteRegStr HKCR 'HVIFFile' '' 'Haiku Vector Icon Format'
    WriteRegStr HKCR 'HVIFFile' 'FriendlyTypeName' 'HVIF Vector Image'
    WriteRegStr HKCR 'HVIFFile\\\\DefaultIcon' '' '\$INSTDIR\\\\installer\\\\hvif-file.ico,0'
    
    DetailPrint 'Registering IOM file type...'
    ReadRegStr \$R0 HKCR '.iom' ''
    StrCmp \$R0 '' iom_no_backup_needed
        StrCmp \$R0 'IOMFile' iom_no_backup_needed
        WriteRegStr HKCR '.iom' 'hvif_tools_backup' '\$R0'
    iom_no_backup_needed:
    WriteRegStr HKCR '.iom' '' 'IOMFile'
    WriteRegStr HKCR '.iom' 'Content Type' 'application/x-vnd.haiku-icon'
    WriteRegStr HKCR '.iom' 'PerceivedType' 'image'
    WriteRegStr HKCR 'IOMFile' '' 'Icon-O-Matic Format'
    WriteRegStr HKCR 'IOMFile' 'FriendlyTypeName' 'Icon-O-Matic Project'
    WriteRegStr HKCR 'IOMFile\\\\DefaultIcon' '' '\$INSTDIR\\\\installer\\\\iom-file.ico,0'
    
    DetailPrint 'Installing icon files...'
    CreateDirectory '\$INSTDIR\\\\installer'
    SetOutPath '\$INSTDIR\\\\installer'
")

if(EXISTS "${CMAKE_SOURCE_DIR}/installer")
    file(GLOB ICON_FILES "${CMAKE_SOURCE_DIR}/installer/*.ico")
    foreach(ICON_FILE ${ICON_FILES})
        get_filename_component(ICON_NAME ${ICON_FILE} NAME)
        install(FILES ${ICON_FILE} DESTINATION installer COMPONENT b_tools)
    endforeach()
endif()

if(BUILD_SHARED_LIBS)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
    
    DetailPrint 'Verifying runtime libraries...'
    IfFileExists '\$INSTDIR\\\\bin\\\\hviftools.dll' 0 missing_hviftools
    DetailPrint 'Found hviftools.dll'
    Goto check_imagetracer
    missing_hviftools:
    MessageBox MB_OK|MB_ICONEXCLAMATION 'Warning: hviftools.dll not found. Tools may not work correctly.'
    
    check_imagetracer:
    IfFileExists '\$INSTDIR\\\\bin\\\\imagetracer.dll' 0 missing_imagetracer
    DetailPrint 'Found imagetracer.dll'
    Goto runtime_check_done
    missing_imagetracer:
    MessageBox MB_OK|MB_ICONEXCLAMATION 'Warning: imagetracer.dll not found. img2svg may not work correctly.'
    
    runtime_check_done:
    ")
endif()

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
    DetailPrint 'Refreshing shell icons...'
    System::Call 'shell32.dll::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "
    DetailPrint 'Removing from system PATH...'
    ReadRegStr \$R0 HKLM 'SYSTEM\\\\CurrentControlSet\\\\Control\\\\Session Manager\\\\Environment' 'Path'
    StrCpy \$R1 '\$INSTDIR\\\\bin'
    StrCpy \$R7 \$R0
    
    StrLen \$R2 \$R1
    StrCpy \$R3 0
    StrCpy \$R6 ''
    
    unpath_loop:
        StrCpy \$R4 \$R7 \$R2 \$R3
        StrCmp \$R4 '' unpath_done
        StrCmp \$R4 \$R1 unpath_found
        IntOp \$R3 \$R3 + 1
        Goto unpath_loop
    
    unpath_found:
        StrCpy \$R6 \$R7 \$R3
        IntOp \$R3 \$R3 + \$R2
        StrCpy \$R5 \$R7 1 \$R3
        StrCmp \$R5 ';' 0 +2
            IntOp \$R3 \$R3 + 1
        StrCpy \$R7 \$R7 '' \$R3
        StrCpy \$R7 '\$R6\$R7'
        StrCpy \$R3 \$R6
        StrLen \$R3 \$R3
        Goto unpath_loop
    
    unpath_done:
        StrCpy \$R0 \$R7
        StrCpy \$R3 0
        StrLen \$R2 \$R0
        IntOp \$R2 \$R2 - 1
        StrCpy \$R4 \$R0 1 \$R2
        StrCmp \$R4 ';' 0 +2
            StrCpy \$R0 \$R0 \$R2
    
    WriteRegExpandStr HKLM 'SYSTEM\\\\CurrentControlSet\\\\Control\\\\Session Manager\\\\Environment' 'Path' '\$R0'
    SendMessage 0xFFFF 0x001A 0 'STR:Environment' \$0 /TIMEOUT=5000
    
    DetailPrint 'Removing HVIF file type registration...'
    ReadRegStr \$R0 HKCR '.hvif' 'hvif_tools_backup'
    StrCmp \$R0 '' hvif_remove_key
        WriteRegStr HKCR '.hvif' '' '\$R0'
        DeleteRegValue HKCR '.hvif' 'hvif_tools_backup'
        DeleteRegValue HKCR '.hvif' 'Content Type'
        DeleteRegValue HKCR '.hvif' 'PerceivedType'
        Goto hvif_cleanup_progid
    hvif_remove_key:
        DeleteRegKey HKCR '.hvif'
    hvif_cleanup_progid:
    DeleteRegKey HKCR 'HVIFFile'
    
    DetailPrint 'Removing IOM file type registration...'
    ReadRegStr \$R0 HKCR '.iom' 'hvif_tools_backup'
    StrCmp \$R0 '' iom_remove_key
        WriteRegStr HKCR '.iom' '' '\$R0'
        DeleteRegValue HKCR '.iom' 'hvif_tools_backup'
        DeleteRegValue HKCR '.iom' 'Content Type'
        DeleteRegValue HKCR '.iom' 'PerceivedType'
        Goto iom_cleanup_progid
    iom_remove_key:
        DeleteRegKey HKCR '.iom'
    iom_cleanup_progid:
    DeleteRegKey HKCR 'IOMFile'
    
    DetailPrint 'Removing icon files...'
    Delete '\$INSTDIR\\\\installer\\\\hvif-file.ico'
    Delete '\$INSTDIR\\\\installer\\\\iom-file.ico'
    Delete '\$INSTDIR\\\\installer\\\\hvif-tools.ico'
    RMDir '\$INSTDIR\\\\installer'
    System::Call 'shell32.dll::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
")

if(TARGET HVIFThumbnailProvider)
    set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
    
    DetailPrint 'Registering thumbnail provider...'
    IfFileExists '\$INSTDIR\\\\bin\\\\addons\\\\HVIFThumbnailProvider.dll' 0 skip_thumbnail_reg
    ExecWait '\\\"regsvr32\\\" /s \\\"\$INSTDIR\\\\bin\\\\addons\\\\HVIFThumbnailProvider.dll\\\"' \$R0
    StrCmp \$R0 '0' thumbnail_reg_success
        DetailPrint 'Failed to register thumbnail provider'
        Goto skip_thumbnail_reg
    thumbnail_reg_success:
    DetailPrint 'Thumbnail provider registered successfully'
    skip_thumbnail_reg:
    ")
    
    set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
    
    DetailPrint 'Unregistering thumbnail provider...'
    IfFileExists '\$INSTDIR\\\\bin\\\\addons\\\\HVIFThumbnailProvider.dll' 0 skip_thumbnail_unreg
    ExecWait '\\\"regsvr32\\\" /u /s \\\"\$INSTDIR\\\\bin\\\\addons\\\\HVIFThumbnailProvider.dll\\\"' \$R0
    StrCmp \$R0 '0' thumbnail_unreg_success
        DetailPrint 'Warning: Failed to unregister thumbnail provider'
        Goto skip_thumbnail_unreg
    thumbnail_unreg_success:
    DetailPrint 'Thumbnail provider unregistered successfully'
    skip_thumbnail_unreg:
    ")
endif()

set(CPACK_NSIS_EXTRA_INSTALL_COMMANDS "${CPACK_NSIS_EXTRA_INSTALL_COMMANDS}
    
    IfFileExists '\$INSTDIR\\\\share\\\\inkscape\\\\hvif_input.py' 0 skip_inkscape_install
    DetailPrint 'Installing Inkscape extensions...'
    
    ClearErrors
    ReadRegStr \$R1 HKLM 'SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\App Paths\\\\inkscape.exe' 'Path'
    IfErrors 0 inkscape_check_path
        ReadRegStr \$R2 HKLM 'SOFTWARE\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\App Paths\\\\inkscape.exe' ''
        IfErrors try_standard_locations
            StrCpy \$R1 \$R2
            StrLen \$R3 \$R1
            find_last_slash:
                IntOp \$R3 \$R3 - 1
                IntCmp \$R3 0 try_standard_locations
                StrCpy \$R4 \$R1 1 \$R3
                StrCmp \$R4 '\\\\' found_slash
                Goto find_last_slash
            found_slash:
                StrCpy \$R1 \$R1 \$R3
                StrLen \$R3 \$R1
                IntOp \$R3 \$R3 - 4
                StrCpy \$R4 \$R1 4 \$R3
                StrCmp \$R4 '\\\\bin' 0 +2
                    StrCpy \$R1 \$R1 \$R3
                Goto inkscape_check_path
    
    try_standard_locations:
    IfFileExists '\$PROGRAMFILES64\\\\Inkscape\\\\bin\\\\inkscape.exe' 0 +3
        StrCpy \$R1 '\$PROGRAMFILES64\\\\Inkscape'
        Goto inkscape_path_found
    
    IfFileExists '\$PROGRAMFILES\\\\Inkscape\\\\bin\\\\inkscape.exe' 0 +3
        StrCpy \$R1 '\$PROGRAMFILES\\\\Inkscape'
        Goto inkscape_path_found
    
    DetailPrint 'Inkscape installation not found'
    Goto skip_inkscape_install
    
    inkscape_check_path:
    IfFileExists '\$R1\\\\bin\\\\inkscape.exe' inkscape_path_found
        DetailPrint 'Invalid Inkscape path: \$R1'
        Goto try_standard_locations
    
    inkscape_path_found:
    DetailPrint 'Found Inkscape installation at: \$R1'
    
    StrCpy \$R3 '\$R1\\\\share\\\\inkscape\\\\extensions'
    DetailPrint 'Extensions directory: \$R3'
    IfFileExists '\$R3\\\\*.*' 0 inkscape_ext_dir_not_found
    
    DetailPrint 'Copying extension files...'
    CopyFiles /SILENT '\$INSTDIR\\\\share\\\\inkscape\\\\*.py' '\$R3\\\\'
    CopyFiles /SILENT '\$INSTDIR\\\\share\\\\inkscape\\\\*.inx' '\$R3\\\\'
    
    DetailPrint 'Extensions installed successfully to: \$R3'
    
    WriteRegStr HKCU 'Software\\\\hvif-tools' 'InkscapeExtPath' '\$R3'
    WriteRegStr HKCU 'Software\\\\hvif-tools' 'InkscapePath' '\$R1'
    
    StrCpy \$R4 '\$R1\\\\bin\\\\inkscape.exe'
    IfFileExists '\$R4' inkscape_exe_found
        DetailPrint 'Warning: inkscape.exe not found'
        Goto skip_file_associations
    inkscape_exe_found:
    DetailPrint 'Found Inkscape executable: \$R4'
    
    DetailPrint 'Registering Inkscape as default application for HVIF/IOM files...'
    WriteRegStr HKCR 'HVIFFile\\\\shell' '' 'open'
    WriteRegStr HKCR 'HVIFFile\\\\shell\\\\open' '' 'Open with Inkscape'
    WriteRegStr HKCR 'HVIFFile\\\\shell\\\\open\\\\command' '' '\\\"\$R4\\\" \\\"%1\\\"'
    WriteRegStr HKCR 'HVIFFile\\\\shell\\\\open' 'Icon' '\\\"\$R4\\\",0'
    WriteRegStr HKCR 'HVIFFile\\\\shell\\\\edit' '' 'Edit with Inkscape'
    WriteRegStr HKCR 'HVIFFile\\\\shell\\\\edit\\\\command' '' '\\\"\$R4\\\" \\\"%1\\\"'
    WriteRegStr HKCR 'HVIFFile\\\\shell\\\\edit' 'Icon' '\\\"\$R4\\\",0'
    
    WriteRegStr HKCR 'IOMFile\\\\shell' '' 'open'
    WriteRegStr HKCR 'IOMFile\\\\shell\\\\open' '' 'Open with Inkscape'
    WriteRegStr HKCR 'IOMFile\\\\shell\\\\open\\\\command' '' '\\\"\$R4\\\" \\\"%1\\\"'
    WriteRegStr HKCR 'IOMFile\\\\shell\\\\open' 'Icon' '\\\"\$R4\\\",0'
    WriteRegStr HKCR 'IOMFile\\\\shell\\\\edit' '' 'Edit with Inkscape'
    WriteRegStr HKCR 'IOMFile\\\\shell\\\\edit\\\\command' '' '\\\"\$R4\\\" \\\"%1\\\"'
    WriteRegStr HKCR 'IOMFile\\\\shell\\\\edit' 'Icon' '\\\"\$R4\\\",0'
    
    WriteRegStr HKCR 'Applications\\\\inkscape.exe\\\\SupportedTypes' '.hvif' ''
    WriteRegStr HKCR 'Applications\\\\inkscape.exe\\\\SupportedTypes' '.iom' ''
    DetailPrint 'File associations configured successfully'
    
    skip_file_associations:
    System::Call 'shell32.dll::SHChangeNotify(i 0x08000000, i 0, i 0, i 0)'
    DetailPrint 'Inkscape extensions installation completed'
    Goto skip_inkscape_install
    
    inkscape_ext_dir_not_found:
    DetailPrint 'Inkscape extensions directory not found: \$R3'
    skip_inkscape_install:
")

set(CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS "${CPACK_NSIS_EXTRA_UNINSTALL_COMMANDS}
    
    DetailPrint 'Removing Inkscape extensions...'
    ClearErrors
    ReadRegStr \$R1 HKCU 'Software\\\\hvif-tools' 'InkscapeExtPath'
    IfErrors skip_inkscape_uninstall
    
    DetailPrint 'Removing extensions from: \$R1'
    
    Delete '\$R1\\\\hvif_input.py'
    Delete '\$R1\\\\hvif_input.inx'
    Delete '\$R1\\\\hvif_output.py'
    Delete '\$R1\\\\hvif_output.inx'
    Delete '\$R1\\\\iom_input.py'
    Delete '\$R1\\\\iom_input.inx'
    Delete '\$R1\\\\iom_output.py'
    Delete '\$R1\\\\iom_output.inx'
    
    DeleteRegKey HKCR 'HVIFFile\\\\shell\\\\open'
    DeleteRegKey HKCR 'HVIFFile\\\\shell\\\\edit'
    DeleteRegKey HKCR 'IOMFile\\\\shell\\\\open'
    DeleteRegKey HKCR 'IOMFile\\\\shell\\\\edit'
    DeleteRegValue HKCR 'Applications\\\\inkscape.exe\\\\SupportedTypes' '.hvif'
    DeleteRegValue HKCR 'Applications\\\\inkscape.exe\\\\SupportedTypes' '.iom'
    DeleteRegKey HKCU 'Software\\\\hvif-tools'
    
    DetailPrint 'Inkscape extensions removed successfully'
    skip_inkscape_uninstall:
")
