#pragma once
#include <cstddef>

#include <Unknwn.h>

class CClassFactory : public IClassFactory
{
public:
	CClassFactory();

	// IUnknown
	virtual HRESULT _stdcall QueryInterface(const IID& riid, void** ppvObject) override;
	virtual ULONG _stdcall AddRef() override;
	virtual ULONG _stdcall Release() override;

	// IClassFactory
	virtual HRESULT _stdcall CreateInstance(
		IUnknown* pUnkOuter, const IID& riid, void** ppvObject) override;
	virtual HRESULT _stdcall LockServer(BOOL fLock) override;

private:
	~CClassFactory();
	std::size_t ReferenceCount;
};
