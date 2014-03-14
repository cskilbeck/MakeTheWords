//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace UI
{
	//////////////////////////////////////////////////////////////////////

	struct Screen;

	struct Item
	{
		Item()
			: mColor(0xffffffff)
			, mAngle (0)
			, mScale(Vec2::one)
			, mOrigin(Vec2::zero)
		{
			CalculateHitRect();
		}

		Item(Vec2 pos)
			: mColor(0xffffffff)
			, mAngle (0)
			, mScale(Vec2::one)
			, mOrigin(Vec2::zero)
			, mLocation(pos)
		{
			mSize = Vec2(0,0);	// hmm
			CalculateHitRect();
		}

		virtual ~Item();

		virtual void Draw(SpriteList *spriteList) = 0;

		virtual bool Update()
		{
			return true;
		}

		//////////////////////////////////////////////////////////////////////

		virtual void CalculateHitRect()
		{
			mHitRect = Rect2D(mLocation, mSize);
		}

		//////////////////////////////////////////////////////////////////////

		bool Contains(Point2D const &p)
		{
			return mHitRect.Contains(p);
		}

		//////////////////////////////////////////////////////////////////////

		virtual void SetLocation(Vec2 const &location)
		{
			mLocation = location;
			CalculateHitRect();
		}

		//////////////////////////////////////////////////////////////////////

		virtual void SetSize(Vec2 const &size)
		{
			mSize = size;
			CalculateHitRect();
		}

		//////////////////////////////////////////////////////////////////////

		virtual Vec2 GetSize() const
		{
			return mSize;
		}

		//////////////////////////////////////////////////////////////////////

		virtual void SetOrigin(Vec2 const &origin)
		{
			mOrigin = origin;
		}

		//////////////////////////////////////////////////////////////////////

		virtual Vec2 GetOrigin() const
		{
			return mOrigin;
		}


		Rect2D		mHitRect;
		Vec2		mScale;
		float		mAngle;
		uint32		mColor;
		Screen *	mParent;

		list_node<Item>	mListNode;

	protected:

		Vec2		mLocation;
		Vec2		mSize;
		Vec2		mOrigin;

	};

//////////////////////////////////////////////////////////////////////

} // ::UI

