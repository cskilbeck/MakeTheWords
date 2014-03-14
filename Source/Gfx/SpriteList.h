//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct SpriteList : RefCount
{
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
	void AddSprite(Vec2 const &pos, Vec2 const &size, Color color);
	void AddSprite(Vec2 const &pos, Vec2 const &size, Vec2 const &uvTopLeft, Vec2 const &uvBottomRight, Color color);
	void AddSprite(Vec2 const &pos, Vec2 const &size, Point2D const &uvTopLeft, Point2D const &uvBottomRight, Color color);
	void BeginFan();
	void BeginStrip();
	void BeginLines();
	void AddPoint(Vec2 const &pos, Vec2 const &uv, Color color);
	void Submit();
	
	int	RunsSubmitted() const;

	static Texture *WhiteTexture();
	static void SubmitAll();

	static SpriteList *Create(int maxSprites = 4096, int maxRuns = 0);

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

void DrawHorizontalLine(SpriteList &spriteList, Vec2 const &start, float width, Color color);
void DrawVerticalLine(SpriteList &spriteList, Vec2 const &start, float height, Color color);
void DrawSolidRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &size, Color color);
void DrawSolidOutlinedRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &size, Color lineColor, Color fillColor);
void DrawRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &size, Color color);
void DrawSolidLine(SpriteList &spriteList, Vec2 const &start, Vec2 const &end, float width, Color color);
void DrawTexturedLine(SpriteList &spriteList, Vec2 const &start, Vec2 const &end, float width, Color color);
void DrawRoundedRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &bottomRight, float radius, int numSegments, Color fillColor);
void DrawOutlinedRoundedRectangle(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &bottomRight, float innerRadius, float outerRadius, int numSegments, Color fillColor, Color outlineColor);
void DrawArc(SpriteList &spriteList, Vec2 const &origin, float startAngle, float endAngle, float radius, int numSegments, Color color);
void DrawLine(SpriteList &spriteList, Vec2 const &start, Vec2 const &end, Color color);
void DrawCircle(SpriteList &spriteList, Vec2 const &org, float radius, int numSegments, Color color);
void DrawOblong(SpriteList &spriteList, Vec2 const &topLeft, Vec2 const &bottomRight, int numSegments, Color fillColor);
void AddArc(SpriteList &spriteList, Vec2 const &origin, float innerRadius, float outerRadius, float startAngle, float endAngle, int numSegments, Color fillColor);
