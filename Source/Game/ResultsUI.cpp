//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

ResultsUI::ResultsUI(SpriteList *spriteList) : Screen(spriteList)
{
	mHeaderFont = FontManager::Load("definitionBody");
	mHeaderBar = new HeaderBar(mSpriteList, mHeaderFont);
	mLeaderboard = new Leaderboard(mSpriteList);
	mSummary = new Summary(mSpriteList);
	mSummary->mVisible = false;
	mSummary->mActive = false;
	mLeaderboard->mVisible = false;
	mLeaderboard->mActive = false;
	mSummary->mVisible = false;
	mSummary->mActive = false;
	mFillRectangle = new UI::Rectangle(Vec2(-2, -2), Vec2((float)::Screen::Width() + 4, 36), Color(0xff, 0x50, 0x50, 0x50), Color::Black, 2);
	mFillRectangle->mHitRect = Rect2D(Point2D(-1, -1), Size2D(0,0));

	mCloseTexture = Texture::Load("icon_close.png");
	mCloseButton = new UI::ImageButton(mCloseTexture, Vec2(::Screen::Width() - 48.0f,   0), Vec2(48, 48));

	AddUIItem(mFillRectangle);
	AddUIItem(mCloseButton);

	mHeaderBar->AddButton("High scores", mLeaderboard);
	mHeaderBar->AddButton("Summary", mSummary);

	mCloseButton->OnPress = [this] ()
	{
		Activate(false);
		if(OnClose != null)
		{
			OnClose();
		}
	};

	Activate(false);

	this->mZIndex = 2;
	mHeaderBar->mZIndex = 3;
	mLeaderboard->mZIndex = 0;
	mSummary->mZIndex = 0;
}

//////////////////////////////////////////////////////////////////////

ResultsUI::~ResultsUI()
{
	RemoveUIItem(mFillRectangle);
	Delete(mFillRectangle);
	RemoveUIItem(mCloseButton);
	Delete(mCloseButton);
	::Release(mCloseTexture);

	Delete(mHeaderBar);
	Delete(mLeaderboard);
	Delete(mSummary);

	::Release(mHeaderFont);
}

//////////////////////////////////////////////////////////////////////

void ResultsUI::Activate(bool active)
{
	mActive = active;
	mVisible = active;
	mHeaderBar->mVisible = active;
	mHeaderBar->mActive = active;
	mHeaderBar->Activate(active ? "Summary" : "");
}
