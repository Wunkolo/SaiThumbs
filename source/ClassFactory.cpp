#include <ClassFactory.hpp>

#include <atomic>

#include <Shlwapi.h>

#include <SaiThumbProvider.hpp>
#include <Globals.hpp>

CClassFactory::CClassFactory()
	: ReferenceCount(1)
{
	Globals::ReferenceAdd();
}

CClassFactory::~CClassFactory()
{
	Globals::ReferenceRelease();
}

HRESULT CClassFactory::QueryInterface(const IID& riid, void** ppvObject)
{
	static const QITAB Tabs[] =
	{
		QITABENT(CClassFactory, IClassFactory),
		{
			nullptr
		},
	};
	return QISearch(this, Tabs, riid, ppvObject);
}

ULONG CClassFactory::AddRef()
{
	return ++ReferenceCount;
}

ULONG CClassFactory::Release()
{
	const std::size_t NewReferenceCount = --ReferenceCount;
	if( NewReferenceCount == 0 )
	{
		delete this;
	}
	return NewReferenceCount;
}

HRESULT CClassFactory::CreateInstance(IUnknown* pUnkOuter, const IID& riid, void** ppvObject)
{
	if( pUnkOuter != nullptr )
	{
		return CLASS_E_NOAGGREGATION;
	}
	*ppvObject = nullptr;
	SaiThumbProvider* Provider = new SaiThumbProvider();
	if( Provider == nullptr )
	{
		return E_OUTOFMEMORY;
	}

	HRESULT hr = Provider->QueryInterface(riid, ppvObject);
	Provider->Release();
	return hr;
}

HRESULT CClassFactory::LockServer(BOOL fLock)
{
	return E_NOTIMPL;
}
