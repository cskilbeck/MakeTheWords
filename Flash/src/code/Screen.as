package code
{
	import flash.display.Sprite;
	import flash.events.Event;
	
	public class Screen extends Sprite
	{
		public function Screen()
		{
			super();
		}
		
		public virtual function Preload():void
		{
			
		}
		
		public virtual function Load():void
		{
			
		}

		public virtual function Init():void
		{
			// Called before becoming active
		}
		
		public virtual function Update():void
		{
			// Called each frame when active
		}
		
		protected virtual function Close(reason:int):void
		{
			// default behaviour:
			dispatchEvent(new ScreenExitEvent(Event.CLOSE, reason));
		}
	}
}