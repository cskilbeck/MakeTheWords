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

