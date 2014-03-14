//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Size2D
{
	int w;
	int h;

	Size2D()
	{
	}

	Size2D(int w, int h) : w(w), h(h)
	{
	}

	explicit Size2D(Vec2 const &o)
	{
		w = (int)o.x;
		h = (int)o.y;
	}

	explicit Size2D(Point2D const &o)
	{
		w = o.x;
		h = o.y;
	}

	void Clear()
	{
		w = 0;
		h = 0;
	}
};