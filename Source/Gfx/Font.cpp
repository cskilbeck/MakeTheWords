//////////////////////////////////////////////////////////////////////
// Kerning
// Measurement of layers

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace
{
	linked_list<Font, &Font::mListNode> sAllFonts;
}

//////////////////////////////////////////////////////////////////////

struct Font::KerningValue
{
	wchar otherChar;			// if the char before me was this
	float amount;				// add this to the x cursor pos before drawing
};

//////////////////////////////////////////////////////////////////////

struct Font::Graphic
{
	Vec2 drawOffset;			// add this to the cursor position before drawing it
	Point2D position;				// top left on the texture page
	Point2D size;					// size of graphic in pixels
	int pageIndex;				// which page it's on
};

//////////////////////////////////////////////////////////////////////

struct Font::Glyph
{
	float advance;				// advance the x cursor pos this much after drawing
	wchar character;			// what character this Glyph is for

	Graphic *imageTable;
	int imageTableSize;

	KerningValue *kerningTable;
	int kerningTableSize;
};

//////////////////////////////////////////////////////////////////////

struct Font::Layer
{
	Color color;
	Vec2 offset;
	bool measure;
};

//////////////////////////////////////////////////////////////////////

Font::Font()
	: RefCount()
	, mPages(null)
	, mGlyphs(null)
	, mLayers(null)
	, mKerningValues(null)
	, mGraphics(null)
{
	sAllFonts.push_back(this);
}

//////////////////////////////////////////////////////////////////////

Font::~Font()
{
	sAllFonts.remove(this);
	for(int i=0; i<mPageCount; ++i)
	{
		::Release(mPages[i]);
	}
	Delete(mPages);
	Delete(mGlyphs);
	Delete(mLayers);
	Delete(mKerningValues);
	Delete(mGraphics);
}

//////////////////////////////////////////////////////////////////////

void Font::LoadFromFile(char const *filename)
{
	mName = ToLower(string(filename));

	wstring buffer;
	xml_doc *doc = LoadUTF8XMLFile(Format("%s.bitmapfont", filename).c_str(), buffer);

	node *root = doc->first_node(L"BitmapFont");

	if(root != null)
	{
		mPageCount = GetInt(root, L"PageCount");
		mHeight = GetInt(root, L"Height");
		mBaseline = GetFloat(root, L"Baseline");
			
		mKerningValues = null;
		mKerningValueCount = 0;

		mGraphicCount = 0;

		node *layersNode = root->first_node(L"Layers", 0, false);

		if(layersNode != null)
		{
			mLayerCount = GetInt(layersNode, L"Count");

			if(mLayerCount != 0)
			{
				mPages = new Texture *[mPageCount];

				for(int i=0; i<mPageCount; ++i)
				{
					mPages[i] = Texture::Load(Format("%s%d.png", filename, i).c_str());
				}

				mLayers = new Layer[mLayerCount];

				int layerIndex = 0;

				for(node *layer = layersNode->first_node(); layer != null; layer = layer->next_sibling())
				{
					Layer &la = mLayers[layerIndex++];

					la.offset = Vec2(GetFloat(layer, L"offsetX"), GetFloat(layer, L"offsetY"));
					la.color = GetHex32(layer, L"color");
					la.measure = (layerIndex == 1);	// TODO: see editor
				}

				node *glyphsNode = root->first_node(L"Glyphs", 0, false);

				if(glyphsNode != null)
				{
					mGlyphCount = GetInt(glyphsNode, L"Count");

					if(mGlyphCount != 0)
					{
						mGlyphs = new Glyph[mGlyphCount];
	
						int glyphIndex = 0;

						mGraphics = new Graphic[mLayerCount * mGlyphCount];

						int graphicIndex = 0;

						for(node *glyph = glyphsNode->first_node(); glyph != null; glyph = glyph->next_sibling())
						{
							Glyph &gl = mGlyphs[glyphIndex];

							gl.character = (wchar)GetInt(glyph, L"char");
							gl.advance = GetFloat(glyph, L"advance");

							gl.imageTableSize = GetInt(glyph, L"images");
							gl.imageTable = null;

							gl.kerningTableSize = 0;
							gl.kerningTable = null;

							mGlyphMap.insert(Map::value_type(gl.character, glyphIndex));

							glyphIndex++;

							if(gl.imageTableSize != 0)
							{
								gl.imageTable = mGraphics + graphicIndex;
								node *graphicNode = glyph->first_node();

								int numGraphics = 0;

								while(graphicNode != null)
								{
									Graphic &gr = mGraphics[graphicIndex++];

									gr.drawOffset = Vec2(GetFloat(graphicNode, L"offsetX"), GetFloat(graphicNode, L"offsetY"));
									gr.pageIndex = GetInt(graphicNode, L"page");
									gr.position = Point2D(GetInt(graphicNode, L"x"), GetInt(graphicNode, L"y"));
									gr.size = Point2D(GetInt(graphicNode, L"w"), GetInt(graphicNode, L"h"));

									graphicNode = graphicNode->next_sibling();

									++numGraphics;

									assert(numGraphics < mLayerCount * mGlyphCount);
								}

								assert(numGraphics == gl.imageTableSize);

								mGraphicCount += numGraphics;
							}
						}
					}
				}
			}
		}
	}
	delete doc;
}

//////////////////////////////////////////////////////////////////////

bool Font::DrawChar(int layer, Vec2 &cursor, SpriteList &spriteList, wchar c, Color color)
{
	bool rc = false;
	Map::const_iterator i = mGlyphMap.find(c);
	if(i != mGlyphMap.end())
	{
		Glyph &glyph = mGlyphs[i->second];
		if(layer < glyph.imageTableSize)
		{
			Graphic &graphic = glyph.imageTable[layer];
			Vec2 size((float)graphic.size.x, (float)graphic.size.y);
			spriteList.SetTexture(mPages[graphic.pageIndex]);
			spriteList.AddSprite(cursor + graphic.drawOffset, size, graphic.position, graphic.position + graphic.size, color);
		}
		cursor.x += glyph.advance;
		rc = true;
	}
	return rc;
}

//////////////////////////////////////////////////////////////////////

Vec2 Font::MeasureChar(char c, Vec2 *offsetOf)
{
	Vec2 offset;
	float right;
	float bottom;
	Map::const_iterator i = mGlyphMap.find(c);	// TODO: shame to be messing with unordered_map in the font draw function!
	if(i != mGlyphMap.end())
	{
		Glyph &glyph = mGlyphs[i->second];
		right = glyph.advance;
		bottom = 0;

		int layer = mLayerCount - 1;

		if(layer < glyph.imageTableSize)
		{
			Graphic &g = glyph.imageTable[layer];

			float left = g.drawOffset.x;
			bottom = offset.y + g.drawOffset.y + g.size.y;
			float top = g.drawOffset.y;
			right = Max(right, g.drawOffset.x + g.size.x);

			if(left < offset.x)
			{
				offset.x = left;
			}

			if(top < offset.y)
			{
				offset.y = top;
			}
		}
	}
	if(offsetOf != null)
	{
		*offsetOf = offset;
	}
	return Vec2(right, bottom);
}

//////////////////////////////////////////////////////////////////////

Vec2 Font::MeasureString(char const *text, Vec2 &offset, vector<Link> *links) const
{
	offset.Set(0,0);
	Vec2 mmax(0, (float)mHeight);

	float x = 0;
	float y = 0;

	int layer = mLayerCount - 1;

	bool inLink = false;
	bool oldInLink = false;
	bool inHash = false;
	bool escape = false;

	Vec2 topleft;
	Vec2 bottomRight;

	string linkString;

	char const *textBegin;
	char const *textEnd;

	UTF8Decoder decoder(text);
	wchar c;
	while((c = decoder.Next()) > 0)
	{
		textEnd = decoder.currentPtr;

		if(inHash)
		{
			if(c == '#')
			{
				inHash = false;
			}
			continue;
		}
		else
		{
			if(escape)
			{
				escape = false;
			}
			else
			{
				if(c == '\\')
				{
					escape = true;
					continue;
				}
				else if(c == '@')
				{
					inLink = !inLink;
					if(inLink)
					{
						textBegin = decoder.currentPtr;
					}
					else if(links != null)
					{
						Link l;
						l.mTopLeft = topleft;
						l.mBottomRight = bottomRight;
						l.mText = wstring(textBegin + 1, textEnd);
						links->push_back(l);
					}
					continue;
				}
				else if(c == '#')
				{
					inHash = true;
					continue;
				}
				else
				{
					if(c == '\n')
					{
						x = 0;
						y += mHeight;
						mmax.y += mHeight;
					}
					else
					{
						Map::const_iterator i = mGlyphMap.find(c);	// TODO: shame to be messing with unordered_map in the font draw function!
						if(i != mGlyphMap.end())
						{
							Glyph &glyph = mGlyphs[i->second];
							float right = x + glyph.advance;

							if(layer < glyph.imageTableSize)
							{
								Graphic &g = glyph.imageTable[layer];

								float left = g.drawOffset.x;
								float bottom = offset.y + g.drawOffset.y + g.size.y;
								float top = g.drawOffset.y;
								right = Max(right, g.drawOffset.x + g.size.x);

								if(left < offset.x)
								{
									offset.x = left;
								}

								if(top < offset.y)
								{
									offset.y = top;
								}

								//if(bottom > mmax.y)
								//{
								//	mmax.y = bottom;
								//}

								if(inLink && !oldInLink)
								{
									topleft.x = x + left;
									topleft.y = y;
								}

								bottomRight.y = y + mHeight;
							}
							if(right > mmax.x)
							{
								mmax.x = right;
							}
							bottomRight.x = right;
							x += glyph.advance;
						}
					}
				}
			}
		}
		oldInLink = inLink;
	}
	return mmax;
}

//////////////////////////////////////////////////////////////////////

string Font::WrapText(string txt, uint pixelWidth, string lineBreak)
{
	int lineBreakLength = lineBreak.size();
	uint lastGood = 1;
	Vec2 offset;

	while (MeasureString(txt.c_str(), offset).x > pixelWidth)
	{
		int newGood = -1;
		for (uint i = lastGood; i < txt.size(); ++i)
		{
			if (txt[i] == ' ')
			{
				string newText = txt.substr(0, i);

				if (MeasureString(newText.c_str(), offset).x < pixelWidth)
				{
					newGood = i;
				}
				else
				{
					break;
				}
			}
		}

		if (newGood == -1)
		{
			break;
		}
		else
		{
			lastGood = newGood;
		}

		if (lastGood >= txt.size())
		{
			break;
		}
		else
		{
			txt = txt.substr(0, lastGood) + lineBreak + txt.substr(lastGood + 1);
			lastGood += lineBreakLength;
		}
	}
	return txt;
}

//////////////////////////////////////////////////////////////////////

void Font::DrawString(SpriteList *spriteList, string const &text, Vec2 const &pos, Font::HorizontalAlign horizAlign, Font::VerticalAlign vertAlign)
{
	DrawString(*spriteList, text.c_str(), pos, horizAlign, vertAlign);
}

//////////////////////////////////////////////////////////////////////

void Font::DrawString(SpriteList &spriteList, string const &text, Vec2 const &pos, Font::HorizontalAlign horizAlign, Font::VerticalAlign vertAlign)
{
	DrawString(spriteList, text.c_str(), pos, horizAlign, vertAlign);
}

//////////////////////////////////////////////////////////////////////

void Font::DrawString(SpriteList *spriteList, char const *text, Vec2 const &pos, Font::HorizontalAlign horizAlign, Font::VerticalAlign vertAlign)
{
	DrawString(*spriteList, text, pos, horizAlign, vertAlign);
}

//////////////////////////////////////////////////////////////////////

void Font::DrawString(SpriteList &spriteList, char const *text, Vec2 const &pos, Font::HorizontalAlign horizAlign, Font::VerticalAlign vertAlign)
{
	Vec2 drawOffset;
	Vec2 offset;
	Vec2 stringSize;
	bool measured = false;

	switch(vertAlign)
	{
	case Font::VTop:
		offset.y = 0;
		break;

	case Font::VBaseline:
		offset.y = -mBaseline;
		break;

	case Font::VBottom:
		offset.y = -(float)mHeight;
		break;

	case Font::VCentre:
		stringSize = MeasureString(text, drawOffset);
		measured = true;
		offset.y = -stringSize.y/2;
		break;
	}

	switch(horizAlign)
	{
	case Font::HLeft:
		offset.x = 0;
		break;

	case Font::HCentre:
		if(!measured)
		{
			stringSize = MeasureString(text, drawOffset);
		}
		offset.x = -stringSize.x / 2;
		break;

	case Font::HRight:
		if(!measured)
		{
			stringSize = MeasureString(text, drawOffset);
		}
		offset.x = -stringSize.x;
		break;
	}

	for(int i=0; i<mLayerCount; ++i)
	{
		Layer &l = mLayers[i];
		Vec2 cursor = pos + l.offset + offset;

		uint32 layerColor = l.color;
		uint32 currentColor = Color::White;
		uint32 colorMask = 0xFFFFFFFF;

		bool escape = false;
		bool inHash = false;
		bool inLink = false;
		int inShift = 0;

		Vec2 linkTopLeft;
		Vec2 linkBottomRight;
		int linkChars = 0;

		Color linkColor(224, 192, 64);

		UTF8Decoder decoder(text);
		wchar c;
		while((c = decoder.Next()) > 0)
		{
			if(inHash)
			{
				if(c == '#')
				{
					inHash = false;
				}
				else
				{
					inShift -= 4;

					if(inShift >= 0)
					{
						wchar cl = (wchar)tolower(c);

						if(cl == '-')
						{
							colorMask &= ~0xf << inShift;
						}
						else if(cl >= '0' && cl <= '9')
						{
							colorMask |= 0xf << inShift;
							currentColor |= (cl - '0') << inShift;
						}
						else if(cl >= 'a' && cl <= 'f')
						{
							colorMask |= 0xf << inShift;
							currentColor |= (cl - 'a' + 10) << inShift;
						}
					}
				}
				continue;
			}
			else
			{
				if(escape)
				{
					escape = false;
				}
				else
				{
					if(c == '\\')
					{
						escape = true;
						continue;
					}
					else if(c == '@')
					{
						inLink = !inLink;
						continue;
					}
					else if(c == '\n')
					{
						cursor.x = pos.x + l.offset.x + offset.x;
						cursor.y += mHeight;
						continue;
					}
					else if(c == '#')
					{
						if(!inHash)
						{
							inHash = true;
							colorMask = 0x00000000;
							currentColor = 0x00000000;
							inShift = 32;
						}
						continue;
					}
				}
			}

			Color col = (layerColor & ~colorMask) | (currentColor & colorMask);
			if(inLink)
			{
				++linkChars;
				col = linkColor;
			}
			DrawChar(i, cursor, spriteList, c, col);
		}
	}
}

//////////////////////////////////////////////////////////////////////

int Font::GetHeight() const
{
	return mHeight;
}

//////////////////////////////////////////////////////////////////////

float Font::GetBaseline() const
{
	return mBaseline;
}

//////////////////////////////////////////////////////////////////////

Font *FontManager::Load(char const *name)
{
	string l = ToLower(name);
	for(auto &f: sAllFonts)
	{
		if(f.mName == name)
		{
			return &f;
		}
	}
	Font *f = new Font();
	f->LoadFromFile(name);
	return f;
}
