//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

void DrawHorizontalLine(SpriteList &spriteList, Vec2 const &start, float width, Color color)
{
	spriteList.SetTexture(SpriteList::WhiteTexture());
	spriteList.AddSprite(start, Vec2(width, 1), color);
}

//////////////////////////////////////////////////////////////////////

void DrawVerticalLine(SpriteList &spriteList, Vec2 const &start, float height, Color color)
{
	spriteList.SetTexture(SpriteList::WhiteTexture());
	spriteList.AddSprite(start, Vec2(1, height), color);
}

//////////////////////////////////////////////////////////////////////

void DrawSolidRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &size, Color color)
{
	spriteList.SetTexture(SpriteList::WhiteTexture());
	spriteList.AddSprite(topLeft, size, color);
}

//////////////////////////////////////////////////////////////////////

void DrawSolidOutlinedRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &size, Color lineColor, Color fillColor)
{
	spriteList.SetTexture(SpriteList::WhiteTexture());
	if(size.x > 1 && size.y > 1)
	{
		spriteList.AddSprite(topLeft + Vec2(1,1), size - Vec2(2,2), fillColor);
	}
	if(size.x > 2)
	{
		spriteList.AddSprite(topLeft + Vec2(1,0), Vec2(size.x - 2, 1), lineColor);
		spriteList.AddSprite(topLeft + Vec2(1,size.y - 1), Vec2(size.x - 2, 1), lineColor);
	}
	if(size.y > 1)
	{
		spriteList.AddSprite(topLeft, Vec2(1, size.y), lineColor);
		spriteList.AddSprite(topLeft + Vec2(size.x - 1, 0), Vec2(1, size.y), lineColor);
	}
}

//////////////////////////////////////////////////////////////////////

void DrawRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &size, Color color)
{
	spriteList.SetTexture(SpriteList::WhiteTexture());
	if(size.x > 2)
	{
		spriteList.AddSprite(topLeft + Vec2(1,0), Vec2(size.x - 1, 1), color);
		spriteList.AddSprite(topLeft + Vec2(1,size.y-1), Vec2(size.x - 1, 1), color);
	}
	if(size.y > 1)
	{
		spriteList.AddSprite(topLeft, Vec2(1, size.y), color);
		spriteList.AddSprite(topLeft + Vec2(size.x, 0), Vec2(1, size.y), color);
	}
}

//////////////////////////////////////////////////////////////////////

void DrawTexturedLine(SpriteList &spriteList, Vec2 const &start, Vec2 const &end, float width, Color color)
{
	Vec2 n = Vec2(end.y - start.y, start.x - end.x).Normalize() * (width / 2);
	spriteList.BeginStrip();
	spriteList.AddPoint(start - n, Vec2::zero, color);
	spriteList.AddPoint(start + n, Vec2(0, 1), color);
	spriteList.AddPoint(end - n, Vec2(1, 0), color);
	spriteList.AddPoint(end + n, Vec2::one, color);
}

//////////////////////////////////////////////////////////////////////

void DrawSolidLine(SpriteList &spriteList, Vec2 const &start, Vec2 const &end, float width, Color color)
{
	spriteList.SetTexture(SpriteList::WhiteTexture());
	DrawTexturedLine(spriteList, start, end, width, color);
}

//////////////////////////////////////////////////////////////////////

static void AddFan(SpriteList &spriteList, Vec2 const &org, float radius, int numSegments, float startAngle, Vec2 const &uvOrg, float uvRadius, Color color)
{
	float scale = (float)PI_2 / numSegments;
	spriteList.BeginFan();
	spriteList.AddPoint(org, uvOrg, color);
	for(int i=0; i<=numSegments; ++i)
	{
		float r = startAngle + i * scale;
		float x = sinf(r);
		float y = cosf(r);
		Vec2 uv(uvOrg + Vec2(uvRadius * x, uvRadius * y));
		Vec2 pos(org + Vec2(radius * x, radius * y));
		spriteList.AddPoint(pos, uv, color);
	}
}

//////////////////////////////////////////////////////////////////////

void AddArc(SpriteList &spriteList, Vec2 const &origin, float innerRadius, float outerRadius, float startAngle, float endAngle, int numSegments, Color fillColor)
{
	spriteList.BeginStrip();
	
	float span = endAngle - startAngle;
	float scale = span / numSegments;
	
	for(int i=0; i<=numSegments; ++i)
	{
		float r(i * scale + startAngle);
		Vec2 c(sinf(r), cosf(r));
		spriteList.AddPoint(origin + c * innerRadius, c, fillColor);
		spriteList.AddPoint(origin + c * outerRadius, c, fillColor);
	}
}

//////////////////////////////////////////////////////////////////////

void DrawOblong(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &bottomRight, int numSegments, Color fillColor)
{
	DrawSolidRectangle(spriteList, topLeft, bottomRight - topLeft, fillColor);
	float radius = (bottomRight.y - topLeft.y) / 2;
	AddArc(spriteList, topLeft + Vec2(0, radius), 0, radius, PI, PI * 2, numSegments, fillColor);
	AddArc(spriteList, bottomRight - Vec2(0, radius), 0, radius, 0, PI, numSegments, fillColor);
}

//////////////////////////////////////////////////////////////////////

void DrawCircle(SpriteList &spriteList, Vec2 const &org, float radius, int numSegments, Color color)
{
	AddArc(spriteList, org, 0, radius, 0, PI * 2, numSegments, color);
}

//////////////////////////////////////////////////////////////////////

void DrawRoundedRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &bottomRight, float radius, int numSegments, Color fillColor)
{
	Vec2 dimensions = bottomRight - topLeft;
	Vec2 half = dimensions / 2;
	radius = Min(radius, Min(half.x, half.y));
	spriteList.SetTexture(SpriteList::WhiteTexture());
	float uvr = 1.0f / radius;
	Vec2 r(radius, radius);
	AddFan(spriteList, topLeft + r, radius, numSegments, PI, Vec2(uvr, uvr), uvr, fillColor);
	AddFan(spriteList, Vec2(bottomRight.x - radius, topLeft.y + radius), radius, numSegments, PI_2, Vec2(1-uvr, uvr), uvr, fillColor);
	AddFan(spriteList, bottomRight - r, radius, numSegments, 0, Vec2(1-uvr, 1-uvr), uvr, fillColor);
	AddFan(spriteList, Vec2(topLeft.x + radius, bottomRight.y - radius), radius, numSegments, PI + PI_2, Vec2(uvr, 1-uvr), uvr, fillColor);
	DrawSolidRectangle(spriteList, Vec2(topLeft.x + radius, topLeft.y), Vec2(dimensions.x - radius * 2, radius), fillColor);
	DrawSolidRectangle(spriteList, Vec2(topLeft.x + radius, bottomRight.y - radius), Vec2(dimensions.x - radius * 2, radius), fillColor);
	DrawSolidRectangle(spriteList, Vec2(topLeft.x, topLeft.y + radius), Vec2(dimensions.x, dimensions.y - radius * 2), fillColor);
}

//////////////////////////////////////////////////////////////////////

void DrawOutlinedRoundedRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &bottomRight, float innerRadius, float outerRadius, int numSegments, Color fillColor, Color outlineColor)
{
	float d = outerRadius - innerRadius;
	Vec2 o(d,d);
	DrawRoundedRectangle(spriteList, topLeft + o, bottomRight - o, innerRadius, numSegments, fillColor);
	AddArc(spriteList, topLeft + Vec2(outerRadius, outerRadius), innerRadius, outerRadius, PI, PI + PI_2, numSegments, outlineColor);
	AddArc(spriteList, Vec2(bottomRight.x - outerRadius, topLeft.y + outerRadius), innerRadius, outerRadius, PI_2, PI_2 + PI_2, numSegments, outlineColor);
	AddArc(spriteList, bottomRight - Vec2(outerRadius, outerRadius), innerRadius, outerRadius, 0, PI_2, numSegments, outlineColor);
	AddArc(spriteList, Vec2(topLeft.x + outerRadius, bottomRight.y - outerRadius), innerRadius, outerRadius, PI + PI_2, PI + PI_2 * 2, numSegments, outlineColor);
	Vec2 dim(bottomRight - topLeft);
	float w = bottomRight.x - topLeft.x - outerRadius * 2;
	float h = bottomRight.y - topLeft.y - outerRadius * 2;
	DrawSolidRectangle(spriteList, Vec2(topLeft.x + outerRadius, topLeft.y), Vec2(w, d), outlineColor);
	DrawSolidRectangle(spriteList, Vec2(topLeft.x + outerRadius, bottomRight.y - d), Vec2(w, d), outlineColor);
	DrawSolidRectangle(spriteList, Vec2(topLeft.x, topLeft.y + outerRadius), Vec2(d, h), outlineColor);
	DrawSolidRectangle(spriteList, Vec2(bottomRight.x - d, topLeft.y + outerRadius), Vec2(d, h), outlineColor);
}

//////////////////////////////////////////////////////////////////////

void DrawArc(SpriteList &spriteList, Vec2 const &origin, float startAngle, float endAngle, float radius, int numSegments, Color color)
{
	spriteList.SetTexture(SpriteList::WhiteTexture());
	spriteList.BeginLines();

	float diff = endAngle - startAngle;
	float scale = diff / numSegments;
	for(int i=0; i<=numSegments; ++i)
	{
		float t = i * scale + startAngle;
		Vec2 p(sinf(t) * radius + origin.x, cosf(t) * radius + origin.y);
		spriteList.AddPoint(p, Vec2::zero, color);
	}
}

//////////////////////////////////////////////////////////////////////

void DrawLine(SpriteList &spriteList, Vec2 const &start, Vec2 const &end, Color color)
{
	spriteList.SetTexture(SpriteList::WhiteTexture());
	spriteList.BeginLines();
	spriteList.AddPoint(start, Vec2::zero, color);
	spriteList.AddPoint(end, Vec2::zero, color);
}

