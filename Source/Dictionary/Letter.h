//////////////////////////////////////////////////////////////////////

#pragma once

namespace Reference
{
	//////////////////////////////////////////////////////////////////////

	struct Letter
	{
		uint8	mScore;
		uint8	mDistribution;

		static Letter sLetters[26];

		static void	Open();
		static void	Close();

		static void SetRandomSeed(uint32 seed);
		static int	GetRandomLetter();
		static int	GetWordScore(char const *word);
	};

}