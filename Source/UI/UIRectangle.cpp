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
			spriteList->AddSolidRectangle(mLocation, mSize, mColor);
		}
		else if(mBorderThickness == 0 && mBorderColor != Color::Transparent)
		{
			spriteList->AddOutlinedRectangle(mLocation, mSize, mBorderColor, mColor);
		}
		else
		{
			spriteList->AddOutlinedRoundedRectangle(mLocation, mSize, (float)mBorderRadius - mBorderThickness, (float)mBorderRadius, 16, mColor, mBorderColor);
		}
	}
}