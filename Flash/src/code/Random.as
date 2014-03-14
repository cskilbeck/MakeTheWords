﻿package code
{
	//////////////////////////////////////////////////////////////////////
	
	public class Random
	{
		//////////////////////////////////////////////////////////////////////
	
		private var mW		: uint;
		private var mZ		: uint;
		
		//////////////////////////////////////////////////////////////////////
	
		public function Seed(seed:uint):void
		{
			mZ = seed;
			mW = ~seed;
		}
		
		//////////////////////////////////////////////////////////////////////
	
		public function Random(seed:uint)
		{
			Seed(seed);
		}

		//////////////////////////////////////////////////////////////////////
	
		public function Next():uint
		{
			mZ = 36969 * (mZ & 65535) + (mZ >>> 16);
			mW = 18000 * (mW & 65535) + (mW >>> 16);
			return (mZ << 16) + mW;
		}

		//////////////////////////////////////////////////////////////////////
	
		public function NextFloat():Number
		{
			return Next() / Number(0xffffffff);
		}
	}
}
