#include <cstddef>
#include <cstdint>
#include <cwchar>
#include <type_traits>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>

#include <ClassFactory.hpp>
#include <Config.hpp>
#include <Globals.hpp>

extern "C" IMAGE_DOS_HEADER __ImageBase;

std::int32_t __stdcall DllMain(
	HINSTANCE hDLL, std::uint32_t Reason, void* Reserved
)
{
	switch( Reason )
	{
	case DLL_PROCESS_ATTACH:
	{
		DisableThreadLibraryCalls(hDLL);
		break;
	}
	}

	return true;
}

/// Registry
struct RegistryEntry
{
	HKEY           Root;
	const wchar_t* KeyName;
	const wchar_t* KeyValue;
	DWORD          ValueType;
	const wchar_t* Data;
};

/// Registers this DLL as a COM server
extern "C" HRESULT __stdcall DllRegisterServer()
{
	WCHAR ModulePath[MAX_PATH];
	GetModuleFileNameW(
		reinterpret_cast<HMODULE>(&__ImageBase), ModulePath, MAX_PATH
	);

	const RegistryEntry Registry[] = {
		// clang-format off
		// Register Sai1 Handler
		{HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" Sai1ThumbHandlerCLSID,                     nullptr,           REG_SZ, Sai1ThumbHandlerName},
		{HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" Sai1ThumbHandlerCLSID L"\\InProcServer32", nullptr,           REG_SZ, ModulePath},
		{HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" Sai1ThumbHandlerCLSID L"\\InProcServer32", L"ThreadingModel", REG_SZ, L"Apartment"},
		{HKEY_CURRENT_USER, L"Software\\Classes\\" Sai1ThumbHandlerExtension L"\\ShellEx\\" IThumbnailProviderCLSID, nullptr, REG_SZ, Sai1ThumbHandlerCLSID},
		// Register Sai2 Handler
		{HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" Sai2ThumbHandlerCLSID,                     nullptr,           REG_SZ, Sai2ThumbHandlerName},
		{HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" Sai2ThumbHandlerCLSID L"\\InProcServer32", nullptr,           REG_SZ, ModulePath},
		{HKEY_CURRENT_USER, L"Software\\Classes\\CLSID\\" Sai2ThumbHandlerCLSID L"\\InProcServer32", L"ThreadingModel", REG_SZ, L"Apartment"},
		{HKEY_CURRENT_USER, L"Software\\Classes\\" Sai2ThumbHandlerExtension L"\\ShellEx\\" IThumbnailProviderCLSID, nullptr, REG_SZ, Sai2ThumbHandlerCLSID},
		// clang-format on
	};

	// Set all the appropriate registery entries
	for( std::size_t i = 0; i < std::extent_v<decltype(Registry)>; i++ )
	{
		HKEY                 CurKey;
		const RegistryEntry& CurReg = Registry[i];
		RegCreateKeyExW(
			CurReg.Root, CurReg.KeyName, 0, nullptr, REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE, nullptr, &CurKey, nullptr
		);
		RegSetValueExW(
			CurKey, CurReg.KeyValue, 0, CurReg.ValueType,
			reinterpret_cast<const unsigned char*>(CurReg.Data),
			static_cast<std::uint32_t>(
				(std::wcslen(CurReg.Data) + 1) * sizeof(wchar_t)
			)
		);
		RegCloseKey(CurKey);
	}

	// Further configure the Sai1 thumbnail-handler
	{

		HKEY CurKey;
		RegCreateKeyExW(
			HKEY_CURRENT_USER,
			L"Software\\Classes\\CLSID\\" Sai1ThumbHandlerCLSID, 0, nullptr,
			REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &CurKey, nullptr
		);

		// Don't run this thumbnail handler in a separate process
		DWORD DisableProcessIsolation = 1;
		RegSetValueExW(
			CurKey, L"DisableProcessIsolation", 0, REG_DWORD,
			reinterpret_cast<const unsigned char*>(&DisableProcessIsolation),
			sizeof(DWORD)
		);
		RegCloseKey(CurKey);

		// Use the Photo-Border for this thumbnail-handler
		RegCreateKeyExW(
			HKEY_CURRENT_USER, L"Software\\Classes\\" Sai1ThumbHandlerExtension,
			0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr,
			&CurKey, nullptr
		);
		DWORD Treatment = 2;
		RegSetValueExW(
			CurKey, L"Treatment", 0, REG_DWORD,
			reinterpret_cast<const unsigned char*>(&Treatment), sizeof(DWORD)
		);
		RegCloseKey(CurKey);
	}

	// Further configure the Sai2 thumbnail-handler
	{

		HKEY CurKey;
		RegCreateKeyExW(
			HKEY_CURRENT_USER,
			L"Software\\Classes\\CLSID\\" Sai2ThumbHandlerCLSID, 0, nullptr,
			REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr, &CurKey, nullptr
		);

		// Don't run this thumbnail handler in a separate process
		DWORD DisableProcessIsolation = 1;
		RegSetValueExW(
			CurKey, L"DisableProcessIsolation", 0, REG_DWORD,
			reinterpret_cast<const unsigned char*>(&DisableProcessIsolation),
			sizeof(DWORD)
		);
		RegCloseKey(CurKey);

		// Use the Photo-Border for this thumbnail-handler
		RegCreateKeyExW(
			HKEY_CURRENT_USER, L"Software\\Classes\\" Sai2ThumbHandlerExtension,
			0, nullptr, REG_OPTION_NON_VOLATILE, KEY_SET_VALUE, nullptr,
			&CurKey, nullptr
		);
		DWORD Treatment = 2;
		RegSetValueExW(
			CurKey, L"Treatment", 0, REG_DWORD,
			reinterpret_cast<const unsigned char*>(&Treatment), sizeof(DWORD)
		);
		RegCloseKey(CurKey);
	}

	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
	return S_OK;
}

/// Unregisters this DLL as a COM server
extern "C" HRESULT __stdcall DllUnregisterServer()
{
	const wchar_t* RegistryFolders[] = {
		L"Software\\Classes\\CLSID\\" Sai1ThumbHandlerCLSID,
		L"Software\\Classes\\CLSID\\" Sai2ThumbHandlerCLSID,
		L"Software\\Classes\\" Sai1ThumbHandlerExtension,
		L"Software\\Classes\\" Sai2ThumbHandlerExtension,
	};

	for( std::size_t i = 0; i < std::extent_v<decltype(RegistryFolders)>; i++ )
	{
		RegDeleteTreeW(HKEY_CURRENT_USER, RegistryFolders[i]);
	}
	return S_OK;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
	return Globals::ReferenceGet() ? S_FALSE : S_OK;
}

extern "C" HRESULT __stdcall DllGetClassObject(
	const IID& rclsid, const IID& riid, void** ppv
)
{
	if( ppv == nullptr )
	{
		return E_INVALIDARG;
	}

	IID Sai1ThumbHandlerIID;
	IIDFromString(Sai1ThumbHandlerCLSID, &Sai1ThumbHandlerIID);

	IID Sai2ThumbHandlerIID;
	IIDFromString(Sai2ThumbHandlerCLSID, &Sai2ThumbHandlerIID);

	// Our class factory only handles sai1 and sai2
	if( !IsEqualCLSID(Sai1ThumbHandlerIID, rclsid)
		|| !IsEqualCLSID(Sai2ThumbHandlerIID, rclsid) )
	{
		return CLASS_E_CLASSNOTAVAILABLE;
	}

	CClassFactory* ClassFactory = new CClassFactory();
	if( ClassFactory == nullptr )
	{
		return E_OUTOFMEMORY;
	}
	const HRESULT Result = ClassFactory->QueryInterface(riid, ppv);
	ClassFactory->Release();
	return Result;
}
