/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <vector>
#include <algorithm>

#include "HVIFThumbnailProvider.h"
#include "HVIFParser.h"
#include "SVGRenderer.h"
#include "Globals.h"

#define NANOSVG_IMPLEMENTATION
#include "nanosvg.h"
#define NANOSVGRAST_IMPLEMENTATION
#include "nanosvgrast.h"

HVIFThumbnailProvider::HVIFThumbnailProvider() : 
	m_cRef(1), 
	m_pStream(NULL)
{
	InterlockedIncrement(&g_cDllRef);
}

HVIFThumbnailProvider::~HVIFThumbnailProvider()
{
	if (m_pStream)
	{
		m_pStream->Release();
	}
	InterlockedDecrement(&g_cDllRef);
}

IFACEMETHODIMP
HVIFThumbnailProvider::QueryInterface(REFIID riid, void **ppv)
{
	static const QITAB qit[] = 
	{
		QITABENT(HVIFThumbnailProvider, IInitializeWithStream),
		QITABENT(HVIFThumbnailProvider, IThumbnailProvider),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG)
HVIFThumbnailProvider::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG)
HVIFThumbnailProvider::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (0 == cRef)
	{
		delete this;
	}
	return cRef;
}

IFACEMETHODIMP
HVIFThumbnailProvider::Initialize(IStream *pStream, DWORD grfMode)
{
	HRESULT hr = E_INVALIDARG;
	if (pStream)
	{
		if (m_pStream)
		{
			m_pStream->Release();
		}
		m_pStream = pStream;
		m_pStream->AddRef();
		hr = S_OK;
	}
	return hr;
}

IFACEMETHODIMP
HVIFThumbnailProvider::GetThumbnail(UINT cx, HBITMAP *phbmp, WTS_ALPHATYPE *pdwAlpha)
{
	if (!m_pStream || !phbmp)
		return E_INVALIDARG;

	*phbmp = NULL;

	STATSTG stat;
	HRESULT hr = m_pStream->Stat(&stat, STATFLAG_NONAME);
	if (FAILED(hr))
		return hr;

	ULONG fileSize = (ULONG)stat.cbSize.QuadPart;
	if (fileSize == 0 || fileSize > 1024 * 1024)
		return E_FAIL;

	std::vector<uint8_t> hvifData(fileSize);
	
	LARGE_INTEGER li = {0};
	hr = m_pStream->Seek(li, STREAM_SEEK_SET, NULL);
	if (FAILED(hr))
		return hr;

	ULONG bytesRead = 0;
	hr = m_pStream->Read(&hvifData[0], fileSize, &bytesRead);
	if (FAILED(hr) || bytesRead != fileSize)
		return E_FAIL;

	if (!hvif::HVIFParser::IsValidHVIFData(hvifData))
		return E_FAIL;

	hvif::HVIFParser parser;
	if (!parser.ParseData(hvifData))
		return E_FAIL;

	const hvif::HVIFIcon& icon = parser.GetIcon();

	hvif::SVGRenderer renderer;
	std::string svgData = renderer.RenderIcon(icon, cx, cx);

	NSVGimage* image = nsvgParse(const_cast<char*>(svgData.c_str()), "px", 96.0f);
	if (!image)
		return E_FAIL;

	float scale = 1.0f;
	if (image->width > 0 && image->height > 0)
	{
		scale = (float)cx / (std::max)(image->width, image->height);
	}

	int w = (int)(image->width * scale);
	int h = (int)(image->height * scale);
	
	if (w <= 0) w = cx;
	if (h <= 0) h = cx;

	NSVGrasterizer* rast = nsvgCreateRasterizer();
	if (!rast)
	{
		nsvgDelete(image);
		return E_FAIL;
	}

	std::vector<unsigned char> imgData(w * h * 4);
	nsvgRasterize(rast, image, 0, 0, scale, &imgData[0], w, h, w * 4);

	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);

	BITMAPINFO bmi = {0};
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bmi.bmiHeader.biWidth = w;
	bmi.bmiHeader.biHeight = -h;
	bmi.bmiHeader.biPlanes = 1;
	bmi.bmiHeader.biBitCount = 32;
	bmi.bmiHeader.biCompression = BI_RGB;

	void* pBits = NULL;
	HDC hdc = GetDC(NULL);
	HBITMAP hbmp = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
	ReleaseDC(NULL, hdc);

	if (!hbmp)
		return E_FAIL;

	unsigned char* dest = (unsigned char*)pBits;
	for (int i = 0; i < w * h; i++)
	{
		unsigned char r = imgData[i * 4 + 0];
		unsigned char g = imgData[i * 4 + 1];
		unsigned char b = imgData[i * 4 + 2];
		unsigned char a = imgData[i * 4 + 3];

		dest[i * 4 + 0] = (b * a) / 255;
		dest[i * 4 + 1] = (g * a) / 255;
		dest[i * 4 + 2] = (r * a) / 255;
		dest[i * 4 + 3] = a;
	}

	*phbmp = hbmp;
	*pdwAlpha = WTSAT_ARGB;

	return S_OK;
}
