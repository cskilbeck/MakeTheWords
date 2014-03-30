//////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////

namespace Reference
{
	//////////////////////////////////////////////////////////////////////

	struct Dictionary : RefCount
	{
		static Dictionary *	Load(char const *filename);

		void				SaveAsJSON();

		int					WordIndex(char const *word);
		char const *		GetWord(int wordIndex);
		string				GetDefinition(int wordIndex, char const *prepend = " ", char const *appendText = "\n");
	
	private:

		uint8 *			mFile;
		size_t			mFileSize;
		char *			mWord;
		uint32			mNumWords;
		char **			mDefinition;
		uint16			mVersion;

		enum eReferenceType
		{
			kNone,
			kPastTense,
			kPlural,
			kComparative,
			kSuperlative,
			kObsoleteForm,
			kObsoleteSpelling,
			kAltCapitalization,
			kAltSpelling,
			kInformalSpelling,
			kArchaicSpelling,
			kArchaicForm,
			kArchaicThirdPersonSingular,
			kPresentTense,
			kThirdPersonSingular,
			kSecondPersonSingular,
			kMisspelling,
			kAltForm,
			kDatedForm,
			kObsoleteTypography,
			kEyeDialect,
			kAbbreviation,
			kNumReferenceTypes
		};

		enum eWordType
		{
//			kInvalid			= -1,
			kNoun				=  0,
			kVerb				=  1,
			kAdjective			=  2,
			kAdverb				=  3,
			kConjunction		=  4,
			kPronoun			=  5,
			kPreposition		=  6,
			kNumeral			=  7,
			kNumWordTypes		=  8
		};

		Dictionary(char const *filename);
		~Dictionary();

		static char const *sReferenceText[kNumReferenceTypes];
		static char const *sDefinitionNames[kNumWordTypes];
	};

} // ::Reference

using Reference::Dictionary;

extern Dictionary *gDictionary;
