//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace UI
{
	//////////////////////////////////////////////////////////////////////

	struct DocumentItem
	{
		Font *		mFont;
		string		mText;
		Vec2		mPosition;

		DocumentItem() : mFont(null)
		{
		}
	};

	//////////////////////////////////////////////////////////////////////

	struct Document : Item
	{
		Size2D								mSize;
		Size2D								mContentSize;
		Vec2								mCursor;
		Vec2								mScrollPosition;
		Vec2								mScrollVelocity;
		bool								mGrabbed;
		Vec2								mTouchPosition;		// where the use grabbed
		vector<DocumentItem>				mText;
		vector<Link>						mLinks;
		std::function<void (Link *)>		mLinkClickCallback;
		Link *								mCurrentLink;

		Document(int width, int height);
		~Document();

		void Draw(SpriteList *spriteList) override;
		bool Update() override;

		void Clear();
		void Add(Font *font, string const &text);
		void AddNewLine(Font *font);
		Link *GetLinkRectangle(Point2D pos);

		Vec2 MaxOffset() const;
	};
}
