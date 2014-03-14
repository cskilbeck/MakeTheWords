package code
{
	//////////////////////////////////////////////////////////////////////
	
	import flash.events.Event;
	import flash.events.EventDispatcher;
	import flash.events.IOErrorEvent;
	import flash.net.URLLoader;
	import flash.net.URLRequest;
	import flash.net.URLRequestMethod;
	import flash.net.URLVariables;
	import flash.utils.getDefinitionByName;
	
	//////////////////////////////////////////////////////////////////////
	
	public class WebServiceRequest extends EventDispatcher
	{
		//////////////////////////////////////////////////////////////////////
		
		public var mResult:Object;
		public var mURLLoader:URLLoader;
		public var mLastRequestArrivalTime:Number;
		
		//////////////////////////////////////////////////////////////////////
		
		private function RequestComplete(evt:Event):void
		{
			// crack the response into an Object and call the callback
			var u:URLLoader = URLLoader(evt.target);
			var d:String = u.data;
			var o:Object = {};
			var nvp:Array = u.data.split("&");
			for(var s:String in nvp)
			{
				var nameValue:Array = nvp[s].split("=");
				var nameType:Array = nameValue[0].split("-");
				var value:String = unescape(nameValue[1].split("+").join(" "));
				var name:String = nameType[0];
				var type:String = nameType[1];
				switch(type)
				{
					case "long":
					case "int":
						o[name] = Number(value);
						break;
					
					case "str":
						o[name] = String(value);
						break;
					
					case "datetime":
						value = value.split("-").join("/");
						value = value.split(".")[0];
						o[name] = new Date(value);
						break;
					
					default:
						o[name] = value;
						trace("Unknown datatype:" + type);
						break;
				}
			}
			DoCallback(o);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function RequestError(evt:Event):void
		{
			DoCallback( { "error": 11, "errorDescription": "Network problem" } );
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function DoCallback(o:Object):void
		{
			mURLLoader = null;
			mResult = o;
			//Util.DumpObject(o);
			mLastRequestArrivalTime = Globals.Time;
			dispatchEvent(new Event(Event.COMPLETE));
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function IdleTime():Number
		{
			return Globals.Time - mLastRequestArrivalTime;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function Busy():Boolean
		{
			return mURLLoader != null;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function ReadyToGo():Boolean
		{
			return !Busy() && IdleTime() > 1;	// allow server to throttle request rate by sending us a different number than 1 here
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function Execute(action:String, post:Object):void
		{
			var req:URLRequest = new URLRequest(Globals.Protocol + Globals.WebServiceURLBase + "?action=" + action);
			req.method = URLRequestMethod.POST;
			var v:URLVariables = new URLVariables();
			for(var k:String in post)
			{
				v[k] = post[k];
			}
			req.data = v;
			mURLLoader = new URLLoader(req);
			mURLLoader.addEventListener(Event.COMPLETE, RequestComplete);
			mURLLoader.addEventListener(IOErrorEvent.IO_ERROR, RequestError);
			mURLLoader.load(req);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function WebServiceRequest()
		{
			mLastRequestArrivalTime = 0;
			mResult = null;
			mURLLoader = null;
		}
	}
}
