//////////////////////////////////////////////////////////////////////

#pragma once

#include <math.h>

//////////////////////////////////////////////////////////////////////

struct Point2D;
struct Size2D;

//////////////////////////////////////////////////////////////////////

#if defined(IOS)
	#define ALIGNED(x) __attribute__ ((aligned(x)))
#else
	#define ALIGNED(x)
#endif

#pragma pack(push, 4)

struct Vec2
{
	float x;
	float y;

	static Vec2 one;
	static Vec2 zero;
	static Vec2 half;

	Vec2()
	{
	}

	Vec2(float _x, float _y) : x(_x), y(_y)
	{
	}

	Vec2(Vec2 const &o)
	{
		x = o.x;
		y = o.y;
	}

	explicit Vec2(Point2D const &p);
	explicit Vec2(Size2D const &s);

	Vec2 const &operator = (Vec2 const &b)
	{
		x = b.x;
		y = b.y;
		return *this;
	}

	Vec2 const &operator += (Vec2 const &b)
	{
		x += b.x;
		y += b.y;
		return *this;
	}

	Vec2 const &operator -= (Vec2 const &b)
	{
		x -= b.x;
		y -= b.y;
		return *this;
	}

	Vec2 const &operator *= (Vec2 const &b)
	{
		x *= b.x;
		y *= b.y;
		return *this;
	}

	Vec2 const &operator /= (Vec2 const &b)
	{
		x /= b.x;
		y /= b.y;
		return *this;
	}

	Vec2 const &operator *= (float f)
	{
		x *= f;
		y *= f;
		return *this;
	}

	Vec2 const &operator /= (float f)
	{
		x /= f;
		y /= f;
		return *this;
	}

	Vec2 operator - ()
	{
		return Vec2(-x, -y);
	}

	void Set(float x, float y)
	{
		this->x = x;
		this->y = y;
	}

	void Reset()
	{
		x = y = 0;
	}

	float Dot(Vec2 const &o) const
	{
		return x * o.x + y * o.y;
	}

	float Cross(Vec2 const &o) const
	{
		return x * o.y - y * o.x;
	}

	float LengthSquared() const
	{
		return x * x + y * y;
	}

	float Length() const
	{
		return sqrtf(LengthSquared());
	}

	Vec2 Normalize();
};

#pragma pack(pop)

//////////////////////////////////////////////////////////////////////

inline bool operator <= (Vec2 const &a, Vec2 const &b)
{
	return a.x <= b.x && a.y <= b.y;
}

//////////////////////////////////////////////////////////////////////

inline bool operator >= (Vec2 const &a, Vec2 const &b)
{
	return a.x >= b.x && a.y >= b.y;
}

//////////////////////////////////////////////////////////////////////

inline bool operator < (Vec2 const &a, Vec2 const &b)
{
	return a.x < b.x && a.y < b.y;
}

//////////////////////////////////////////////////////////////////////

inline bool operator > (Vec2 const &a, Vec2 const &b)
{
	return a.x > b.x && a.y > b.y;
}

//////////////////////////////////////////////////////////////////////

inline bool operator == (Vec2 const &l, Vec2 const &r)
{
	return l.x == r.x && l.y == r.y;
}

//////////////////////////////////////////////////////////////////////

inline bool operator != (Vec2 const &l, Vec2 const &r)
{
	return l.x != r.x || l.y != r.y;
}

//////////////////////////////////////////////////////////////////////

inline Vec2 operator + (Vec2 const &a, Vec2 const &b)
{
	return Vec2(a.x + b.x, a.y + b.y);
}

//////////////////////////////////////////////////////////////////////

inline Vec2 operator - (Vec2 const &a, Vec2 const &b)
{
	return Vec2(a.x - b.x, a.y - b.y);
}

//////////////////////////////////////////////////////////////////////

inline Vec2 operator * (Vec2 const &a, Vec2 const &b)
{
	return Vec2(a.x * b.x, a.y * b.y);
}

//////////////////////////////////////////////////////////////////////

inline Vec2 operator * (Vec2 const &a, float b)
{
	return Vec2(a.x * b, a.y * b);
}

//////////////////////////////////////////////////////////////////////

inline Vec2 operator / (Vec2 const &a, Vec2 const &b)
{
	return Vec2(a.x / b.x, a.y / b.y);
}

//////////////////////////////////////////////////////////////////////

inline Vec2 operator / (Vec2 const &a, float f)
{
	return Vec2(a.x / f, a.y / f);
}

//////////////////////////////////////////////////////////////////////

inline Vec2 Vec2::Normalize()
{
	return *this / Length();
}

//////////////////////////////////////////////////////////////////////

template <> inline Vec2 Min(Vec2 a, Vec2 b)
{
	return Vec2(Min(a.x, b.x), Min(a.y, b.y));
}

//////////////////////////////////////////////////////////////////////

template <> inline Vec2 Max(Vec2 a, Vec2 b)
{
	return Vec2(Max(a.x, b.x), Max(a.y, b.y));
}