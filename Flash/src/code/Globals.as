package code
{
	import flash.geom.Point;
	import flash.net.SharedObject;
	import flash.system.LoaderContext;
	
	import br.com.stimuli.loading.BulkLoader;

	public class Globals
	{
		public static var MouseX: int;
		public static var MouseY: int;
		public static var MousePosition: Point;
		public static var MousePressed: Boolean;
		public static var MouseHeld: Boolean;
		public static var OldMouseHeld: Boolean;
		public static var MouseReleased: Boolean;
		public static var TimeDelta: Number;
		public static var Time: Number;
		public static var StageWidth:uint;
		public static var StageHeight:uint;
		public static var BuildTimeStamp:Date;
		public static var Protocol:String;
		public static var GameMode:String;
		public static var GameModeIndex:int;
		public static var LocalStore:SharedObject;
		
		//////////////////////////////////////////////////////////////////////
		
		public static var FacebookName: String;
		public static var FacebookID: String;

		//////////////////////////////////////////////////////////////////////
		
		public static const AssetsURLBase: String = "//s3.amazonaws.com/IceCreamSoftware/";
		public static const WebServiceURLBase: String = "//makethewords.com/mtw";
		
		//////////////////////////////////////////////////////////////////////
		
		public static function URL(s:String):String
		{
			return Globals.Protocol + AssetsURLBase + s;
		}

		//////////////////////////////////////////////////////////////////////
		
		public static var Loader: BulkLoader;
		
		public static function AddLoadItem(name:String, needsContext:Boolean=true, type:String=""):void
		{
			var o:Object = new Object();
			//o[BulkLoader.PREVENT_CACHING] = true;
			if(needsContext)
			{
				o[BulkLoader.CONTEXT] = new LoaderContext(true);
			}
			if(type.length > 0)
			{
				o["type"] = type;
			}
			Globals.Loader.add(Globals.URL(name), o);
		}
	}
}