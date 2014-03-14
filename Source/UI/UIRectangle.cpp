//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace UI
{
	//////////////////////////////////////////////////////////////////////

	Rectangle::Rectangle(Vec2 topLeft, Vec2 bottomRight, Color backgroundColor, Color borderColor, int borderWidth, int cornerRadius)
		: Item(topLeft)
		, mBorderColor(borderColor)
		, mBorderThickness(borderWidth)
		, mBorderRadius(cornerRadius)
	{
		mColor = backgroundColor;
		mLocation = topLeft;
		mSize = bottomRight - topLeft;
	}

	Rectangle::~Rectangle()
	{
	}

	void Rectangle::Draw(SpriteList *spriteList)
	{
		spriteList->SetBlendingMode(SpriteList::eBlendMode::kInterpolate);
		if(mBorderThickness == 0 || mBorderColor == Color::Transparent)
		{
			DrawSolidRectangle(*spriteList, mLocation, mSize, mColor);
		}
		else if(mBorderThickness == 0 && mBorderColor != Color::Transparent)
		{
			DrawSolidOutlinedRectangle(*spriteList, mLocation, mSize, mBorderColor, mColor);
		}
		else
		{
			DrawOutlinedRoundedRectangle(*spriteList, mLocation, mSize, (float)mBorderRadius - mBorderThickness, (float)mBorderRadius, 16, mColor, mBorderColor);
		}
	}
}