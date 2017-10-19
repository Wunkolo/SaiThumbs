#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <ObjBase.h>
#include <Shlwapi.h>
#include <Thumbcache.h>
#include <Unknwn.h>

#include <stdio.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Ole32.lib")

LPCOLESTR myGuid = L"{35788B9B-04AB-4B75-AB3A-1ED403AFC746}";

typedef HRESULT ourDllGetClassObjectT(REFCLSID rclsid, REFIID riid, void** ppv);

BOOL SaveHBITMAPToFile(HBITMAP Bitmap, const char* Filename)
{
	const DWORD PaletteSize = 0;
	DWORD BmBitsSize = 0, DibSize = 0, Written = 0;
	BITMAP Bitmap0;
	BITMAPFILEHEADER Header;
	BITMAPINFOHEADER bi;
	HANDLE hOldPal2 = nullptr;
	HDC DeviceContext = CreateDC(TEXT("DISPLAY"), nullptr, nullptr, nullptr);
	const int Bits = GetDeviceCaps(DeviceContext, BITSPIXEL) * GetDeviceCaps(DeviceContext, PLANES);
	DeleteDC(DeviceContext);
	const WORD BitCount = Bits;
	GetObject(Bitmap, sizeof(Bitmap0), reinterpret_cast<LPSTR>(&Bitmap0));
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap0.bmWidth;
	bi.biHeight = -Bitmap0.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = BitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 256;
	BmBitsSize = ((Bitmap0.bmWidth * BitCount + 31) & ~31) / 8
		* Bitmap0.bmHeight;
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
		DeviceContext, Bitmap, 0, static_cast<UINT>(Bitmap0.bmHeight),
		reinterpret_cast<LPSTR>(lpbi) + sizeof(BITMAPINFOHEADER)
		+ PaletteSize, reinterpret_cast<BITMAPINFO *>(lpbi), DIB_RGB_COLORS);

	if( hOldPal2 )
	{
		SelectPalette(DeviceContext, static_cast<HPALETTE>(hOldPal2), TRUE);
		RealizePalette(DeviceContext);
		ReleaseDC(nullptr, DeviceContext);
	}

	HANDLE fh = CreateFile(
		Filename, GENERIC_WRITE, 0, nullptr, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, nullptr);

	if( fh == INVALID_HANDLE_VALUE )
	{
		return FALSE;
	}

	Header.bfType = 0x4D42; // "BM"
	DibSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + PaletteSize + BmBitsSize;
	Header.bfSize = DibSize;
	Header.bfReserved1 = 0;
	Header.bfReserved2 = 0;
	Header.bfOffBits = static_cast<DWORD>(sizeof(BITMAPFILEHEADER)) + static_cast<DWORD>(sizeof(BITMAPINFOHEADER)) +
		PaletteSize;

	WriteFile(fh, reinterpret_cast<LPSTR>(&Header), sizeof(BITMAPFILEHEADER), &Written, nullptr);

	WriteFile(fh, reinterpret_cast<LPSTR>(lpbi), DibSize, &Written, nullptr);
	GlobalUnlock(hDib);
	GlobalFree(hDib);
	CloseHandle(fh);
	return TRUE;
}

int main(int argc, char** argv)
{
	GUID clsid = {
		0
	};
	IIDFromString(myGuid, &clsid);

	HRESULT r;
	HMODULE dll = nullptr;

	dll = LoadLibraryA("SaiThumbs.dll");
	if( !dll )
	{
		printf("can't open DLL\n");
		return 1;
	}

	ourDllGetClassObjectT* ourDllGetClassObject = reinterpret_cast<ourDllGetClassObjectT*>(GetProcAddress(
		dll, "DllGetClassObject"));

	IClassFactory* pFactory = nullptr;
	r = ourDllGetClassObject(clsid, IID_IClassFactory, reinterpret_cast<void**>(&pFactory));
	if( r != S_OK )
	{
		printf("failed: get factory: %08x\n", r);
		return 2;
	}

	IInitializeWithFile* InitWithFile;
	r = pFactory->CreateInstance(nullptr, IID_IInitializeWithFile, reinterpret_cast<void**>(&InitWithFile));
	if( r != S_OK )
	{
		printf("failed: get object\n");
		return 3;
	}
	pFactory->Release();

	IThumbnailProvider* pProvider;
	r = InitWithFile->QueryInterface(IID_IThumbnailProvider, reinterpret_cast<void**>(&pProvider));
	if( r != S_OK )
	{
		printf("failed: get provider\n");
		return 5;
	}

	r = InitWithFile->Initialize(L"./test.sai", 0);
	InitWithFile->Release();
	if( r != S_OK )
	{
		printf("failed: init provider\n");
		return 11;
	}

	HBITMAP bmp;
	WTS_ALPHATYPE alpha;
	r = pProvider->GetThumbnail(256, &bmp, &alpha);
	pProvider->Release();
	if( r != S_OK )
	{
		printf("failed: make thumbnail\n");
		return 12;
	}

	SaveHBITMAPToFile(
		bmp,
		"./test.bmp"
	);

	printf("done");
}
