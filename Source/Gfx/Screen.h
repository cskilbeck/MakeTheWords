//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Screen
{
	enum Orientation
	{
		Portrait,
		Landscape
	};

	extern function<void ()> OnResize;
	extern function<void ()> OnOrientationChanged;

	void Init(int width, int height);
	bool Update();
	void Clear(Color color);
	void Present();

	Orientation CurrentOrientation();

	int Width();
	int Height();
	Size2D GetSize();
	void ChangeSize(int newWidth, int newHeight);

	void Release();

	void EnableScissoring(bool enable);
	void SetScissorRectangle(Rect2D const &rect);
};
