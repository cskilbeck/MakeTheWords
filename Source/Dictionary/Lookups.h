//////////////////////////////////////////////////////////////////////

#pragma  once

//////////////////////////////////////////////////////////////////////

namespace Reference
{

	//////////////////////////////////////////////////////////////////////

	struct Dictionary;

	//////////////////////////////////////////////////////////////////////

	struct Lookup
	{
		enum Kind : uint8
		{
			kDefinition = 0,
			kReference = 1
		};

		Kind	mKind;	// direct definition, or reference
		uint8	mData;	// kind-specific data (refType, if it's a reference, otherwise unused)
		uint16	mIndex;	// Index into Definitions

		wstring GetReferenceText(Dictionary const *dictionary) const;
	};

	//////////////////////////////////////////////////////////////////////

	struct Lookups
	{
		vector<Lookup>	mLookups;
		Dictionary *	mDictionary;

		Lookups(Dictionary *dictionary);
		void Release();
		int AddDefinition(int id);
		int AddReference(int refType, int id);
		wstring GetDefinition(Word const *word) const;
	};

} // ::Dictionary