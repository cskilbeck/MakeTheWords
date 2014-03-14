package code
{
	import flash.display.SimpleButton;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.text.TextField;
	import flash.text.TextFormat;

	//////////////////////////////////////////////////////////////////////
	// Summary
	// Leaderboard
	// 
	
	public class ResultsScreen extends Screen
	{
		//////////////////////////////////////////////////////////////////////

		public var mGameInfo:GameInfo;		
	
		private var mClose:SimpleButton;
		private var mTopBar:Sprite;
		private var mResultsButton:SimpleButton;
		private var mSummaryButton:SimpleButton;
		
		private var mResults:Sprite;
		private var mResultsText:TextField;
		private var mResultsTextFormat:TextFormat;
		private var mSummary:Sprite;
		
		private var mGotResults:Boolean;
		private var mGameWebRequest:WebServiceRequest;
		private var mResultsWebRequest:WebServiceRequest;
		
		//////////////////////////////////////////////////////////////////////
		
		public function ResultsScreen()
		{
			mGotResults = false;
			mResultsWebRequest = new WebServiceRequest();
			mGameWebRequest = new WebServiceRequest();
			mResultsWebRequest.addEventListener(Event.COMPLETE, GotResults);
			mGameWebRequest.addEventListener(Event.COMPLETE, GotGame);
			super();
		}

		//////////////////////////////////////////////////////////////////////
		
		public override function Preload():void
		{
			Globals.AddLoadItem("icon_close.png");
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Load():void
		{
			mTopBar = new Sprite();
			mTopBar.graphics.beginFill(0x606060);
			mTopBar.graphics.drawRect(0, 0, Globals.StageWidth, 34);
			mTopBar.graphics.endFill();
			addChild(mTopBar);
			
			mResultsButton = new SimpleButton();
			UIButton.CreateTextButton(mResultsButton, "Results", 80, 24);
			mResultsButton.x = 10;
			mResultsButton.y = 5;
			mResultsButton.addEventListener(MouseEvent.CLICK, OnResultsButton);
			addChild(mResultsButton);
			
			mSummaryButton = new SimpleButton();
			UIButton.CreateTextButton(mSummaryButton, "Summary", 80, 24);
			mSummaryButton.x = 100;
			mSummaryButton.y = 5;
			mSummaryButton.addEventListener(MouseEvent.CLICK, OnSummaryButton);
			addChild(mSummaryButton);
			
			mClose = new SimpleButton();
			UIButton.CreateGraphicalButton(mClose, Globals.Loader.getBitmap(Globals.URL("icon_close.png")).bitmapData);
			mClose.x = Globals.StageWidth - mClose.width;
			mClose.y = 0;
			mClose.addEventListener(MouseEvent.CLICK, function(evt:MouseEvent):void
			{
				Close(ScreenExitEvent.Quit);
			});
			addChild(mClose);
			
			mSummary = new Sprite();

			mResults = new Sprite();
			addChild(mResults);
			
			mResultsText = new TextField();
			mResultsText.width = Globals.StageWidth - 50;
			mResultsText.height = Globals.StageHeight - 50;
			mResultsText.x = 50;
			mResultsText.y = 50;
			mResultsText.text = "Waiting for results...";
			mResultsTextFormat = new TextFormat("Arial", 22, 0xffffff, true); 
			mResultsTextFormat.tabStops = [ 30, 100, 200 ];
			mResultsText.setTextFormat(mResultsTextFormat);
			mResults.addChild(mResultsText);
		}

		//////////////////////////////////////////////////////////////////////
		
		private function OnSummaryButton(evt:MouseEvent):void
		{
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function OnResultsButton(evt:MouseEvent):void
		{
			
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Init():void
		{
			mGotResults = false;
			mResultsText.text = "Waiting for results...";
			mResultsText.setTextFormat(mResultsTextFormat);
		}

		//////////////////////////////////////////////////////////////////////
		
		private function GotResults(evt:Event):void
		{
			var o:Object = (evt.target as WebServiceRequest).mResult;
			if(!("error" in o))
			{
				mGotResults = true;
				var rows:uint = o["rows"];
				var totalRows:uint = o["total_rows"];

				mResultsText.text = "";
				for(var i:uint=0; i<rows; ++i)
				{
					var s:String = "\t" + o["ranking" + i] + "\t" + o["score" + i] + "\t" + o["name" + i]; 
					mResultsText.text += s + "\n";
				}
				mResultsText.setTextFormat(mResultsTextFormat);
			}
			else
			{
				// some error getting the leaderboard...
				// another request will occur from Update()\GotGame()
			}
		}

		//////////////////////////////////////////////////////////////////////
		
		private function GotGame(evt:Event):void
		{
			var o:Object = (evt.target as WebServiceRequest).mResult;
			if(o["rank_calculated"] != 0)
			{
				mResultsWebRequest.Execute("results", { "game_id": mGameInfo.mGameID, "board_id": mGameInfo.mBoardID });
			}
			else
			{
				var d:int = int(Globals.Time) % 3;
				var s:String = "Waiting for results.";
				for(var i:int=0; i<d; ++i)
				{
					s += ".";
				}
				mResultsText.text = s;
				mResultsText.setTextFormat(mResultsTextFormat);
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Update():void
		{
			if(!mGotResults && mGameWebRequest.ReadyToGo())
			{
				mGameWebRequest.Execute("game", { "game_id" : mGameInfo.mGameID });
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
	}
}