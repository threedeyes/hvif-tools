/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <shlwapi.h>
#include <new>

#include "Globals.h"
#include "ClassFactory.h"
#include "HVIFThumbnailProvider.h"

ClassFactory::ClassFactory() : m_cRef(1)
{
}

ClassFactory::~ClassFactory()
{
}

IFACEMETHODIMP
ClassFactory::QueryInterface(REFIID riid, void **ppv)
{
	static const QITAB qit[] = 
	{
		QITABENT(ClassFactory, IClassFactory),
		{ 0 },
	};
	return QISearch(this, qit, riid, ppv);
}

IFACEMETHODIMP_(ULONG)
ClassFactory::AddRef()
{
	return InterlockedIncrement(&m_cRef);
}

IFACEMETHODIMP_(ULONG)
ClassFactory::Release()
{
	ULONG cRef = InterlockedDecrement(&m_cRef);
	if (0 == cRef)
	{
		delete this;
	}
	return cRef;
}

IFACEMETHODIMP
ClassFactory::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv)
{
	HRESULT hr = CLASS_E_NOAGGREGATION;

	if (pUnkOuter != NULL)
	{
		return hr;
	}

	hr = E_OUTOFMEMORY;
	HVIFThumbnailProvider *pProvider = new (std::nothrow) HVIFThumbnailProvider();
	if (pProvider)
	{
		hr = pProvider->QueryInterface(riid, ppv);
		pProvider->Release();
	}
	return hr;
}

IFACEMETHODIMP
ClassFactory::LockServer(BOOL fLock)
{
	if (fLock)
	{
		InterlockedIncrement(&g_cDllRef);
	}
	else
	{
		InterlockedDecrement(&g_cDllRef);
	}
	return S_OK;
}
