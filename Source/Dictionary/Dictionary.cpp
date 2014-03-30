//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

Dictionary *gDictionary = null;

//////////////////////////////////////////////////////////////////////

namespace Reference
{
	//////////////////////////////////////////////////////////////////////

	char const *Dictionary::sReferenceText[kNumReferenceTypes] =
	{
		"",
		"Past tense",
		"Plural",
		"Comparative",
		"Superlative",
		"Obsolete form",
		"Obsolete spelling",
		"Alt. capitalization",
		"Alt. spelling",
		"Informal spelling",
		"Archaic spelling",
		"Archaic form",
		"Archaic 3rd person singular",
		"Present tense",
		"3rd person singular",
		"2nd person singular",
		"Misspelling",
		"Alt. form",
		"Dated form",
		"Obsolete typography",
		"Eye dialect",
		"Abbreviation",
	};

	//////////////////////////////////////////////////////////////////////

	char const *Dictionary::sDefinitionNames[kNumWordTypes] =
	{
		"n",
		"v",
		"adj",
		"adv",
		"conj",
		"pron",
		"prep",
		"numeral"
	};

	//////////////////////////////////////////////////////////////////////

	Dictionary *Dictionary::Load(char const *filename)
	{
		if(gDictionary == null)
		{
			gDictionary = new Dictionary(filename);
			if(gDictionary->mFile == null)
			{
				::Release(gDictionary);
			}
		}
		else
		{
			gDictionary->AddRef();
		}
		return gDictionary;
	}

	//////////////////////////////////////////////////////////////////////

	static inline uint16 GetU16(char const * &d)
	{
		uint16 rc = ((uint8 *)d)[0] | (((uint8 *)d)[1] << 8);
		d += 2;
		return rc;
	}

	//////////////////////////////////////////////////////////////////////

	static inline uint32 GetU24(char const * &d)
	{
		uint32 rc = ((uint8 *)d)[0] | (((uint8 *)d)[1] << 8) | (((uint8 *)d)[2] << 16);
		d += 3;
		return rc;
	}

	//////////////////////////////////////////////////////////////////////

	static inline uint32 GetU32(char const * &d)
	{
		uint32 rc = ((uint8 *)d)[0] | (((uint8 *)d)[1] << 8) | (((uint8 *)d)[2] << 16) | (((uint8 *)d)[3] << 24);
		d += 4;
		return rc;
	}

	//////////////////////////////////////////////////////////////////////

	static inline uint16 GetU16(uint8 const * &d)
	{
		return GetU16((char const * &)d);
	}

	static inline uint32 GetU24(uint8 const * &d)
	{
		return GetU24((char const * &)d);
	}

	static inline uint32 GetU32(uint8 const * &d)
	{
		return GetU32((char const * &)d);
	}

	//////////////////////////////////////////////////////////////////////
	void find_and_replace(string& source, string const& find, string const& replace)
	{
		for(std::string::size_type i = 0; (i = source.find(find, i)) != std::string::npos;)
		{
			source.replace(i, find.length(), replace);
			i += replace.length() - find.length() + 1;
		}
	}

	void Dictionary::SaveAsJSON()
	{
		FILE *f;
		if(fopen_s(&f, "C:\\Users\\chs\\dictionary.json", "w") == 0)
		{
			fputs("Dictionary = {\n", f);
			for(uint i=0; i<mNumWords; ++i)
			{
				char *p = Dictionary::mWord + i * 8;
				string s = GetDefinition(i, "", "\\n");
				find_and_replace(s, "\"", "'");
				fprintf_s(f, "\t\"%s\": \"%s\"%s\n", p, s.c_str(), (i < mNumWords - 1) ? "," : "");
			}
			fputs("}\n", f);
			fclose(f);
		}
	}

	//////////////////////////////////////////////////////////////////////

	// 16 byte header:

	// char [10] = "dictionary"
	// uint16 = (versionMajor << 8) | versionMinor
	// uint32 = numWords

	Dictionary::Dictionary(char const *filename) : mDefinition(null)
	{
		mFile = LoadFile("english.dictionary", &mFileSize);

		if(mFile != null)
		{
			uint8 const *p = mFile;
			uint8 const *bufferEnd = p + mFileSize;

			char const *id = (char const *)p;
			if(strncmp(id, "dictionary", 10) == 0)
			{
				p += 10;

				mVersion = GetU16(p);
				mNumWords = GetU32(p);

				mWord = (char *)p;
				p += mNumWords * 8;

				mDefinition = new char *[mNumWords];

				int wordIndex = 0;
				while(p < bufferEnd)
				{
					mDefinition[wordIndex] = (char *)p;

					uint16 definitionMask = GetU16(p);

					for(int i = 0; i < eWordType::kNumWordTypes; ++i)
					{
						if((definitionMask & (1 << i)) != 0)
						{
							if(*p++ == eReferenceType::kNone)
							{
								p += strlen((char *)p) + 1;
							}
							else
							{
								p += 3;
							}
						}
					}
					++wordIndex;
				}
			}
		}
		SaveAsJSON();
	}

	//////////////////////////////////////////////////////////////////////

	Dictionary::~Dictionary()
	{
		Delete(mDefinition);
		Delete(mFile);
	}

	//////////////////////////////////////////////////////////////////////

	int Dictionary::WordIndex(char const *word)
	{
		int t = mNumWords - 1;
		int b = 0;
		while (b <= t)
		{
			int m = b + (t - b) / 2;
			char *w = mWord + m * 8;

			int c = strcmp(word, w);

			if (c > 0)
			{
				b = m + 1;
			}
			else if (c < 0)
			{
				t = m - 1;
			}
			else
			{
				return m;
			}
		}
		return -1;
	}

	//////////////////////////////////////////////////////////////////////

	char const *Dictionary::GetWord(int wordIndex)
	{
		return mWord + wordIndex * 8;
	}

	//////////////////////////////////////////////////////////////////////

	string Dictionary::GetDefinition(int wordIndex, char const *prepend, char const *appendText)
	{
		string definition;

		if(wordIndex >= 0 && wordIndex < (int)mNumWords)
		{
			char const *def = mDefinition[wordIndex];

			uint16 mask = GetU16(def);

			for(int i=0; i<kNumWordTypes; ++i)
			{
				if((mask & (1 << i)) != 0)
				{
					definition += prepend;
					definition += "(";
					definition += sDefinitionNames[i];
					definition += ") ";

					uint8 type = *def++;

					if(type == kNone)
					{
						definition += def;
						def += strlen(def) + 1;
					}
					else
					{
						uint32 word = GetU24(def);

						definition += sReferenceText[type];
						definition += " of @";
						definition += mWord + word * 8;
						definition += "@";
					}
					definition += appendText;
				}
			}
		}
		return definition;
	}

} // Reference