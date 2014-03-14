//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace UI
{
	//////////////////////////////////////////////////////////////////////

	Document::Document(int width, int height)
		: mSize(width, height)
		, mContentSize(0,0)
		, mCursor(0,0)
		, mScrollPosition(0,0)
		, mScrollVelocity(0,0)
		, mLinkClickCallback(null)
		, mCurrentLink(null)
		, mGrabbed(false)
	{
		mLinks.reserve(32);
		mText.reserve(8);
		SetLocation(Vec2::zero);
		CalculateHitRect();
	}

	//////////////////////////////////////////////////////////////////////

	Document::~Document()
	{
	}

	//////////////////////////////////////////////////////////////////////

	void Document::Clear()
	{
		mText.clear();
		mCursor.Reset();
		mLinks.clear();
		mContentSize.Clear();
		mScrollVelocity.Reset();
		mScrollPosition.Reset();
		mCurrentLink = null;
	}

	//////////////////////////////////////////////////////////////////////

	void Document::Add(Font *font, string const &text)
	{
		DocumentItem i;
		Vec2 offset;
		size_t linkBase = mLinks.size();

		Vec2 size = font->MeasureString(text.c_str(), offset, &mLinks);
		for(size_t i = linkBase; i != mLinks.size(); ++i)
		{
			mLinks[i].mTopLeft += mCursor;
			mLinks[i].mBottomRight += mCursor;
		}
		i.mFont = font;
		i.mText = text;
		i.mPosition = mCursor;
		mText.push_back(i);
		mCursor.y += size.y;
		if(mCursor.y > mContentSize.h)
		{
			mContentSize.h = (int)mCursor.y;
		}
	}

	//////////////////////////////////////////////////////////////////////

	Vec2 Document::MaxOffset() const
	{
		Vec2 m((float)Max(mContentSize.w - mSize.w, 0), (float)Max(mContentSize.h - mSize.h, 0));
		return m;
	}

	//////////////////////////////////////////////////////////////////////

	void Document::AddNewLine(Font *font)
	{
		mCursor.y += font->GetHeight();
	}

	//////////////////////////////////////////////////////////////////////

	void Document::Draw(SpriteList *spriteList)
	{
		spriteList->SetBlendingMode(SpriteList::eBlendMode::kModulate);
		spriteList->ResetTransform();
		for(auto it = mText.begin(); it != mText.end(); ++it)
		{
			it->mFont->DrawString(spriteList, it->mText, it->mPosition - mScrollPosition, Font::HLeft, Font::VTop);
		}
	}

	//////////////////////////////////////////////////////////////////////

	Link *Document::GetLinkRectangle(Point2D pos)
	{
		pos += Point2D(mScrollPosition);
		for(size_t i = 0; i < mLinks.size(); i++)
		{
			Link &link = mLinks[i];

			float l = link.mTopLeft.x;
			float t = link.mTopLeft.y;
			float r = link.mBottomRight.x;
			float b = link.mBottomRight.y;

			if(pos.x >= l && pos.x <= r && pos.y >= t && pos.y <= b)
			{
				return &link;
			}
		}
		return null;
	}

	//////////////////////////////////////////////////////////////////////
	// Scrolling

	bool Document::Update()
	{
		bool rc = true;
		bool scrolling = false;

		if(Touch(0).Pressed())
		{
			mTouchPosition = Vec2(Touch(0).mPosition);
			mCurrentLink = GetLinkRectangle(Touch(0).mPosition);
			mGrabbed = true;
			rc = false;
		}
		else if(mGrabbed)
		{
			if(Touch(0).Held())
			{
				Vec2 vel (Touch(0).mPosition - Touch(0).mPreviousPosition);
				if(vel.x != 0 || vel.y != 0)
				{
					mScrollVelocity = vel;
					scrolling = true;
				}

				if(mCurrentLink != GetLinkRectangle(Touch(0).mPosition))
				{
					mCurrentLink = null;
				}
			}
			else if(Touch(0).Released())
			{
				mGrabbed = false;
				scrolling = true;
				if(mCurrentLink != null)
				{
					if(mLinkClickCallback != null)
					{
						mLinkClickCallback(mCurrentLink);
					}
				}
				mCurrentLink = null;
			}
			else
			{
				scrolling = true;
			}
		}

		if(scrolling)
		{
			mScrollPosition -= mScrollVelocity;
			mScrollPosition = Min(Max(Vec2::zero, mScrollPosition), MaxOffset());
		}
		mScrollVelocity *= 0.95f;
		return rc;
	}
}
