#include <SaiThumbProvider.hpp>

#include <string>
#include <locale>
#include <codecvt>
#include <algorithm>

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include <stb_image_resize.h>

#include <Globals.hpp>
#include <Shlwapi.h>

SaiThumbProvider::SaiThumbProvider()
	: ReferenceCount(1)
{
	Globals::ReferenceAdd();
}

SaiThumbProvider::~SaiThumbProvider()
{
	Globals::ReferenceRelease();
}

HRESULT SaiThumbProvider::QueryInterface(const IID& riid, void** ppvObject)
{
	static const QITAB Tabs[] =
	{
		QITABENT(SaiThumbProvider, IInitializeWithFile),
		QITABENT(SaiThumbProvider, IThumbnailProvider),
		{
			nullptr
		}
	};
	return QISearch(this, Tabs, riid, ppvObject);
}

ULONG SaiThumbProvider::AddRef() throw()
{
	return ++ReferenceCount;
}

ULONG SaiThumbProvider::Release() throw()
{
	const std::size_t NewReferenceCount = --ReferenceCount;
	if( NewReferenceCount == 0 )
	{
		delete this;
	}
	return NewReferenceCount;
}

HRESULT SaiThumbProvider::GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha) throw()
{
	if( CurDocument == nullptr )
	{
		return E_FAIL;
	}

	std::uint32_t Width, Height;
	std::unique_ptr<std::uint8_t[]> PixelData;
	std::tie(PixelData, Width, Height) = CurDocument->GetThumbnail();

	if( PixelData == nullptr || !Width || !Height )
	{
		return E_FAIL;
	}

	std::uint32_t NewWidth = Width, NewHeight = Height;

	const std::float_t Scale = (std::min)(
		cx / static_cast<std::float_t>(Width),
		cx / static_cast<std::float_t>(Height)
		);

	NewWidth = static_cast<std::uint32_t>(NewWidth * Scale);
	NewHeight = static_cast<std::uint32_t>(NewHeight * Scale);

	if( !NewWidth || !NewHeight )
	{
		return E_FAIL;
	}

	std::unique_ptr<std::uint8_t[]> Resized = std::make_unique<std::uint8_t[]>(
		NewWidth * NewHeight * 4
	);

	// Resize image to fit requested size
	stbir_resize_uint8(
		PixelData.get(),
		Width, Height, 0,
		Resized.get(),
		NewWidth, NewHeight, 0,
		4
	);

	const HBITMAP Bitmap = CreateBitmap(
		NewWidth,
		NewHeight,
		1,
		32,
		Resized.get()
	);

	if( Bitmap == nullptr )
	{
		return E_FAIL;
	}

	*phbmp = Bitmap;
	*pdwAlpha = WTSAT_UNKNOWN;
	return S_OK;
}

HRESULT SaiThumbProvider::Initialize(LPCWSTR pszFilePath, DWORD grfMode) throw()
{
	const std::wstring WFilePath(pszFilePath);

	using ConverterType = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<ConverterType, wchar_t> Converter;
	std::string FilePath = Converter.to_bytes(WFilePath);

	std::unique_ptr<sai::Document> NewDocument = std::make_unique<sai::Document>(
		FilePath.c_str()
	);

	if( !NewDocument->IsOpen() )
	{
		return E_FAIL;
	}

	CurDocument = std::move(NewDocument);
	return S_OK;
}
