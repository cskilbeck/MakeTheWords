#include "pch.h"

void UI::TextButton::Draw(SpriteList *spriteList)
{
	spriteList->ResetTransform();
	spriteList->SetScale(mScale);
	spriteList->SetOrigin(mOrigin);
	spriteList->SetBlendingMode(SpriteList::eBlendMode::kInterpolate);
	Vec2 size(mSize.x, (float)mFont->GetHeight());
	spriteList->AddOblong(mLocation - Vec2(-6, 4), mLocation + size + Vec2(-6, 2), 8, mColor);
	TextLabel::Draw(spriteList);
}

void UI::TextButton::CalculateHitRect()
{
	UI::Item::CalculateHitRect();
	mHitRect.mTopLeft.x -= 8;
	mHitRect.mTopLeft.y -= 5;
	mHitRect.mSize.w += 16;
	mHitRect.mSize.h += 8;
}