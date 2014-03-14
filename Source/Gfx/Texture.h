//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

struct Texture : RefCount
{
	int Width() const { return mSize.w; }
	int Height() const { return mSize.h; }
	bool IsValid() const { return impl != null; }

	Size2D const &GetSize() const { return mSize; }
	Vec2 const &Scale() const { return mScale; }

	void Activate();

	Size2D	mSize;
	Vec2	mScale;
	string	mName;

	static Texture *Load(char const *name);
	static Texture *Create(int width, int height, Color color);
	static Texture *FromFBID(uint64 FBID, int width, int height);

	#if defined(IOS)
		static void SetSamplerID(int id);
	#endif

	list_node<Texture> mListNode;

private:

	Texture();
	Texture(char const *name);
	Texture(int w, int h, Color color);
	~Texture();

	struct TextureImpl;
	TextureImpl *impl;
};

//////////////////////////////////////////////////////////////////////

