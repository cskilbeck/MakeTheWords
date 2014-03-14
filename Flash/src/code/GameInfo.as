package code
{
	//////////////////////////////////////////////////////////////////////
	
	public class GameInfo
	{
		//////////////////////////////////////////////////////////////////////

		public var mGameID:uint;
		public var mGameRandomSeed:uint;
		public var mGameStartTime:Date;
		public var mGameEndTime:Date;
		public var mGameServerTime:Date;
		public var mGameLength:uint;
		public var mBoardID:uint;
		public var mBoard:String;

		//////////////////////////////////////////////////////////////////////
		
		public function GameInfo()
		{
			mGameID = 0;
			mBoardID = 0;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function SetFromServerResults(o:Object):void
		{
			if("error" in o)
			{
				mGameID = 0;
				mBoardID = 0;
			}
			else
			{
				mGameID = o["game_id"];
				mGameLength = o["duration"];
				mGameStartTime = o["start_time"];
				mGameEndTime = o["end_time"];
				mGameServerTime = o["current_time"];
				mGameRandomSeed = o["random_seed"];
				mBoardID = o["board_id"];
				mBoard = o["board"];
			}
		}
	}
}