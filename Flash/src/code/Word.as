package code
{
	//////////////////////////////////////////////////////////////////////

	public class Word
	{
		//////////////////////////////////////////////////////////////////////

		public var mX: int;
		public var mY: int;
		public var mLength: int;
		public var mDirection: int;
		public var mWordIndex: int;
		public var mScore: int;

		//////////////////////////////////////////////////////////////////////

		public function toString():String
		{
			return "(" + mX.toString() + "," + mY.toString() + ") " + mScore.toString() + "[" + mDirection.toString() + "] = " + Dictionary.sWords[mWordIndex];
		}

		//////////////////////////////////////////////////////////////////////

		public function Word(x:int, y:int, length:int, direction:int, wordIndex:int)
		{
			mX = x;
			mY = y;
			mLength = length;
			mDirection = direction;
			mWordIndex = wordIndex;
			mScore = Letters.ScoreWord(Dictionary.sWords[mWordIndex]);
		}
	}
}