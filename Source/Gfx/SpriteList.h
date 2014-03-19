//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct SpriteList : RefCount
{
	static SpriteList *Create(int maxSprites = 4096, int maxRuns = 0);
	static void SubmitAll();

	void Submit();
	int	RunsSubmitted() const;

	enum eBlendMode
	{
		kModulate,			// 1, 1-a		(letters, text)
		kInterpolate,		// a, 1-a		(tiles)
		kAdditive			// 1, 1
	};
	
	bool Init(int maxSprites = 4096, int maxRuns = 0);
	void SetTexture(Texture *texture);
	void SetScale(Vec2 const &scale);
	void SetRotation(float radians);
	void SetOrigin(Vec2 const &origin);
	void SetBlendingMode(eBlendMode mode);
	void ResetTransform();

	void BeginFan();
	void BeginStrip();
	void BeginLines();
	void SetPoint(Vec2 const &pos, Vec2 const &uv, Color color);

	void AddSprite(Vec2 const &pos, Vec2 const &size, Color color);
	void AddSprite(Vec2 const &pos, Vec2 const &size, Vec2 const &uvTopLeft, Vec2 const &uvBottomRight, Color color);
	void AddSprite(Vec2 const &pos, Vec2 const &size, Point2D const &uvTopLeft, Point2D const &uvBottomRight, Color color);
	void AddHorizontalLine(Vec2 const &start, float width, Color color);
	void AddVerticalLine(Vec2 const &start, float height, Color color);
	void AddSolidRectangle(Vec2 const &topLeft, Vec2 const &size, Color color);
	void AddOutlinedRectangle(Vec2 const &topLeft, Vec2 const &size, Color lineColor, Color fillColor);
	void AddRectangle(Vec2 const &topLeft, Vec2 const &size, Color color);
	void AddSolidLine(Vec2 const &start, Vec2 const &end, float width, Color color);
	void AddTexturedLine(Vec2 const &start, Vec2 const &end, float width, Color color);
	void AddRoundedRectangle(Vec2 const &topLeft, Vec2 const &bottomRight, float radius, int numSegments, Color fillColor);
	void AddOutlinedRoundedRectangle(Vec2 const &topLeft, Vec2 const &bottomRight, float innerRadius, float outerRadius, int numSegments, Color fillColor, Color outlineColor);
	void AddArc(Vec2 const &origin, float startAngle, float endAngle, float radius, int numSegments, Color color);
	void AddLine(Vec2 const &start, Vec2 const &end, Color color);
	void AddCircle(Vec2 const &org, float radius, int numSegments, Color color);
	void AddOblong(Vec2 const &topLeft, Vec2 const &bottomRight, int numSegments, Color fillColor);
	void AddArc(Vec2 const &origin, float innerRadius, float outerRadius, float startAngle, float endAngle, int numSegments, Color fillColor);
	void AddFan(Vec2 const &org, float radius, int numSegments, float startAngle, Vec2 const &uvOrg, float uvRadius, Color color);

	static Texture *WhiteTexture();

	struct SpriteListImpl;
	SpriteListImpl *impl;

	list_node<SpriteList> mListNode;

private:

	SpriteList();
	SpriteList(SpriteList const &o);
	SpriteList &operator = (SpriteList const &p);
	~SpriteList();
};

//////////////////////////////////////////////////////////////////////

