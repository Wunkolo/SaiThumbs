#pragma once
#include <atomic>
#include <cstddef>

#include <Shlwapi.h>
#include <Unknwn.h>
#include <combaseapi.h>
#include <thumbcache.h>

#include "Globals.hpp"

template<typename ThumbnailProviderT>
class ThumbnailProviderClassFactory final : public IClassFactory
{
public:
	ThumbnailProviderClassFactory() : ReferenceCount(1)
	{
		Globals::ReferenceAdd();
	}

	// IUnknown
	virtual HRESULT _stdcall QueryInterface(
		const IID& riid, void** ppvObject
	) override
	{
		static const QITAB InterfaceTable[] = {
			QITABENT(ThumbnailProviderClassFactory, IClassFactory),
			{nullptr},
		};
		return QISearch(this, InterfaceTable, riid, ppvObject);
	}

	virtual ULONG _stdcall AddRef() override
	{
		return static_cast<std::uint32_t>(++ReferenceCount);
	}

	virtual ULONG _stdcall Release() override
	{
		const std::size_t NewReferenceCount = --ReferenceCount;
		if( NewReferenceCount == 0 )
		{
			delete this;
		}
		return static_cast<std::uint32_t>(NewReferenceCount);
	}

	// IClassFactory
	virtual HRESULT _stdcall CreateInstance(
		IUnknown* pUnkOuter, const IID& riid, void** ppvObject
	) override
	{
		if( pUnkOuter != nullptr )
		{
			return CLASS_E_NOAGGREGATION;
		}
		*ppvObject = nullptr;

		IThumbnailProvider* Provider = new ThumbnailProviderT();
		if( Provider == nullptr )
		{
			return E_OUTOFMEMORY;
		}

		const HRESULT Result = Provider->QueryInterface(riid, ppvObject);
		Provider->Release();
		return Result;
	}

	virtual HRESULT _stdcall LockServer(BOOL fLock) override
	{
		return E_NOTIMPL;
	}

private:
	virtual ~ThumbnailProviderClassFactory()
	{
		Globals::ReferenceRelease();
	}

	std::atomic<std::size_t> ReferenceCount;
};
