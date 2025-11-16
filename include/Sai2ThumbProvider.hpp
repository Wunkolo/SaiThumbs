#pragma once
#include <atomic>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#include <Propsys.h>
#include <thumbcache.h>

#include <sai2.hpp>

#include <mio/mmap.hpp>

class Sai2ThumbProvider : public IThumbnailProvider, IInitializeWithFile
{
public:
	Sai2ThumbProvider();
	virtual ~Sai2ThumbProvider();

	// IUnknown
	virtual HRESULT _stdcall QueryInterface(
		const IID& riid, void** ppvObject
	) override;
	virtual ULONG __stdcall AddRef() throw() override;
	virtual ULONG __stdcall Release() throw() override;

	// IInitializeWithFile
	virtual HRESULT _stdcall Initialize(
		LPCWSTR pszFilePath, DWORD grfMode
	) throw() override;

	// IThumbnailProvider
	virtual HRESULT _stdcall GetThumbnail(
		UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha
	) throw() override;

private:
	std::atomic<std::size_t> ReferenceCount;

	mio::mmap_source MappedFile;
};
