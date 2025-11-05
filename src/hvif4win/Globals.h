/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>

// {89D4EEC4-E3CA-4441-B9EE-D960224B1202}
static const CLSID CLSID_HVIFThumbnailProvider = 
{ 0x89D4EEC4, 0xE3CA, 0x4441, { 0xB9, 0xEE, 0xD9, 0x60, 0x22, 0x4B, 0x12, 0x02 } };

extern HINSTANCE g_hInst;
extern long g_cDllRef;

#endif