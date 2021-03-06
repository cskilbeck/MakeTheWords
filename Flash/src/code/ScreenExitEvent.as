package code
{
	//////////////////////////////////////////////////////////////////////
	
	import flash.events.Event;
	
	//////////////////////////////////////////////////////////////////////
	
	public class ScreenExitEvent extends Event
	{
		//////////////////////////////////////////////////////////////////////
		
		private var mReason:int;
		
		//////////////////////////////////////////////////////////////////////
		
		public static const StartGame:int = 0;
		public static const Quit:int = 1;
		public static const OutOfTime:int = 2;
		
		//////////////////////////////////////////////////////////////////////
		
		public function ScreenExitEvent(type:String, reason:int, bubbles:Boolean=false, cancelable:Boolean=false)
		{
			mReason = reason;
			super(type, bubbles, cancelable);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function get Reason():int
		{
			return mReason;
		}
	}
}
