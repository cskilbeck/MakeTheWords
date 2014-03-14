
struct Rect2D
{
	Point2D	mTopLeft;
	Size2D	mSize;

	Rect2D()
	{
	}

	Rect2D(Point2D topLeft, Size2D size)
		: mTopLeft(topLeft)
		, mSize(size)
	{
	}

	Rect2D(Vec2 topLeft, Vec2 size)
		: mTopLeft(topLeft)
		, mSize(size)
	{
	}

	bool Contains(Point2D p) const
	{
		return p.x >= mTopLeft.x && p.x < mTopLeft.x + mSize.w && p.y >= mTopLeft.y && p.y < mTopLeft.y + mSize.h;
	}
};
