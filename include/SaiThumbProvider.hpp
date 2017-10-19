#pragma once
#include <memory>
#include <atomic>

#define WIN32_LEAN_AND_MEAN
#include <thumbcache.h>
#include <Propsys.h>

#include <sai.hpp>

class SaiThumbProvider
	: public IThumbnailProvider,
	IInitializeWithFile
{
public:
	SaiThumbProvider();
	virtual ~SaiThumbProvider();

	// IUnknown
	virtual HRESULT _stdcall QueryInterface(const IID& riid, void** ppvObject) override;
	virtual ULONG __stdcall AddRef() throw() override;
	virtual ULONG __stdcall Release() throw() override;

	// IInitializeWithFile
	virtual HRESULT _stdcall Initialize(LPCWSTR pszFilePath, DWORD grfMode) throw() override;

	// IThumbnailProvider
	virtual HRESULT _stdcall GetThumbnail(UINT cx, HBITMAP* phbmp, WTS_ALPHATYPE* pdwAlpha) throw() override;
private:
	std::unique_ptr<sai::Document> CurDocument;

	std::atomic<std::size_t> ReferenceCount;
};
