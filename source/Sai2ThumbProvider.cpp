#include <Sai2ThumbProvider.hpp>

#include <algorithm>

#include <stb_image_resize2.h>

#include <Globals.hpp>
#include <Shlwapi.h>

Sai2ThumbProvider::Sai2ThumbProvider() : ReferenceCount(1)
{
	Globals::ReferenceAdd();
}

Sai2ThumbProvider::~Sai2ThumbProvider()
{
	Globals::ReferenceRelease();
}

HRESULT Sai2ThumbProvider::QueryInterface(const IID& riid, void** ppvObject)
{
	static const QITAB InterfaceTable[] = {
		QITABENT(Sai2ThumbProvider, IInitializeWithFile),
		QITABENT(Sai2ThumbProvider, IThumbnailProvider),
		{nullptr},
	};
	return QISearch(this, InterfaceTable, riid, ppvObject);
}

ULONG Sai2ThumbProvider::AddRef() throw()
{
	return static_cast<std::uint32_t>(++ReferenceCount);
}

ULONG Sai2ThumbProvider::Release() throw()
{
	const std::size_t NewReferenceCount = --ReferenceCount;
	if( NewReferenceCount == 0 )
	{
		delete this;
	}
	return static_cast<std::uint32_t>(NewReferenceCount);
}

HRESULT Sai2ThumbProvider::GetThumbnail(
	UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha
) throw()
{
	std::size_t TestSize = cx;

	std::unique_ptr<std::uint32_t[]> TestPixels
		= std::make_unique<std::uint32_t[]>(TestSize * TestSize);

	std::fill_n(TestPixels.get(), TestSize * TestSize, 0xFF'FF'FF'FF);

	const HBITMAP Bitmap
		= CreateBitmap(TestSize, TestSize, 1, 32, TestPixels.get());

	if( Bitmap == nullptr )
	{
		return E_FAIL;
	}

	*phbmp    = Bitmap;
	*pdwAlpha = WTSAT_UNKNOWN;
	return S_OK;
}

HRESULT
Sai2ThumbProvider::Initialize(LPCWSTR pszFilePath, DWORD grfMode) throw()
{
	return S_OK;
}
