#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <ObjBase.h>
#include <Shlwapi.h>
#include <Thumbcache.h>
#include <Unknwn.h>

#include <stdio.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "Ole32.lib")

// Our GUID here:
LPCOLESTR myGuid = L"{35788B9B-04AB-4B75-AB3A-1ED403AFC746}";

typedef HRESULT ourDllGetClassObjectT(REFCLSID rclsid, REFIID riid, void** ppv);

BOOL SaveHBITMAPToFile(HBITMAP hBitmap, LPCTSTR lpszFileName)
{
	HDC hDC;
	int iBits;
	WORD wBitCount;
	DWORD dwPaletteSize = 0, dwBmBitsSize = 0, dwDIBSize = 0, dwWritten = 0;
	BITMAP Bitmap0;
	BITMAPFILEHEADER bmfHdr;
	BITMAPINFOHEADER bi;
	LPBITMAPINFOHEADER lpbi;
	HANDLE fh, hDib, hPal, hOldPal2 = NULL;
	hDC = CreateDC(TEXT("DISPLAY"), NULL, NULL, NULL);
	iBits = GetDeviceCaps(hDC, BITSPIXEL) * GetDeviceCaps(hDC, PLANES);
	DeleteDC(hDC);
	wBitCount = iBits;
	GetObject(hBitmap, sizeof(Bitmap0), (LPSTR)&Bitmap0);
	bi.biSize = sizeof(BITMAPINFOHEADER);
	bi.biWidth = Bitmap0.bmWidth;
	bi.biHeight = -Bitmap0.bmHeight;
	bi.biPlanes = 1;
	bi.biBitCount = wBitCount;
	bi.biCompression = BI_RGB;
	bi.biSizeImage = 0;
	bi.biXPelsPerMeter = 0;
	bi.biYPelsPerMeter = 0;
	bi.biClrImportant = 0;
	bi.biClrUsed = 256;
	dwBmBitsSize = ((Bitmap0.bmWidth * wBitCount + 31) & ~31) / 8
		* Bitmap0.bmHeight;
	hDib = GlobalAlloc(GHND, dwBmBitsSize + dwPaletteSize + sizeof(BITMAPINFOHEADER));
	lpbi = (LPBITMAPINFOHEADER)GlobalLock(hDib);
	*lpbi = bi;

	hPal = GetStockObject(DEFAULT_PALETTE);
	if( hPal )
	{
		hDC = GetDC(NULL);
		hOldPal2 = SelectPalette(hDC, (HPALETTE)hPal, FALSE);
		RealizePalette(hDC);
	}

	GetDIBits(
		hDC, hBitmap, 0, (UINT)Bitmap0.bmHeight, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER)
		+ dwPaletteSize, (BITMAPINFO *)lpbi, DIB_RGB_COLORS);

	if( hOldPal2 )
	{
		SelectPalette(hDC, (HPALETTE)hOldPal2, TRUE);
		RealizePalette(hDC);
		ReleaseDC(NULL, hDC);
	}

	fh = CreateFile(
		lpszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if( fh == INVALID_HANDLE_VALUE )
		return FALSE;

	bmfHdr.bfType = 0x4D42; // "BM"
	dwDIBSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwPaletteSize + dwBmBitsSize;
	bmfHdr.bfSize = dwDIBSize;
	bmfHdr.bfReserved1 = 0;
	bmfHdr.bfReserved2 = 0;
	bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER) + dwPaletteSize;

	WriteFile(fh, (LPSTR)&bmfHdr, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);

	WriteFile(fh, (LPSTR)lpbi, dwDIBSize, &dwWritten, NULL);
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
	HMODULE dll = NULL;

	dll = LoadLibraryA("SaiThumbs.dll");
	if( !dll )
	{
		printf("can't open DLL\n");
		return 1;
	}

	ourDllGetClassObjectT* ourDllGetClassObject = reinterpret_cast<ourDllGetClassObjectT*>(GetProcAddress(
		dll, "DllGetClassObject"));

	IClassFactory* pFactory = NULL;
	r = ourDllGetClassObject(clsid, IID_IClassFactory, reinterpret_cast<void**>(&pFactory));
	if( r != S_OK )
	{
		printf("failed: get factory: %08x\n", r);
		return 2;
	}

	IInitializeWithFile* InitWithFile;
	r = pFactory->CreateInstance(NULL, IID_IInitializeWithFile, reinterpret_cast<void**>(&InitWithFile));
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
