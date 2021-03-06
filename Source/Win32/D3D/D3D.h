//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

extern ID3D11Device *			gDevice;
extern ID3D11DeviceContext *	gContext;

//////////////////////////////////////////////////////////////////////

#if defined(DEBUG)
#define DX(x) { HRESULT hr = (x); if(FAILED(hr)) { TRACE(#x" failed: %08x\n", hr); assert(false); return hr; } }
#define DXV(x) { HRESULT hr = (x); if(FAILED(hr)) { TRACE(#x" failed: %08x\n", hr); assert(false); return; } }
#define DXB(x) { HRESULT hr = (x); if(FAILED(hr)) { TRACE(#x" failed: %08x\n", hr); assert(false); return false; } }
#else
#define DX(x) { HRESULT hr = (x); if(FAILED(hr)) return hr; }
#define DXV(x) { HRESULT hr = (x); if(FAILED(hr)) return; }
#define DXB(x) { HRESULT hr = (x); if(FAILED(hr)) return false; }
#endif

//////////////////////////////////////////////////////////////////////

template<typename T> struct DXPtr
{
	DXPtr(T *init = nullptr) : p(init)
	{
		leak = malloc(1);
	}

	DXPtr(DXPtr const &ptr) : p(ptr.p)
	{
		p->AddRef();
	}

	DXPtr(DXPtr &&ptr) : p(nullptr)
	{
		std::swap(p, ptr.p);
	}

	DXPtr &operator=(DXPtr const &ptr)
	{
		reset();
		p = ptr.p;
		p->AddRef();
		return *this;
	}

	DXPtr &operator=(DXPtr&& ptr)
	{
		std::swap(p, ptr.p);
		return *this;
	}

	~DXPtr()
	{
		free(leak);
		Release();
	}

	T **operator &()
	{
		return &p;
	}

	T *operator->() const
	{
		return p;
	}

	T &operator *() const
	{
		return *p;
	}

	operator T *()
	{
		return p;
	}

	T *get() const
	{
		return p;
	}

	void Release()
	{
		if(p != nullptr)
		{
			p->Release();
			p = nullptr;
		}
	}

	T *p;
	void *leak;
};