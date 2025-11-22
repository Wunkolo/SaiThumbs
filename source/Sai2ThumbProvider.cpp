#include <Sai2ThumbProvider.hpp>

#include <algorithm>

#include <stb_image.h>
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
	if( !MappedFile.is_open() )
	{
		return E_FAIL;
	}

	const std::span<const std::byte> FileData(
		reinterpret_cast<const std::byte*>(MappedFile.data()), MappedFile.size()
	);

	std::uint32_t                    ThumbnailWidth  = 0;
	std::uint32_t                    ThumbnailHeight = 0;
	std::unique_ptr<std::uint32_t[]> ThumbnailRGBA8;

	const auto HandleCanvasThumbnailProc
		= [&](const sai2::CanvasHeader&  Header,
			  const sai2::CanvasEntry&   TableEntry,
			  std::span<const std::byte> Bytes) -> bool {
		switch( TableEntry.Type )
		{
		case sai2::CanvasDataType::ThumbnailLossy:
		{
			if( const auto JpegStream = sai2::ExtractJssfToJpeg(Bytes);
				!std::get<0>(JpegStream).empty() )
			{
				// Decode jpeg stream
				const auto JpegData = std::get<0>(JpegStream);

				// Decode jpeg data into RGBA8
				int      JpegWidth       = 0;
				int      JpegHeight      = 0;
				int      JpegChannels    = 0;
				stbi_uc* JpegDecodedData = stbi_load_from_memory(
					reinterpret_cast<const stbi_uc*>(JpegData.data()),
					static_cast<int>(JpegData.size()), &JpegWidth, &JpegHeight,
					&JpegChannels, 4
				);
				if( JpegDecodedData == nullptr )
				{
					return false;
				}

				ThumbnailWidth  = JpegWidth;
				ThumbnailHeight = JpegHeight;
				ThumbnailRGBA8
					= std::make_unique<std::uint32_t[]>(JpegWidth * JpegHeight);
				std::memcpy(
					ThumbnailRGBA8.get(), JpegDecodedData,
					JpegWidth * JpegHeight * sizeof(std::uint32_t)
				);
				stbi_image_free(JpegDecodedData);
			}
			// Stop iterating after finding a thumbnail
			return false;
		}
		case sai2::CanvasDataType::ThumbnailLossless:
		{
			if( auto Extracted = sai2::ExtractDpcmToBGRA(Header, Bytes);
				!std::get<0>(Extracted).empty() )
			{
				std::vector<std::byte>& ThumbnailData = std::get<0>(Extracted);

				const std::span<std::uint32_t> ThumbnailImage(
					reinterpret_cast<std::uint32_t*>(ThumbnailData.data()),
					std::get<1>(Extracted) * std::get<2>(Extracted)
				);

				//// BGRA to RGBA
				for( std::uint32_t& Pixel : ThumbnailImage )
				{
					// G and R are already where they need to be.
					// Just swap B and R channels
					const std::uint8_t R = ((Pixel >> 16));
					const std::uint8_t B = ((Pixel >> 0));
					Pixel                = (Pixel & 0xFF'00'FF'00) | R
						  | (std::uint32_t(B) << 16);
				}

				ThumbnailWidth  = std::get<1>(Extracted);
				ThumbnailHeight = std::get<2>(Extracted);
				ThumbnailRGBA8  = std::make_unique<std::uint32_t[]>(
                    ThumbnailWidth * ThumbnailHeight
                );
				std::memcpy(
					ThumbnailRGBA8.get(), ThumbnailImage.data(),
					ThumbnailImage.size_bytes()
				);
			}
			// Stop iterating after finding a thumbnail
			return false;
			break;
		}
		default:
		{
			// Keep iterating
			return true;
		}
		}
	};

	sai2::IterateCanvasData(FileData, HandleCanvasThumbnailProc);

	if( !ThumbnailWidth || !ThumbnailHeight || !ThumbnailRGBA8 )
	{
		return E_FAIL;
	}

	if( cx < ThumbnailWidth || cx < ThumbnailHeight )
	{
		const std::float_t Scale
			= (std::min)(cx / static_cast<std::float_t>(ThumbnailWidth),
						 cx / static_cast<std::float_t>(ThumbnailHeight));

		const std::uint32_t NewWidth
			= static_cast<std::uint32_t>(ThumbnailWidth * Scale);
		const std::uint32_t NewHeight
			= static_cast<std::uint32_t>(ThumbnailHeight * Scale);

		if( !NewWidth || !NewHeight )
		{
			return E_FAIL;
		}

		std::unique_ptr<std::uint32_t[]> Resized
			= std::make_unique<std::uint32_t[]>(NewWidth * NewHeight);

		// Resize image to fit requested size
		stbir_resize_uint8_linear(
			reinterpret_cast<const std::uint8_t*>(ThumbnailRGBA8.get()),
			ThumbnailWidth, ThumbnailHeight, 0,
			reinterpret_cast<std::uint8_t*>(Resized.get()), NewWidth, NewHeight,
			0, STBIR_RGBA
		);

		ThumbnailWidth  = NewWidth;
		ThumbnailHeight = NewHeight;
		ThumbnailRGBA8  = std::move(Resized);
	}

	//// RGBA to ABGR
	const std::span<std::uint32_t> ThumbnailPixels(
		ThumbnailRGBA8.get(), ThumbnailWidth * ThumbnailHeight
	);
	for( std::uint32_t& Pixel32 : ThumbnailPixels )
	{
		const std::uint32_t Alpha = (Pixel32 >> 24) & 0xFF;
		const std::uint32_t Red   = (Pixel32 >> 16) & 0xFF;
		const std::uint32_t Green = (Pixel32 >> 8) & 0xFF;
		const std::uint32_t Blue  = (Pixel32 >> 0) & 0xFF;

		Pixel32 = (Alpha << 24) | // AA
				  (Blue << 16) |  // BB
				  (Green << 8) |  // GG
				  (Red << 0);     // RR
	}

	const HBITMAP Bitmap = CreateBitmap(
		ThumbnailWidth, ThumbnailHeight, 1, 32, ThumbnailRGBA8.get()
	);

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
	mio::mmap_source NewMappedFile = mio::mmap_source(pszFilePath);

	if( !NewMappedFile.is_open() )
	{
		return E_FAIL;
	}

	MappedFile = std::move(NewMappedFile);

	return S_OK;
}
