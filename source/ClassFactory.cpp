#include <ClassFactory.hpp>

#include <Config.hpp>
#include <Globals.hpp>

#include <Sai1ThumbProvider.hpp>
#include <Sai2ThumbProvider.hpp>

#include <Shlwapi.h>
#include <combaseapi.h>

#include <atomic>

CClassFactory::CClassFactory() : ReferenceCount(1)
{
	Globals::ReferenceAdd();
}

CClassFactory::~CClassFactory()
{
	Globals::ReferenceRelease();
}

HRESULT CClassFactory::QueryInterface(const IID& riid, void** ppvObject)
{
	static const QITAB InterfaceTable[] = {
		QITABENT(CClassFactory, IClassFactory),
		{nullptr},
	};
	return QISearch(this, InterfaceTable, riid, ppvObject);
}

ULONG CClassFactory::AddRef()
{
	return static_cast<std::uint32_t>(++ReferenceCount);
}

ULONG CClassFactory::Release()
{
	const std::size_t NewReferenceCount = --ReferenceCount;
	if( NewReferenceCount == 0 )
	{
		delete this;
	}
	return static_cast<std::uint32_t>(NewReferenceCount);
}

HRESULT CClassFactory::CreateInstance(
	IUnknown* pUnkOuter, const IID& riid, void** ppvObject
)
{
	if( pUnkOuter != nullptr )
	{
		return CLASS_E_NOAGGREGATION;
	}
	*ppvObject = nullptr;

	IThumbnailProvider* Provider = nullptr;

	IID Sai1ThumbHandlerIID;
	IIDFromString(Sai1ThumbHandlerCLSID, &Sai1ThumbHandlerIID);

	IID Sai2ThumbHandlerIID;
	IIDFromString(Sai2ThumbHandlerCLSID, &Sai2ThumbHandlerIID);

	if( IsEqualCLSID(riid, Sai1ThumbHandlerIID) )
	{
		Provider = new Sai1ThumbProvider();
	}
	else if( IsEqualCLSID(riid, Sai2ThumbHandlerIID) )
	{
		Provider = new Sai2ThumbProvider();
	}

	if( Provider == nullptr )
	{
		return E_OUTOFMEMORY;
	}

	const HRESULT Result = Provider->QueryInterface(riid, ppvObject);
	Provider->Release();
	return Result;
}

HRESULT CClassFactory::LockServer(BOOL fLock)
{
	return E_NOTIMPL;
}
