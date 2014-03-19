//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Link
{
	Vec2		mTopLeft;
	Vec2		mBottomRight;
	wstring		mText;
};

//////////////////////////////////////////////////////////////////////

struct Font : RefCount
{
	enum HorizontalAlign
	{
		HLeft = 0,
		HRight = 1,
		HCentre = 2
	};

	enum VerticalAlign
	{
		VTop = 3,
		VCentre = 4,
		VBaseline = 5,
		VBottom = 6
	};

	bool DrawChar(int layer, Vec2 &cursor, SpriteList &spriteList, wchar c, Color color);
	Vec2 MeasureChar(char c, Vec2 *offsetOf = null);
	Vec2 MeasureString(char const *text, Vec2 &offset, vector<Link> *links = null) const;
	int GetHeight() const;
	float GetBaseline() const;
	string WrapText(string txt, uint pixelWidth, string lineBreak);
	void DrawString(SpriteList *spriteList, string const &text, Vec2 const &pos, Font::HorizontalAlign horizAlign = HLeft, Font::VerticalAlign vertAlign = VTop, Color color = 0xffffffff);
	void DrawString(SpriteList *spriteList, char const *text, Vec2 const &pos, HorizontalAlign horizAlign = HLeft, VerticalAlign vertAlign = VTop, Color color = 0xffffffff);
	void DrawString(SpriteList &spriteList, string const &text, Vec2 const &pos, Font::HorizontalAlign horizAlign = HLeft, Font::VerticalAlign vertAlign = VTop, Color color = 0xffffffff);
	void DrawString(SpriteList &spriteList, char const *text, Vec2 const &pos, HorizontalAlign horizAlign = HLeft, VerticalAlign vertAlign = VTop, Color color = 0xffffffff);

	string mName;
	list_node<Font> mListNode;

private:

	friend struct FontManager;

	Font();
	~Font();

	void LoadFromFile(char const *filename);

	typedef std::unordered_map<wchar, int> Map;	// map into Glyph array per char

	struct Glyph;
	struct Layer;
	struct KerningValue;
	struct Graphic;

	float				mBaseline;
	int					mHeight;
	int					mPageCount;
	int					mGlyphCount;
	int					mLayerCount;
	int					mKerningValueCount;
	int					mGraphicCount;
	Map					mGlyphMap;
	Texture **			mPages;
	Glyph *				mGlyphs;
	Layer *				mLayers;
	KerningValue *		mKerningValues;
	Graphic *			mGraphics;
};

//////////////////////////////////////////////////////////////////////

struct FontManager
{
	static Font *Load(char const *name);
};

//////////////////////////////////////////////////////////////////////

