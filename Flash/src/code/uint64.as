package code
{
	public class uint64
	{
		private var mLow:uint;
		private var mHigh:uint;
		
		public function uint64(low:uint = 0, high:uint = 0)
		{
			mLow = low;
			mHigh = high;
		}
		
		public static function FromString(s:String):uint64
		{
			return new uint64();
		}
		
		public function CompareTo(o:uint64):Boolean
		{
			return mLow == o.mLow && mHigh == o.mHigh;
		}
	}
}