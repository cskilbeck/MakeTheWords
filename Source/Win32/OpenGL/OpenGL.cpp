//////////////////////////////////////////////////////////////////////

#include "pch.h"
#include "Graphics.h"
#include <gl/GL.h>

//////////////////////////////////////////////////////////////////////

Graphics gGraphics;

struct Graphics::GraphicsImpl
{
	GraphicsImpl()
		: mHWND(null)
		, mDC(null)
		, mGLRC(null)
	{
	}

	HWND						mHWND;
	HDC							mDC;
	HGLRC						mGLRC;

};

//////////////////////////////////////////////////////////////////////

Graphics::Graphics()
	: impl(new GraphicsImpl)
{
}

//////////////////////////////////////////////////////////////////////

Graphics::~Graphics()
{
	SafeDelete(impl);
}

//////////////////////////////////////////////////////////////////////

bool Graphics::Init(HWND hWnd)
{
	impl->mHWND = hWnd;
	impl->mDC = GetDC(hWnd);
	PIXELFORMATDESCRIPTOR pfd;
	ZeroMemory(&pfd, sizeof(pfd));
	pfd.nSize = sizeof(pfd);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int iFormat = ChoosePixelFormat(impl->mDC, &pfd);
	SetPixelFormat(impl->mDC, iFormat, &pfd);
	impl->mGLRC = wglCreateContext(impl->mDC);
	wglMakeCurrent(impl->mDC, impl->mGLRC);

	glEnable(GL_TEXTURE_2D);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	RECT rc;
	GetClientRect(impl->mHWND, &rc);
	Resize(rc.right, rc.bottom);

	return true;
}

//////////////////////////////////////////////////////////////////////

void Graphics::Resize(int width, int height)
{
	glViewport(0, 0, width, height);
}

//////////////////////////////////////////////////////////////////////

void Graphics::Release()
{
	wglDeleteContext(impl->mGLRC);
	ReleaseDC(impl->mHWND, impl->mDC);
	impl->mGLRC = null;
	impl->mDC = null;
	impl->mHWND = null;
}

//////////////////////////////////////////////////////////////////////

void Graphics::ClearBackBuffer(Color color)
{
	float r = color.GetRed() / 255.0f;
	float g = color.GetGreen() / 255.0f;
	float b = color.GetBlue() / 255.0f;
	glClearColor(r, g, b, 1);
	glClear(GL_COLOR_BUFFER_BIT);
}

//////////////////////////////////////////////////////////////////////

void Graphics::Present()
{
	SwapBuffers(impl->mDC);
}

//////////////////////////////////////////////////////////////////////

void Graphics::EnableScissoring(bool enable)
{
	if(enable)
	{
		glEnable(GL_SCISSOR_TEST);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}
}

//////////////////////////////////////////////////////////////////////

void Graphics::SetScissorRectangle(Rect2D const &rect)
{
	glScissor(rect.mTopLeft.x, rect.mTopLeft.y, rect.mSize.w, rect.mSize.h);
}

//////////////////////////////////////////////////////////////////////

