#pragma once

#define WIN32_LEAN_AND_MEAN
#include <winreg.h>

namespace SaiThumb
{

#define SaiThumbHandlerCLSID L"{35788B9B-04AB-4B75-AB3A-1ED403AFC746}"
#define SaiThumbHandlerName L"Sai Thumb Handler"
#define SaiThumbHandlerExtension L".sai"

#define IThumbnailProviderCLSID L"{E357FCCD-A995-4576-B01F-234630154E96}"

/// Registry
struct RegistryEntry
{
	HKEY Root;
	const wchar_t* KeyName;
	const wchar_t* KeyValue;
	DWORD ValueType;
	const wchar_t* Data;
};


}
