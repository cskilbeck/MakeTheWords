//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

namespace
{
	char *distribution;
	int distributionTotal;
	static Random randy;
};

//////////////////////////////////////////////////////////////////////

namespace Reference
{
	Letter Letter::sLetters[] =
	{
		1,9,	//A
		3,2,	//B
		3,2,	//C
		2,4,	//D
		1,12,	//E
		4,2,	//F
		2,3,	//G
		4,2,	//H
		1,9,	//I
		8,1,	//J
		5,1,	//K
		1,4,	//L
		3,2,	//M
		1,6,	//N
		1,8,	//O
		3,2,	//P
		10,1,	//Q
		1,6,	//R
		1,4,	//S
		1,6,	//T
		1,4,	//U
		4,2,	//V
		4,2,	//W
		8,1,	//X
		4,2,	//Y
		10,1	//Z
	};

	//////////////////////////////////////////////////////////////////////

	void Letter::Open()
	{
		distributionTotal = 0;
		for(int i=0; i<ARRAYSIZE(sLetters); ++i)
		{
			distributionTotal += sLetters[i].mDistribution;
		}
		distribution = new char[distributionTotal];
		char *d = distribution;
		for(int i=0; i<ARRAYSIZE(sLetters); ++i)
		{
			Letter &l = sLetters[i];
			for(int j=0; j<l.mDistribution; ++j)
			{
				*d++ = (char)i + 'A';
			}
		}
	}

	//////////////////////////////////////////////////////////////////////

	void Letter::Close()
	{
		SafeDeleteArray(distribution);
	}

	//////////////////////////////////////////////////////////////////////

	void Letter::SetRandomSeed(uint32 seed)
	{
		randy.Seed(seed);
	}

	//////////////////////////////////////////////////////////////////////

	int Letter::GetRandomLetter()
	{
		return distribution[randy.Next() % distributionTotal];
	}

	//////////////////////////////////////////////////////////////////////

	int Letter::GetWordScore(char const *word)
	{
		int score = 0;
		int len = 0;
		for(char const *p = word; *p != 0; ++p, ++len)
		{
			score += sLetters[*p - 'a'].mScore;
		}
		score *= len;
		return score;
	}
}
