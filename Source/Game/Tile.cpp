//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace Game
{

	//////////////////////////////////////////////////////////////////////

	Timer Tile::sTimer;
	Texture *Tile::sTileTexture = null;
	Font *Tile::sTileFont = null;
	SpriteList *Tile::sSpriteList = null;

	//////////////////////////////////////////////////////////////////////

	void Tile::Open(SpriteList *spriteList)
	{
		sTimer.Reset();
		Tile::sSpriteList = spriteList;
		sTileTexture = Texture::Load("allColour.png");
		sTileFont = FontManager::Load("Cooper_Black_48");
	}

	//////////////////////////////////////////////////////////////////////

	void Tile::Close()
	{
		Release(sTileTexture);
		Release(sTileFont);
	}

	//////////////////////////////////////////////////////////////////////

	bool Tile::Update(float deltaTime)
	{
		if(mState != mOldState)
		{
			mStateTime = 0;
			mOldState = mState;
		}
		else
		{
			mStateTime += deltaTime;
		}

		switch(mState)
		{
			case kLerp:
			case kBeingDragged:
				{
					Vec2 diff = mTarget - mPosition;
					if(fabsf(diff.x) < 4 && fabsf(diff.y) < 4)
					{
						mPosition = mTarget;
						mAtRest = true;
					}
					else
					{
						mAtRest = false;
						mPosition += diff * deltaTime * 20;
					}
				}
				break;

			case kSelected:
				mPosition = mOrigin;
				break;

			case kDrop:
				mPosition.y += (float)(mDropVelocity * g_DeltaTime);
				mDropVelocity += (float)(2000 * g_DeltaTime);
				if(mPosition.y > mTarget.y)
				{
					if(mDropVelocity > 500)
					{
						mPosition.y -= (float)(mDropVelocity * g_DeltaTime);
						mDropVelocity *= -0.5f;
					}
					else
					{
						mPosition = mTarget;
						mState = kLerp;
						mLayer = 0;
						mAtRest = true;
					}
				}
				break;

			default:
				break;
		}
		return mAtRest;
	}

	//////////////////////////////////////////////////////////////////////

	void Tile::ResetPosition()
	{
		mPosition = mTarget = mOrigin;
		mAtRest = true;
	}

	//////////////////////////////////////////////////////////////////////

	void Tile::SetupTransform(Point2D const &tileSize, float scaleFactor)
	{
		Vec2 ts(tileSize);
		Vec2 ts2(ts/2);
		Vec2 pos = mPosition;
		float scale = 1.0f;
		float rot = 0.0f;
		Vec2 origin = Vec2::zero;
		switch(mState)
		{
			case kBeingDragged:
				sSpriteList->ResetTransform();
				sSpriteList->SetOrigin(mPosition + ts2);
				sSpriteList->SetScale(Vec2(scaleFactor * 1.25f, scaleFactor * 1.25f));
				break;

			case kLerp:
			case kDrop:
				sSpriteList->ResetTransform();
				sSpriteList->SetOrigin(mPosition + ts2);
				sSpriteList->SetScale(Vec2(scaleFactor, scaleFactor));
				break;

			case kSelected:
				rot = (float)(sinf((float)sTimer.GetElapsed() * 25) * 0.1f);
				scale += (float)(cosf((float)sTimer.GetElapsed() * 23) * 0.05f) + 0.333f;
				origin += ts2;
				pos += origin;
				sSpriteList->SetOrigin(mPosition + ts2);
				sSpriteList->SetRotation(rot);
				scale *= scaleFactor;
				sSpriteList->SetScale(Vec2(scale, scale));
				break;

			default:
				break;
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Tile::DrawLetter(Point2D const &tileSize, float letterScale)
	{
		SetupTransform(tileSize, letterScale);
		char str[2];
		str[0] = (char)mLetter;
		str[1] = 0;
		sSpriteList->SetBlendingMode(SpriteList::eBlendMode::kModulate);
		Vec2 offset(tileSize / 2);
		offset.y += sTileFont->GetBaseline() / 4;	// TODO: fix this letter offset hassle!!
		Color c = Color::White;
		if(mState == kBeingDragged)
		{
			c = 0xC0FFFFFF;
		}
		else if(mFlags(kHoveredOver))
		{
			c = 0xFF80FF40;
		}
		sTileFont->DrawString(sSpriteList, str, mPosition + offset, Font::HCentre, Font::VCentre, c);
	}

	//////////////////////////////////////////////////////////////////////

	void Tile::DrawTile(Point2D const &tileSize, Point2D const &tileSourceSize)
	{
		SetupTransform(tileSize, 1.0f);

		Vec2 ts(tileSize);

		Point2D uv((int)mHorizontalWordPosition * tileSourceSize.x, (int)mVerticalWordPosition * tileSourceSize.y);

		if(mState == kSelected)
		{
			uv.Clear();
		}
		if (mHorizontalWord == null && mVerticalWord == null)
		{
			uv.y += 384;
		}
		sSpriteList->SetTexture(sTileTexture);
		sSpriteList->SetBlendingMode(SpriteList::eBlendMode::kInterpolate);

		Color c = 0xffffffff;
		if(mState == kBeingDragged)
		{
			c = 0xA0FFFFFF;
		}
		else if(mFlags(kHoveredOver))
		{
			c = 0xFF80FF40;
		}

		sSpriteList->AddSprite(mPosition, ts, uv, uv + tileSourceSize, c);
		//g_DebugFont->DrawString(g_DebugSpriteList, Format("%d", mIndex).c_str(), mPosition);
	}

	//////////////////////////////////////////////////////////////////////

	void Tile::SetWord(Word *w, int index)
	{
		WordPosition p;
		if (index == 0)
		{
			p = WordPosition::Beginning;
		}
		else if (index == w->mLength - 1)
		{
			p = WordPosition::End;
		}
		else
		{
			p = WordPosition::Middle;
		}
		switch (w->mOrientation)
		{
		case Word::Orientation::horizontal:
			mHorizontalWord = w;
			mHorizontalIndex = index;
			mHorizontalWordPosition = p;
			break;

		case Word::Orientation::vertical:
			mVerticalWord = w;
			mVerticalIndex = index;
			mVerticalWordPosition = p;
			break;
		}
	}

} // ::Game