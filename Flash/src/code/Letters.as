package  code
{
	//////////////////////////////////////////////////////////////////////
	
	public class Letters
	{
		//////////////////////////////////////////////////////////////////////
	
		private static var sLetters:Array =
		[
			[1,9],	//A
			[3,2],	//B
			[3,2],	//C
			[2,4],	//D
			[1,12],	//E
			[4,2],	//F
			[2,3],	//G
			[4,2],	//H
			[1,9],	//I
			[8,1],	//J
			[5,1],	//K
			[1,4],	//L
			[3,2],	//M
			[1,6],	//N
			[1,8],	//O
			[3,2],	//P
			[10,1],	//Q
			[1,6],	//R
			[1,4],	//S
			[1,6],	//T
			[1,4],	//U
			[4,2],	//V
			[4,2],	//W
			[8,1],	//X
			[4,2],	//Y
			[10,1]	//Z
		];

		//////////////////////////////////////////////////////////////////////

		private static var sDistribution: Array;
		private static var sRandom: Random;
		private static var sA: int;
		private static var sLowerCaseA: int;
		
		//////////////////////////////////////////////////////////////////////
	
		public static function Open():void
		{
			sDistribution = [];
			sA = "A".charCodeAt(0);
			sLowerCaseA = "a".charCodeAt(0);
			for(var i:int=0; i<sLetters.length; ++i)
			{
				var score:int = sLetters[i][0];
				var dist:int = sLetters[i][1];
				for(var j:int=0; j<dist; ++j)
				{
					sDistribution.push(i + sA);
				}
			}
			sRandom = new Random(0);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public static function GetLetterScore(letter:int):int
		{
			return sLetters[letter - sA][0];
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public static function SetRandomSeed(seed:int):void
		{
			sRandom.Seed(seed);
		}
		
		//////////////////////////////////////////////////////////////////////

		public static function GetRandomChar():int
		{
			return sDistribution[sRandom.Next() % sDistribution.length];
			//return sRandom.Next() % 26 + sA;
		}
		
		//////////////////////////////////////////////////////////////////////
	
		public static function ScoreWord(word:String):int
		{
			var score:int = 0;
			var len:int = word.length;
			for(var i:int=0; i<len; ++i)
			{
				var c:int = word.charCodeAt(i) - sLowerCaseA;
				score += sLetters[c][0];
			}
			return score * len;
		}

		// English:
		// the sum of the letter points multiplied by the length of the word

		// Pseudocode:
		// score = 0
		// for each letter in word
		//   score = score + Points(letter)
		// score = score * Length(word)
	}
}
