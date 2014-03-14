//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace UI
{
	//////////////////////////////////////////////////////////////////////

	struct Image : virtual Item
	{
		//////////////////////////////////////////////////////////////////////

		Image(Texture *texture, Vec2 pos, Vec2 size, Point2D uvtl, Point2D uvbr) : Item()
		{
			mTexture = texture;
			mLocation = pos;
			mSize = size;
			mOrigin = pos + size / 2;
			Vec2 tsize(texture->GetSize());
			mUVTopLeft = Vec2(uvtl) / tsize;
			mUVBottomRight = Vec2(uvbr) / tsize;
			CalculateHitRect();
		}

		//////////////////////////////////////////////////////////////////////

		Image(Texture *texture, Vec2 pos, Vec2 size) : Item()
		{
			mTexture = texture;
			mLocation = pos;
			mSize = size;
			mOrigin = pos + size / 2;
			mUVTopLeft = Vec2::zero;
			mUVBottomRight = Vec2::one;
			CalculateHitRect();
		}

		//////////////////////////////////////////////////////////////////////

		Image(Texture *texture, Vec2 pos) : Item()
		{
			mTexture = texture;
			mLocation = pos;
			mSize = Vec2(mTexture->GetSize());
			mOrigin = pos + mSize / 2;
			mUVTopLeft = Vec2::zero;
			mUVBottomRight = Vec2::one;
			CalculateHitRect();
		}

		//////////////////////////////////////////////////////////////////////

		void Draw(SpriteList *spriteList)
		{
			spriteList->SetBlendingMode(SpriteList::eBlendMode::kInterpolate);
			spriteList->SetTexture(mTexture);
			spriteList->SetOrigin(mOrigin);
			spriteList->SetScale(mScale);
			spriteList->SetRotation(mAngle);
			spriteList->AddSprite(mLocation, mSize, mUVTopLeft, mUVBottomRight, mColor);
		}

		//////////////////////////////////////////////////////////////////////

		Texture *mTexture;
		Vec2 mUVTopLeft;
		Vec2 mUVBottomRight;
	};

//////////////////////////////////////////////////////////////////////

} // ::UI