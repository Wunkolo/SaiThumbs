#include <Sai1ThumbProvider.hpp>

#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>

#include <stb_image_resize2.h>

#include <Globals.hpp>
#include <Shlwapi.h>

Sai1ThumbProvider::Sai1ThumbProvider() : ReferenceCount(1)
{
	Globals::ReferenceAdd();
}

Sai1ThumbProvider::~Sai1ThumbProvider()
{
	Globals::ReferenceRelease();
}

HRESULT Sai1ThumbProvider::QueryInterface(const IID& riid, void** ppvObject)
{
	static const QITAB InterfaceTable[] = {
		QITABENT(Sai1ThumbProvider, IInitializeWithFile),
		QITABENT(Sai1ThumbProvider, IThumbnailProvider),
		{nullptr},
	};
	return QISearch(this, InterfaceTable, riid, ppvObject);
}

ULONG Sai1ThumbProvider::AddRef() throw()
{
	return static_cast<std::uint32_t>(++ReferenceCount);
}

ULONG Sai1ThumbProvider::Release() throw()
{
	const std::size_t NewReferenceCount = --ReferenceCount;
	if( NewReferenceCount == 0 )
	{
		delete this;
	}
	return static_cast<std::uint32_t>(NewReferenceCount);
}

HRESULT Sai1ThumbProvider::GetThumbnail(
	UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha
) throw()
{
	if( CurDocument == nullptr )
	{
		return E_FAIL;
	}

	std::uint32_t                Width, Height;
	std::unique_ptr<std::byte[]> PixelData;
	std::tie(PixelData, Width, Height) = CurDocument->GetThumbnail();

	if( PixelData == nullptr || !Width || !Height )
	{
		return E_FAIL;
	}

	if( cx < Width || cx < Height )
	{
		const std::float_t Scale
			= (std::min)(cx / static_cast<std::float_t>(Width),
						 cx / static_cast<std::float_t>(Height));

		const std::uint32_t NewWidth
			= static_cast<std::uint32_t>(Width * Scale);
		const std::uint32_t NewHeight
			= static_cast<std::uint32_t>(Height * Scale);

		if( !NewWidth || !NewHeight )
		{
			return E_FAIL;
		}

		std::unique_ptr<std::byte[]> Resized
			= std::make_unique<std::byte[]>(NewWidth * NewHeight * 4);

		// Resize image to fit requested size
		stbir_resize_uint8_linear(
			reinterpret_cast<const std::uint8_t*>(PixelData.get()), Width,
			Height, 0, reinterpret_cast<std::uint8_t*>(Resized.get()), NewWidth,
			NewHeight, 0, STBIR_RGBA
		);

		Width     = NewWidth;
		Height    = NewHeight;
		PixelData = std::move(Resized);
	}

	const HBITMAP Bitmap = CreateBitmap(Width, Height, 1, 32, PixelData.get());

	if( Bitmap == nullptr )
	{
		return E_FAIL;
	}

	*phbmp    = Bitmap;
	*pdwAlpha = WTSAT_UNKNOWN;
	return S_OK;
}

HRESULT
	Sai1ThumbProvider::Initialize(LPCWSTR pszFilePath, DWORD grfMode) throw()
{
	std::unique_ptr<sai::Document> NewDocument
		= std::make_unique<sai::Document>(pszFilePath);

	if( !NewDocument->IsOpen() )
	{
		return E_FAIL;
	}

	CurDocument = std::move(NewDocument);
	return S_OK;
}
