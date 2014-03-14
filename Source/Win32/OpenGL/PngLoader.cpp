//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include <D2D1.h>
#include <dxgiformat.h>
#include <assert.h>
#include "ScopedObject.h"

#pragma warning(push)
#pragma warning(disable : 4005)
#include <wincodec.h>
#pragma warning(pop)

#include <memory>

//////////////////////////////////////////////////////////////////////

struct WICImagingFactory
{
	IWICImagingFactory *mFactory;

	WICImagingFactory()
	{
		CoInitializeEx(null, 0);

		if(FAILED(CoCreateInstance(CLSID_WICImagingFactory1, null, CLSCTX_INPROC_SERVER, __uuidof(IWICImagingFactory), (LPVOID*)&mFactory)))
		{
			mFactory = null;
		}
	}

	~WICImagingFactory()
	{
		SafeRelease(mFactory);
		CoUninitialize();
	}
};

//////////////////////////////////////////////////////////////////////

static WICImagingFactory sWICImagingFactory;

//////////////////////////////////////////////////////////////////////

#define DO(x) { HRESULT hr = (x); if(FAILED(hr)) return hr; }

//////////////////////////////////////////////////////////////////////

HRESULT LoadPNGFile(wchar_t const *filename, uint8 * &pixels, Size2D &size, Size2D &tsize)	// returns as 32 bit RGBA, no resizing
{
	pixels = null;
	size.Clear();
	tsize.Clear();

	IWICImagingFactory* pWIC = sWICImagingFactory.mFactory;
	if(!pWIC)
	{
		return E_NOINTERFACE;
	}
	ScopedObject<IWICBitmapDecoder> decoder;
	DO(pWIC->CreateDecoderFromFilename(filename, 0, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &decoder))

	ScopedObject<IWICBitmapFrameDecode> frame;
	DO(decoder->GetFrame(0, &frame))

	DO(frame->GetSize((uint *)&size.w, (uint *)&size.h))

	tsize.w = NextPowerOf2(size.w);
	tsize.h = NextPowerOf2(size.h);

	size_t rowPitch = (tsize.w * 32 + 7) / 8;
	size_t imageSize = rowPitch * tsize.h;

	std::unique_ptr<uint8 []> temp(new uint8[imageSize]);
	memset(temp.get(), 0, imageSize);

	DO(frame->CopyPixels(0, static_cast<UINT>(rowPitch), static_cast<UINT>(imageSize), temp.get()))

	pixels = temp.get();
	temp.release();

	return S_OK;
}

