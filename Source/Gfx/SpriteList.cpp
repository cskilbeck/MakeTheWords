//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

void SpriteList::AddHorizontalLine(Vec2 const &start, float width, Color color)
{
	SetTexture(SpriteList::WhiteTexture());
	AddSprite(start, Vec2(width, 1), color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddVerticalLine(Vec2 const &start, float height, Color color)
{
	SetTexture(SpriteList::WhiteTexture());
	AddSprite(start, Vec2(1, height), color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddSolidRectangle(Vec2 const &topLeft, Vec2 const &size, Color color)
{
	SetTexture(SpriteList::WhiteTexture());
	AddSprite(topLeft, size, color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddOutlinedRectangle(Vec2 const &topLeft, Vec2 const &size, Color lineColor, Color fillColor)
{
	SetTexture(SpriteList::WhiteTexture());
	if(size.x > 1 && size.y > 1)
	{
		AddSprite(topLeft + Vec2(1,1), size - Vec2(2,2), fillColor);
	}
	if(size.x > 2)
	{
		AddSprite(topLeft + Vec2(1,0), Vec2(size.x - 2, 1), lineColor);
		AddSprite(topLeft + Vec2(1,size.y - 1), Vec2(size.x - 2, 1), lineColor);
	}
	if(size.y > 1)
	{
		AddSprite(topLeft, Vec2(1, size.y), lineColor);
		AddSprite(topLeft + Vec2(size.x - 1, 0), Vec2(1, size.y), lineColor);
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddRectangle(Vec2 const &topLeft, Vec2 const &size, Color color)
{
	SetTexture(SpriteList::WhiteTexture());
	if(size.x > 2)
	{
		AddSprite(topLeft + Vec2(1,0), Vec2(size.x - 1, 1), color);
		AddSprite(topLeft + Vec2(1,size.y-1), Vec2(size.x - 1, 1), color);
	}
	if(size.y > 1)
	{
		AddSprite(topLeft, Vec2(1, size.y), color);
		AddSprite(topLeft + Vec2(size.x, 0), Vec2(1, size.y), color);
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddTexturedLine(Vec2 const &start, Vec2 const &end, float width, Color color)
{
	Vec2 n = Vec2(end.y - start.y, start.x - end.x).Normalize() * (width / 2);
	BeginStrip();
	SetPoint(start - n, Vec2::zero, color);
	SetPoint(start + n, Vec2(0, 1), color);
	SetPoint(end - n, Vec2(1, 0), color);
	SetPoint(end + n, Vec2::one, color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddSolidLine(Vec2 const &start, Vec2 const &end, float width, Color color)
{
	SetTexture(SpriteList::WhiteTexture());
	AddTexturedLine(start, end, width, color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddFan(Vec2 const &org, float radius, int numSegments, float startAngle, Vec2 const &uvOrg, float uvRadius, Color color)
{
	float scale = (float)PI_2 / numSegments;
	BeginFan();
	SetPoint(org, uvOrg, color);
	for(int i=0; i<=numSegments; ++i)
	{
		float r = startAngle + i * scale;
		float x = sinf(r);
		float y = cosf(r);
		Vec2 uv(uvOrg + Vec2(uvRadius * x, uvRadius * y));
		Vec2 pos(org + Vec2(radius * x, radius * y));
		SetPoint(pos, uv, color);
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddArc(Vec2 const &origin, float innerRadius, float outerRadius, float startAngle, float endAngle, int numSegments, Color fillColor)
{
	BeginStrip();
	
	float span = endAngle - startAngle;
	float scale = span / numSegments;
	
	for(int i=0; i<=numSegments; ++i)
	{
		float r(i * scale + startAngle);
		Vec2 c(sinf(r), cosf(r));
		SetPoint(origin + c * innerRadius, c, fillColor);
		SetPoint(origin + c * outerRadius, c, fillColor);
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddOblong(Vec2 const &topLeft, Vec2 const &bottomRight, int numSegments, Color fillColor)
{
	AddSolidRectangle(topLeft, bottomRight - topLeft, fillColor);
	float radius = (bottomRight.y - topLeft.y) / 2;
	AddArc(topLeft + Vec2(0, radius), 0, radius, PI, PI * 2, numSegments, fillColor);
	AddArc(bottomRight - Vec2(0, radius), 0, radius, 0, PI, numSegments, fillColor);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddCircle(Vec2 const &org, float radius, int numSegments, Color color)
{
	AddArc(org, 0, radius, 0, PI * 2, numSegments, color);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddRoundedRectangle(Vec2 const &topLeft, Vec2 const &bottomRight, float radius, int numSegments, Color fillColor)
{
	Vec2 dimensions = bottomRight - topLeft;
	Vec2 half = dimensions / 2;
	radius = Min(radius, Min(half.x, half.y));
	SetTexture(SpriteList::WhiteTexture());
	float uvr = 1.0f / radius;
	Vec2 r(radius, radius);
	AddFan(topLeft + r, radius, numSegments, PI, Vec2(uvr, uvr), uvr, fillColor);
	AddFan(Vec2(bottomRight.x - radius, topLeft.y + radius), radius, numSegments, PI_2, Vec2(1-uvr, uvr), uvr, fillColor);
	AddFan(bottomRight - r, radius, numSegments, 0, Vec2(1-uvr, 1-uvr), uvr, fillColor);
	AddFan(Vec2(topLeft.x + radius, bottomRight.y - radius), radius, numSegments, PI + PI_2, Vec2(uvr, 1-uvr), uvr, fillColor);
	AddSolidRectangle(Vec2(topLeft.x + radius, topLeft.y), Vec2(dimensions.x - radius * 2, radius), fillColor);
	AddSolidRectangle(Vec2(topLeft.x + radius, bottomRight.y - radius), Vec2(dimensions.x - radius * 2, radius), fillColor);
	AddSolidRectangle(Vec2(topLeft.x, topLeft.y + radius), Vec2(dimensions.x, dimensions.y - radius * 2), fillColor);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddOutlinedRoundedRectangle(Vec2 const &topLeft, Vec2 const &bottomRight, float innerRadius, float outerRadius, int numSegments, Color fillColor, Color outlineColor)
{
	float d = outerRadius - innerRadius;
	Vec2 o(d,d);
	AddRoundedRectangle(topLeft + o, bottomRight - o, innerRadius, numSegments, fillColor);
	AddArc(topLeft + Vec2(outerRadius, outerRadius), innerRadius, outerRadius, PI, PI + PI_2, numSegments, outlineColor);
	AddArc(Vec2(bottomRight.x - outerRadius, topLeft.y + outerRadius), innerRadius, outerRadius, PI_2, PI_2 + PI_2, numSegments, outlineColor);
	AddArc(bottomRight - Vec2(outerRadius, outerRadius), innerRadius, outerRadius, 0, PI_2, numSegments, outlineColor);
	AddArc(Vec2(topLeft.x + outerRadius, bottomRight.y - outerRadius), innerRadius, outerRadius, PI + PI_2, PI + PI_2 * 2, numSegments, outlineColor);
	Vec2 dim(bottomRight - topLeft);
	float w = bottomRight.x - topLeft.x - outerRadius * 2;
	float h = bottomRight.y - topLeft.y - outerRadius * 2;
	AddSolidRectangle(Vec2(topLeft.x + outerRadius, topLeft.y), Vec2(w, d), outlineColor);
	AddSolidRectangle(Vec2(topLeft.x + outerRadius, bottomRight.y - d), Vec2(w, d), outlineColor);
	AddSolidRectangle(Vec2(topLeft.x, topLeft.y + outerRadius), Vec2(d, h), outlineColor);
	AddSolidRectangle(Vec2(bottomRight.x - d, topLeft.y + outerRadius), Vec2(d, h), outlineColor);
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddArc(Vec2 const &origin, float startAngle, float endAngle, float radius, int numSegments, Color color)
{
	SetTexture(SpriteList::WhiteTexture());
	BeginLines();

	float diff = endAngle - startAngle;
	float scale = diff / numSegments;
	for(int i=0; i<=numSegments; ++i)
	{
		float t = i * scale + startAngle;
		Vec2 p(sinf(t) * radius + origin.x, cosf(t) * radius + origin.y);
		SetPoint(p, Vec2::zero, color);
	}
}

//////////////////////////////////////////////////////////////////////

void SpriteList::AddLine(Vec2 const &start, Vec2 const &end, Color color)
{
	SetTexture(SpriteList::WhiteTexture());
	BeginLines();
	SetPoint(start, Vec2::zero, color);
	SetPoint(end, Vec2::zero, color);
}

