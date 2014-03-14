package code
{
	//////////////////////////////////////////////////////////////////////
	
	// When we ask for the game, get ALL games in progress
	// there should be 2:
	//   3 minute blitz
	//   daily challenge
	// either way, show them all in the UI and allow the player to start
	// if there's a boardid already, use that so they can continue
	
	import flash.display.SimpleButton;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.text.TextField;
	
	//////////////////////////////////////////////////////////////////////
	
	public class UI extends Screen
	{
		//////////////////////////////////////////////////////////////////////
		
		public var mStatus:TextField;
		public var mStartButton:SimpleButton;
		public var mGameModeButton:SimpleButton;
		public var mWebServiceRequestRegister:WebServiceRequest;	// used once to register session
		public var mWebServiceRequestGameInfo:WebServiceRequest;	// used every N seconds to check current game status
		public var mRegistered:Boolean;
		public var mGameInfoText:TextField;
		public var mGameInfo:GameInfo; 
		
		//////////////////////////////////////////////////////////////////////
		
		public function UI()
		{
			super();
			
			mGameInfo = new GameInfo();
			mRegistered = false;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function Register():void
		{
			if(mWebServiceRequestRegister.ReadyToGo())
			{
				mWebServiceRequestRegister.Execute("register", { "facebook_id": Globals.FacebookID });
			}
		}

		//////////////////////////////////////////////////////////////////////
		
		private function RegisteredCallback(evt:Event):void
		{
			var wsr:WebServiceRequest = evt.target as WebServiceRequest;
			var o:Object = wsr.mResult;
			if("error" in o)
			{
				// Hmmm
			}
			else
			{
				Globals.FacebookName = o["name"];
				trace("Registered as " + Globals.FacebookName);
				mRegistered = true;
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function CheckGameStatus():void
		{
			if(mWebServiceRequestGameInfo.ReadyToGo())
			{
				mWebServiceRequestGameInfo.Execute("game", { "facebook_id" : Globals.FacebookID });
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function GameInfoCallback(evt:Event):void
		{
			var wsr:WebServiceRequest = evt.target as WebServiceRequest;
			
			mGameInfo.SetFromServerResults(wsr.mResult);
			if(mGameInfo.mGameID != 0)
			{
				var timeUntilGameStarts:Number = (mGameInfo.mGameStartTime.time - mGameInfo.mGameServerTime.time) / 1000;
				var timeUntilGameEnds:Number = (mGameInfo.mGameEndTime.time  - mGameInfo.mGameServerTime.time) / 1000;
				mGameInfoText.text = "Game " + mGameInfo.mGameID;
				if(timeUntilGameStarts > 0)
				{
					mGameInfoText.text += " starts in " + timeUntilGameStarts;
					mStartButton.enabled = false;
					mStartButton.alpha = 0.25;
				}
				else if(timeUntilGameEnds > 0)
				{
					mGameInfoText.text += " ends in " + timeUntilGameEnds + " seconds";
					mStartButton.enabled = true;
					mStartButton.alpha = 1;
				}
				else
				{
					mGameInfoText.text += " has ended, waiting for results";
					mStartButton.enabled = false;
					mStartButton.alpha = 0.25;
				}
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function OnStartButton(evt:MouseEvent):void
		{
			if(mStartButton.enabled)
			{
				Close(ScreenExitEvent.StartGame);
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Preload():void
		{
			Globals.AddLoadItem("PlayButton.png");
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Load():void
		{
			mStatus = new TextField();
			mStatus.width = Globals.StageWidth;
			mStatus.height = 20;
			mStatus.textColor = 0xffffff;
			mStatus.opaqueBackground = true;
			mStatus.backgroundColor = 0x707070;
			addChild(mStatus);

			mStartButton = new SimpleButton();
			UIButton.CreateGraphicalButton(mStartButton, Globals.Loader.getBitmap(Globals.URL("PlayButton.png")).bitmapData);
			mStartButton.x = 50;
			mStartButton.y = 50;
			mStartButton.enabled = false;
			mStartButton.alpha = 0.25;
			mStartButton.addEventListener(MouseEvent.CLICK, OnStartButton);
			addChild(mStartButton);
			
			mGameInfoText = new TextField();
			mGameInfoText.x = 0;
			mGameInfoText.y = 350;
			mGameInfoText.width = 720;
			mGameInfoText.height = 50;
			mGameInfoText.textColor = 0xffffff;
			mGameInfoText.text = Globals.FacebookID;
			addChild(mGameInfoText);
			
			mGameModeButton = new SimpleButton();
			UIButton.CreateTextButton(mGameModeButton, "Mode: " + Globals.GameMode, 140, 20, 12, "Arial", 0, 0xc0c0c0, 0, 0xd0d0d0, 0, 0xc0c0c0);
			mGameModeButton.x = 560;
			mGameModeButton.y = 300;
			mGameModeButton.addEventListener(MouseEvent.CLICK, OnGameModeClicked);
			addChild(mGameModeButton);
			
			mWebServiceRequestRegister = new WebServiceRequest();
			mWebServiceRequestRegister.addEventListener(Event.COMPLETE, RegisteredCallback);
			
			mWebServiceRequestGameInfo = new WebServiceRequest();
			mWebServiceRequestGameInfo.addEventListener(Event.COMPLETE, GameInfoCallback);
			
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function OnGameModeClicked(evt:MouseEvent):void
		{
			Globals.GameModeIndex = (Globals.GameModeIndex + 1) % GameMode.Modes.length;
			Globals.GameMode = GameMode.Modes[Globals.GameModeIndex];
			UIButton.CreateTextButton(mGameModeButton, "Mode: " + Globals.GameMode, 140, 20, 12, "Arial", 0, 0xc0c0c0, 0, 0xd0d0d0, 0, 0xc0c0c0);
			Globals.LocalStore.data.GameModeIndex = Globals.GameModeIndex;
			Globals.LocalStore.flush();
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Init():void
		{
			mStatus.text = Globals.BuildTimeStamp.toString();
			mStartButton.alpha = 0.25;
			mStartButton.enabled = false;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Update():void
		{
			if(!mRegistered)
			{
				Register();
			}
			else
			{
				CheckGameStatus();
			}
		}
	}
}
