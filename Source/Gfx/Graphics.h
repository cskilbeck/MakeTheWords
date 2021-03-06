#pragma once

//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Graphics
{
	Graphics();
	~Graphics();

	bool Init(HWND hWnd);
	void Resize(int width, int height);
	void Release();
	void ClearBackBuffer(Color color);
	void Present();

	void EnableScissoring(bool enable);
	void SetScissorRectangle(Rect2D const &rect);

private:

	struct GraphicsImpl;
	GraphicsImpl *impl;
};

//////////////////////////////////////////////////////////////////////

extern Graphics gGraphics;

//////////////////////////////////////////////////////////////////////

