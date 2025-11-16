#include <SaiThumbProvider.hpp>

#include <algorithm>
#include <codecvt>
#include <locale>
#include <string>

#include <stb_image_resize.h>

#include <Globals.hpp>
#include <Shlwapi.h>

SaiThumbProvider::SaiThumbProvider() : ReferenceCount(1)
{
	Globals::ReferenceAdd();
}

SaiThumbProvider::~SaiThumbProvider()
{
	Globals::ReferenceRelease();
}

HRESULT SaiThumbProvider::QueryInterface(const IID& riid, void** ppvObject)
{
	static const QITAB InterfaceTable[] = {
		QITABENT(SaiThumbProvider, IInitializeWithFile),
		QITABENT(SaiThumbProvider, IThumbnailProvider),
		{nullptr},
	};
	return QISearch(this, InterfaceTable, riid, ppvObject);
}

ULONG SaiThumbProvider::AddRef() throw()
{
	return static_cast<std::uint32_t>(++ReferenceCount);
}

ULONG SaiThumbProvider::Release() throw()
{
	const std::size_t NewReferenceCount = --ReferenceCount;
	if( NewReferenceCount == 0 )
	{
		delete this;
	}
	return static_cast<std::uint32_t>(NewReferenceCount);
}

HRESULT SaiThumbProvider::GetThumbnail(
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
		stbir_resize_uint8(
			reinterpret_cast<const std::uint8_t*>(PixelData.get()), Width,
			Height, 0, reinterpret_cast<std::uint8_t*>(Resized.get()), NewWidth,
			NewHeight, 0, 4
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

HRESULT SaiThumbProvider::Initialize(LPCWSTR pszFilePath, DWORD grfMode) throw()
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
