//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace UI
{

	//////////////////////////////////////////////////////////////////////

	struct TextLabel : virtual Item
	{
		TextLabel(Font *font, string text, Vec2 pos)
			: Item(pos)
			, mText(text)
			, mFont(font)
		{
			Vec2 offset;
			Vec2 size = font->MeasureString(mText.c_str(), offset);
			SetSize(Vec2(size.x, float(font->GetHeight())));
			SetOrigin(mLocation + mSize / 2);
		}

		void Draw(SpriteList *spriteList) override
		{
			spriteList->SetBlendingMode(SpriteList::kModulate);
			spriteList->SetOrigin(mOrigin);
			spriteList->SetScale(mScale);
			mFont->DrawString(spriteList, mText.c_str(), mLocation);
		}

		Font *mFont;
		string mText;
	};

	//////////////////////////////////////////////////////////////////////

} //::UI