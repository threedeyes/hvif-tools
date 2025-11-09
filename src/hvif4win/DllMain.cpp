/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <windows.h>
#include <shlwapi.h>
#include <strsafe.h>
#include <shlobj.h>

#include <new>

#include "Globals.h"
#include "ClassFactory.h"

HINSTANCE g_hInst = NULL;
long g_cDllRef = 0;

BOOL APIENTRY
DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	switch (dwReason) {
		case DLL_PROCESS_ATTACH:
			g_hInst = hModule;
			DisableThreadLibraryCalls(hModule);
			break;
		case DLL_PROCESS_DETACH:
			break;
	}

	return TRUE;
}

STDAPI
DllGetClassObject(REFCLSID rclsid, REFIID riid, void **ppv)
{
	HRESULT hr = CLASS_E_CLASSNOTAVAILABLE;

	if (IsEqualCLSID(CLSID_HVIFThumbnailProvider, rclsid)) {
		hr = E_OUTOFMEMORY;
		ClassFactory *pClassFactory = new (std::nothrow) ClassFactory();
		if (pClassFactory) {
			hr = pClassFactory->QueryInterface(riid, ppv);
			pClassFactory->Release();
		}
	}

	return hr;
}

STDAPI
DllCanUnloadNow(void)
{
	return g_cDllRef > 0 ? S_FALSE : S_OK;
}

HRESULT
SetHKCRRegistryKeyAndValue(PCWSTR pszSubKey, PCWSTR pszValueName, PCWSTR pszData)
{
	HRESULT hr;
	HKEY hKey = NULL;

	hr = HRESULT_FROM_WIN32(RegCreateKeyExW(HKEY_CLASSES_ROOT, pszSubKey, 0, 
		NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL));

	if (SUCCEEDED(hr)) {
		if (pszData != NULL) {
			DWORD cbData = (DWORD)((wcslen(pszData) + 1) * sizeof(*pszData));
			hr = HRESULT_FROM_WIN32(RegSetValueExW(hKey, pszValueName, 0, 
				REG_SZ, reinterpret_cast<const BYTE *>(pszData), cbData));
		}

		RegCloseKey(hKey);
	}

	return hr;
}

STDAPI
DllRegisterServer(void)
{
	HRESULT hr;

	wchar_t szModule[MAX_PATH];
	if (GetModuleFileNameW(g_hInst, szModule, ARRAYSIZE(szModule)) == 0) {
		hr = HRESULT_FROM_WIN32(GetLastError());
		return hr;
	}

	wchar_t szCLSID[MAX_PATH];
	StringFromGUID2(CLSID_HVIFThumbnailProvider, szCLSID, ARRAYSIZE(szCLSID));

	wchar_t szSubkey[MAX_PATH];

	hr = StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
	if (SUCCEEDED(hr)) {
		hr = SetHKCRRegistryKeyAndValue(szSubkey, NULL, L"Haiku Icon Thumbnail Provider");
	}

	if (SUCCEEDED(hr)) {
		hr = StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), 
			L"CLSID\\%s\\InprocServer32", szCLSID);
		if (SUCCEEDED(hr)) {
			hr = SetHKCRRegistryKeyAndValue(szSubkey, NULL, szModule);
			if (SUCCEEDED(hr)) {
				hr = SetHKCRRegistryKeyAndValue(szSubkey, L"ThreadingModel", L"Apartment");
			}
		}
	}

	if (SUCCEEDED(hr)) {
		hr = SetHKCRRegistryKeyAndValue(
			L".hvif\\ShellEx\\{E357FCCD-A995-4576-B01F-234630154E96}", NULL, szCLSID);
	}

	if (SUCCEEDED(hr)) {
		hr = SetHKCRRegistryKeyAndValue(L".hvif", NULL, L"HVIFFile");
		if (SUCCEEDED(hr)) {
			hr = SetHKCRRegistryKeyAndValue(L"HVIFFile", NULL, L"Haiku Vector Icon File");
		}
	}

	if (SUCCEEDED(hr)) {
		hr = SetHKCRRegistryKeyAndValue(
			L".iom\\ShellEx\\{E357FCCD-A995-4576-B01F-234630154E96}", NULL, szCLSID);
	}

	if (SUCCEEDED(hr)) {
		hr = SetHKCRRegistryKeyAndValue(L".iom", NULL, L"IOMFile");
		if (SUCCEEDED(hr)) {
			hr = SetHKCRRegistryKeyAndValue(L"IOMFile", NULL, L"Icon-O-Matic File");
		}
	}

	if (SUCCEEDED(hr)) {
		SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	}

	return hr;
}

STDAPI
DllUnregisterServer(void)
{
	HRESULT hr = S_OK;

	wchar_t szCLSID[MAX_PATH];
	StringFromGUID2(CLSID_HVIFThumbnailProvider, szCLSID, ARRAYSIZE(szCLSID));

	wchar_t szSubkey[MAX_PATH];

	hr = StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), 
		L".hvif\\ShellEx\\{E357FCCD-A995-4576-B01F-234630154E96}");

	if (SUCCEEDED(hr)) {
		RegDeleteTreeW(HKEY_CLASSES_ROOT, szSubkey);
	}

	RegDeleteKeyW(HKEY_CLASSES_ROOT, L".hvif\\ShellEx");
	RegDeleteKeyW(HKEY_CLASSES_ROOT, L"HVIFFile");

	hr = StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), 
		L".iom\\ShellEx\\{E357FCCD-A995-4576-B01F-234630154E96}");

	if (SUCCEEDED(hr)) {
		RegDeleteTreeW(HKEY_CLASSES_ROOT, szSubkey);
	}

	RegDeleteKeyW(HKEY_CLASSES_ROOT, L".iom\\ShellEx");
	RegDeleteKeyW(HKEY_CLASSES_ROOT, L"IOMFile");

	hr = StringCchPrintfW(szSubkey, ARRAYSIZE(szSubkey), L"CLSID\\%s", szCLSID);
	if (SUCCEEDED(hr)) {
		RegDeleteTreeW(HKEY_CLASSES_ROOT, szSubkey);
	}

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);

	return hr;
}
