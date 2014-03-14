//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace Reference
{

	//////////////////////////////////////////////////////////////////////

	wstring Lookup::GetReferenceText(Dictionary const *d) const
	{
		Definition const &def = d->mDefinitions[mIndex];
		return wstring(d->mReferenceTypes[mData]) + L" '" + d->mWords[def.mWordIndex].mEnglishText + L"' ";
	}

	//////////////////////////////////////////////////////////////////////

	Lookups::Lookups(Dictionary *dictionary) : mDictionary(dictionary)
	{
	}

	//////////////////////////////////////////////////////////////////////

	int Lookups::AddDefinition(int id)
	{
		int rc = mLookups.size();
		Lookup l;
		l.mIndex = (uint16)id;
		l.mKind = Lookup::Kind::kDefinition;
		l.mData = 0;
		mLookups.push_back(l);
		return rc;
	}

	//////////////////////////////////////////////////////////////////////

	int Lookups::AddReference(int refType, int id)
	{
		int rc = mLookups.size();
		Lookup l;
		l.mIndex = (uint16)id;
		l.mKind = Lookup::Kind::kReference;
		l.mData = (uint8)refType;
		mLookups.push_back(l);
		return rc;
	}

	//////////////////////////////////////////////////////////////////////

	wstring Lookups::GetDefinition(Word const *word) const
	{
		wstringstream def;
		int le = word->mLookupStart + word->mLookupCount;
		for(int li = word->mLookupStart; li < le; ++li)
		{
			Lookup const &l = mLookups[li];

			if(l.mKind == Lookup::kReference)
			{
				def << l.GetReferenceText(mDictionary);
			}

			Definition const &d = mDictionary->mDefinitions[l.mIndex];
			def << mDictionary->GetWordClass(d.mWordClass) << L" " << d.mText << L"\n";
		}
		return def.str();
	}

	//////////////////////////////////////////////////////////////////////

	void Lookups::Release()
	{
		mLookups.clear();
	}

} // ::Dictionary