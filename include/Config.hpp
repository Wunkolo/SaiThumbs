#pragma once

#include <guiddef.h>

namespace SaiThumb
{

#define Sai1ThumbHandlerCLSID_SZ L"{35788B9B-04AB-4B75-AB3A-1ED403AFC746}"
const GUID Sai1ThumbHandlerCLSID = {
	0x35788B9B, 0x04AB, 0x4B75, {0xAB, 0x3A, 0x1E, 0xD4, 0x03, 0xAF, 0xC7, 0x46}
};
#define Sai1ThumbHandlerName L"Sai1 Thumb Handler"
#define Sai1ThumbHandlerExtension L".sai"

#define Sai2ThumbHandlerCLSID_SZ L"{A7E68441-D892-40FA-B782-5144380A0875}"
const GUID Sai2ThumbHandlerCLSID = {
	0xA7E68441, 0xD892, 0x40FA, {0xB7, 0x82, 0x51, 0x44, 0x38, 0x0A, 0x08, 0x75}
};
#define Sai2ThumbHandlerName L"Sai2 Thumb Handler"
#define Sai2ThumbHandlerExtension L".sai2"

#define IThumbnailProviderCLSID_SZ L"{E357FCCD-A995-4576-B01F-234630154E96}"
const GUID IThumbnailProviderCLSID = {
	0xE357FCCD, 0xA995, 0x4576, {0xB0, 0x1F, 0x23, 0x46, 0x30, 0x15, 0x4E, 0x96}
};

} // namespace SaiThumb
