#include <cstdint>
#include <cstddef>
#include <cstdio>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <ObjBase.h>
#include <Shlwapi.h>
#include <Thumbcache.h>
#include <Unknwn.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Ole32.lib")

const wchar_t* ProviderGUID = L"{35788B9B-04AB-4B75-AB3A-1ED403AFC746}";

using DllGetClassObjectT = HRESULT(const IID& rclsid, const IID& riid, void** ppv);

BOOL SaveHBITMAPToFile(HBITMAP Bitmap, const char* Filename);

int main(int argc, char** argv)
{
	GUID clsid;
	IIDFromString(ProviderGUID, &clsid);

	HMODULE DLLHandle = LoadLibraryA("SaiThumbs.dll");

	if( !DLLHandle )
	{
		std::puts("can't open DLL");
		return EXIT_FAILURE;
	}

	DllGetClassObjectT* DllGetClassObject = reinterpret_cast<DllGetClassObjectT*>(GetProcAddress(
		DLLHandle, "DllGetClassObject"));

	if( DllGetClassObject == nullptr )
	{
		std::puts("Failed: Unable to get DLLGetClassobject address");
		return EXIT_FAILURE;
	}

	IClassFactory* ClassFactory = nullptr;
	HRESULT Result = DllGetClassObject(clsid, IID_IClassFactory, reinterpret_cast<void**>(&ClassFactory));
	if( Result != S_OK )
	{
		std::printf("Failed: Unable to get IClassFactory: %08x\n", Result);
		return EXIT_FAILURE;
	}

	IInitializeWithFile* InitWithFile;
	Result = ClassFactory->CreateInstance(nullptr, IID_IInitializeWithFile, reinterpret_cast<void**>(&InitWithFile));
	if( Result != S_OK )
	{
		std::puts("Failed: Unable to get IInitializeWithFile");
		return EXIT_FAILURE;
	}
	ClassFactory->Release();

	IThumbnailProvider* ThumbProvider;
	Result = InitWithFile->QueryInterface(IID_IThumbnailProvider, reinterpret_cast<void**>(&ThumbProvider));
	if( Result != S_OK )
	{
		std::puts("Failed: Unable to get IThumbnailProvider");
		return EXIT_FAILURE;
	}

	Result = InitWithFile->Initialize(L"./test.sai", 0);
	InitWithFile->Release();
	if( Result != S_OK )
	{
		std::puts("Failed: Unable to initilize IInitializeWithFile");
		return EXIT_FAILURE;
	}

	HBITMAP ThumbImage;
	WTS_ALPHATYPE ThumbAlphaType;
	Result = ThumbProvider->GetThumbnail(256, &ThumbImage, &ThumbAlphaType);
	ThumbProvider->Release();
	if( Result != S_OK )
	{
		std::puts("Failed: GetThumbnail returned failure");
		return EXIT_FAILURE;
	}

	SaveHBITMAPToFile(
		ThumbImage,
		"./test.bmp"
	);

	return EXIT_SUCCESS;
}

BOOL SaveHBITMAPToFile(HBITMAP Bitmap, const char* Filename)
{
	const DWORD PaletteSize = 0;
	DWORD BmBitsSize = 0, HeaderSize = 0, Written = 0;
	BITMAP NewBitmap;
	BITMAPFILEHEADER Header;
	BITMAPINFOHEADER bi;
	HANDLE hOldPal2 = nullptr;
	HDC DeviceContext = CreateDC(TEXT("DISPLAY"), nullptr, nullptr, nullptr);
	const int Bits = GetDeviceCaps(DeviceContext, BITSPIXEL) * GetDeviceCaps(DeviceContext, PLANES);
	DeleteDC(DeviceContext);
	const WORD BitCount = Bits;
	GetObject(Bitmap, sizeof(NewBitmap), reinterpret_cast<char*>(&NewBitmap));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = NewBitmap.bmWidth;
	bi.biHeight = -NewBitmap.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = BitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 256;
	BmBitsSize = ((NewBitmap.bmWidth * BitCount + 31) & ~31) / 8
		* NewBitmap.bmHeight;
	const HANDLE hDib = GlobalAlloc(GHND, BmBitsSize + PaletteSize + sizeof(BITMAPINFOHEADER));
	const LPBITMAPINFOHEADER lpbi = static_cast<LPBITMAPINFOHEADER>(GlobalLock(hDib));
	*lpbi = bi;

	const HANDLE hPal = GetStockObject(DEFAULT_PALETTE);
	if( hPal )
	{
		DeviceContext = GetDC(nullptr);
		hOldPal2 = SelectPalette(DeviceContext, static_cast<HPALETTE>(hPal), FALSE);
		RealizePalette(DeviceContext);
	}

	GetDIBits(
		DeviceContext, Bitmap, 0, static_cast<std::uint32_t>(NewBitmap.bmHeight),
		reinterpret_cast<char*>(lpbi) + sizeof(BITMAPINFOHEADER)
		+ PaletteSize, reinterpret_cast<BITMAPINFO *>(lpbi), DIB_RGB_COLORS);

	if( hOldPal2 )
	{
		SelectPalette(DeviceContext, static_cast<HPALETTE>(hOldPal2), TRUE);
		RealizePalette(DeviceContext);
		ReleaseDC(nullptr, DeviceContext);
	}

	const HANDLE FileHandle = CreateFileA(
		Filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

	if( FileHandle == INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}

	Header.bfType = 0x4D42;
	HeaderSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + PaletteSize + BmBitsSize;
	Header.bfSize = HeaderSize;
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) +
		PaletteSize;

	WriteFile(FileHandle, reinterpret_cast<char*>(&Header), sizeof(BITMAPFILEHEADER), &Written, nullptr);

	WriteFile(FileHandle, reinterpret_cast<char*>(lpbi), HeaderSize, &Written, nullptr);
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(FileHandle);
	return TRUE;
}
