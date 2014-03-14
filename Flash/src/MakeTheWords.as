package
{
	//////////////////////////////////////////////////////////////////////
	// Finish In-game UI (timer etc)
	// Finish FrontEnd
	// Finish ShowLeaderboards/Summmary
	// Finish Instructions/Tutorial
	// 
	//////////////////////////////////////////////////////////////////////
	
	import flash.display.LoaderInfo;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.ProgressEvent;
	import flash.net.SharedObject;
	import flash.text.TextField;
	
	import br.com.stimuli.loading.BulkLoader;
	import br.com.stimuli.loading.BulkProgressEvent;
	
	import code.BuildTimeStamp;
	import code.Dictionary;
	import code.Game;
	import code.GameMode;
	import code.Globals;
	import code.Letter;
	import code.Letters;
	import code.ResultsScreen;
	import code.Screen;
	import code.ScreenExitEvent;
	import code.UI;

	//////////////////////////////////////////////////////////////////////
	
	[SWF(width='720',height='600',backgroundColor='#304050',frameRate='60')]
	public class MakeTheWords extends Sprite
	{
		//////////////////////////////////////////////////////////////////////
		
		public var mCurrentScreen: Screen;
		public var mGame: Game;
		public var mUI:UI;
		public var mResultsScreen:ResultsScreen;
		public var mLoading:TextField;
		public var mLocalStore:SharedObject;

		//////////////////////////////////////////////////////////////////////
		
		private function LoaderProgress(evt:BulkProgressEvent):void
		{
			mLoading.text = "Loaded " + int(evt.percentLoaded * 100) + "%";
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function SetScreen(screen:Screen):void
		{
			if(screen != mCurrentScreen)
			{
				if(mCurrentScreen != null)
				{
					removeChild(mCurrentScreen);
				}
				
				mCurrentScreen = screen;
				
				if(mCurrentScreen != null)
				{
					mCurrentScreen.Init();
					addChild(mCurrentScreen);
				}
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function OnResultsExit(evt:ScreenExitEvent):void
		{
			SetScreen(mUI);
		}

		//////////////////////////////////////////////////////////////////////
		
		private function OnGameExit(evt:ScreenExitEvent):void
		{
			switch(evt.Reason)
			{
				case ScreenExitEvent.OutOfTime:
				case ScreenExitEvent.Quit:
					mResultsScreen.mGameInfo = mGame.mGameInfo;	// ref!
					SetScreen(mResultsScreen);
					break;
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function OnUIExit(evt:ScreenExitEvent):void
		{
			switch(evt.Reason)
			{
				case ScreenExitEvent.StartGame:
					// game info should be stored somewhere central? No... might be more than 1 in progress - how then? a GameInfo class...? Possibly.
					mGame.mGameInfo = mUI.mGameInfo;
					SetScreen(mGame);
					break

				default:
					break; //huh?
			}
		}

		//////////////////////////////////////////////////////////////////////
		
		private function OnFrame(evt:Event):void
		{
			Globals.TimeDelta = 1.0 / 60;
			Globals.Time +=  Globals.TimeDelta;

			if(mCurrentScreen != null)
			{
				mCurrentScreen.Update();
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function LoaderComplete(evt:Event):void
		{
			removeChild(mLoading);
			mLoading = null;
			
			Dictionary.Load();
			Letter.Load();
			mGame.Load();
			mUI.Load();
			mResultsScreen.Load();
			Globals.Time = 0;
			SetScreen(mUI);
			addEventListener(Event.ENTER_FRAME, OnFrame); 
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function Preload():void
		{
			Globals.Loader = new BulkLoader("Main");
			Globals.Loader.addEventListener(ProgressEvent.PROGRESS, LoaderProgress);
			Globals.Loader.addEventListener(Event.COMPLETE, LoaderComplete);
			Dictionary.Preload();
			Letter.Preload();
			mResultsScreen.Preload();
			mGame.Preload();
			mUI.Preload();
			Globals.Loader.start();
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function MouseLeft(evt:Event):void
		{
			Globals.MouseHeld = false;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function MakeTheWords()
		{
			Globals.LocalStore = SharedObject.getLocal("settings");
			
			stage.addEventListener(Event.MOUSE_LEAVE, MouseLeft);

			Globals.GameModeIndex = 0;	// remember this locally, not in the database
			
			if("GameModeIndex" in Globals.LocalStore.data)
			{
				Globals.GameModeIndex = Globals.LocalStore.data.GameModeIndex;
			}

			Globals.GameMode = GameMode.Modes[Globals.GameModeIndex];
			Globals.BuildTimeStamp = BuildTimeStamp.readCompilationDate(this.loaderInfo);
			
			var paramObj:Object = LoaderInfo(root.loaderInfo).parameters;
			Globals.FacebookName = paramObj["FBName"];
			Globals.FacebookID = paramObj["FBID"];
			Globals.Protocol = paramObj["protocol"];
			
			if(Globals.Protocol == "file:")
			{
				Globals.Protocol = "http:";
			}
			
			// if DEBUG
			if(Globals.FacebookName == "undefined")
			{
				Globals.FacebookName = "Charlie Skilbeck";
				Globals.FacebookID = "839575206";
			}
			
			Globals.StageWidth = stage.stageWidth;
			Globals.StageHeight = stage.stageHeight;

			Letters.Open();
			Letters.SetRandomSeed(1);
			
			mResultsScreen = new ResultsScreen();
			mResultsScreen.addEventListener(Event.CLOSE, OnResultsExit);
			
			mGame = new Game();
			mGame.addEventListener(Event.CLOSE, OnGameExit);

			mUI = new UI();
			mUI.addEventListener(Event.CLOSE, OnUIExit);
			
			mLoading = new TextField();
			mLoading.width = stage.stageWidth;
			mLoading.height = stage.stageWidth;
			mLoading.text = "Loading...\n"
			mLoading.textColor = 0xffffff;
			mLoading.x = 0;
			mLoading.y = 0;
			addChild(mLoading);
			
			mCurrentScreen = null;
			
			Preload();
		}
	}
}