/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#pragma once

#include <windows.h>
#include <thumbcache.h>
#include <shlwapi.h>

class HVIFThumbnailProvider : 
	public IInitializeWithStream,
	public IThumbnailProvider
{
public:
	HVIFThumbnailProvider();
	~HVIFThumbnailProvider();

	// IUnknown
	IFACEMETHODIMP			QueryInterface(REFIID riid, void **ppv);
	IFACEMETHODIMP_(ULONG)	AddRef();
	IFACEMETHODIMP_(ULONG)	Release();

	// IInitializeWithStream
	IFACEMETHODIMP			Initialize(IStream *pStream, DWORD grfMode);

	// IThumbnailProvider
	IFACEMETHODIMP			GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha);

private:
	long m_cRef;
	IStream *m_pStream;
};