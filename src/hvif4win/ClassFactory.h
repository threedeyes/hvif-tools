/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#pragma once

#include <windows.h>
#include <unknwn.h>

class ClassFactory : public IClassFactory
{
public:
	ClassFactory();
	~ClassFactory();

	// IUnknown
	IFACEMETHODIMP			QueryInterface(REFIID riid, void **ppv);
	IFACEMETHODIMP_(ULONG)	AddRef();
	IFACEMETHODIMP_(ULONG)	Release();

	// IClassFactory
	IFACEMETHODIMP			CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppv);
	IFACEMETHODIMP			LockServer(BOOL fLock);

private:    
	long m_cRef;
};