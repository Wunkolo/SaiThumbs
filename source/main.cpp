#include <cstddef>
#include <cstdint>
#include <cwchar>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <shlobj.h>

#include <Macros.hpp>
#include <Config.hpp>
#include <Globals.hpp>
#include <ClassFactory.hpp>

extern "C" IMAGE_DOS_HEADER __ImageBase;

std::int32_t __stdcall DllMain(HINSTANCE hDLL, std::uint32_t Reason, void* Reserved)
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

/// Registers this DLL as a COM server
extern "C" HRESULT __stdcall DllRegisterServer()
{
	WCHAR ModulePath[MAX_PATH];
	GetModuleFileNameW(
		reinterpret_cast<HMODULE>(&__ImageBase),
		ModulePath,
		MAX_PATH
	);

	const SaiThumb::RegistryEntry Registry[] =
	{
		{
			HKEY_CURRENT_USER,
			L"Software\\Classes\\CLSID\\" SaiThumbHandlerCLSID, nullptr, REG_SZ, SaiThumbHandlerName
		},
		{
			HKEY_CURRENT_USER,
			L"Software\\Classes\\CLSID\\" SaiThumbHandlerCLSID L"\\InProcServer32", nullptr, REG_SZ, ModulePath
		},
		{
			HKEY_CURRENT_USER,
			L"Software\\Classes\\CLSID\\" SaiThumbHandlerCLSID L"\\InProcServer32", L"ThreadingModel", REG_SZ,
			L"Apartment"
		},
		{
			HKEY_CURRENT_USER,
			L"Software\\Classes\\" SaiThumbHandlerExtension L"\\ShellEx\\" IThumbnailProviderCLSID, nullptr, REG_SZ,
			SaiThumbHandlerCLSID
		},
	};

	for( std::size_t i = 0; i < countof(Registry); i++ )
	{
		HKEY CurKey;
		const SaiThumb::RegistryEntry& CurReg = Registry[i];
		RegCreateKeyExW(
			CurReg.Root,
			CurReg.KeyName,
			0,
			nullptr,
			REG_OPTION_NON_VOLATILE,
			KEY_SET_VALUE,
			nullptr,
			&CurKey,
			nullptr
		);
		RegSetValueExW(
			CurKey,
			CurReg.KeyValue,
			0,
			CurReg.ValueType,
			reinterpret_cast<const unsigned char*>(CurReg.Data),
			static_cast<std::uint32_t>((std::wcslen(CurReg.Data) + 1) * sizeof(wchar_t))
		);
		RegCloseKey(CurKey);
	}

	HKEY CurKey;
	RegCreateKeyExW(
		HKEY_CURRENT_USER,
		L"Software\\Classes\\CLSID\\" SaiThumbHandlerCLSID,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_SET_VALUE,
		nullptr,
		&CurKey,
		nullptr
	);
	DWORD DisableProcessIsolation = 1;
	RegSetValueExW(
		CurKey,
		L"DisableProcessIsolation",
		0,
		REG_DWORD,
		reinterpret_cast<const unsigned char*>(&DisableProcessIsolation),
		sizeof(DWORD)
	);
	RegCloseKey(CurKey);

	RegCreateKeyExW(
		HKEY_CURRENT_USER,
		L"Software\\Classes\\" SaiThumbHandlerExtension,
		0,
		nullptr,
		REG_OPTION_NON_VOLATILE,
		KEY_SET_VALUE,
		nullptr,
		&CurKey,
		nullptr
	);
	DWORD Treatment = 2;
	RegSetValueExW(
		CurKey,
		L"Treatment",
		0,
		REG_DWORD,
		reinterpret_cast<const unsigned char*>(&Treatment),
		sizeof(DWORD)
	);
	RegCloseKey(CurKey);

	SHChangeNotify(
		SHCNE_ASSOCCHANGED,
		SHCNF_IDLIST,
		nullptr,
		nullptr
	);
	return S_OK;
}

/// Unregisters this DLL as a COM server
extern "C" HRESULT __stdcall DllUnregisterServer()
{
	const wchar_t* RegistryFolders[] =
	{
		L"Software\\Classes\\CLSID\\" SaiThumbHandlerCLSID,
		L"Software\\Classes\\" SaiThumbHandlerExtension
	};

	for( std::size_t i = 0; i < countof(RegistryFolders); i++ )
	{
		RegDeleteTreeW(
			HKEY_CURRENT_USER,
			RegistryFolders[i]
		);
	}
	return S_OK;
}

extern "C" HRESULT __stdcall DllCanUnloadNow()
{
	return Globals::ReferenceGet() ? S_FALSE : S_OK;
}

extern "C" HRESULT __stdcall DllGetClassObject(
	const IID& rclsid,
	const IID& riid,
	void** ppv)
{
	if( ppv == nullptr )
	{
		return E_INVALIDARG;
	}

	IID ThumbHandlerCLSID;
	IIDFromString(SaiThumbHandlerCLSID, &ThumbHandlerCLSID);

	if( !IsEqualCLSID(ThumbHandlerCLSID,rclsid) )
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
