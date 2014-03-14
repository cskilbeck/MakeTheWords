package code
{
	//////////////////////////////////////////////////////////////////////
	// Undo layers bug
	// ?Redo
	// Score/UI

	import flash.display.BitmapData;
	import flash.display.SimpleButton;
	import flash.display.Sprite;
	import flash.events.Event;
	import flash.events.MouseEvent;
	import flash.geom.Matrix;
	import flash.geom.Point;
	import flash.text.AntiAliasType;
	import flash.text.GridFitType;
	import flash.text.StyleSheet;
	import flash.text.TextField;
	import flash.text.TextFieldAutoSize;
	import flash.text.TextFormat;
	
	//////////////////////////////////////////////////////////////////////

	public class Game extends Screen
	{
		//////////////////////////////////////////////////////////////////////

		private const kUndoDepth:int = 100;

		//////////////////////////////////////////////////////////////////////

		private var mFrontdrop:Sprite;
		private var mScoreLabel:TextField;
		private var mUndo:SimpleButton;
		private var mQuit:SimpleButton;
		private var mBoard:Board;
		private var mTimer:Sprite;
		private var mNumber:Sprite;
		private var mLastNumber:int;
		private var mTimeCountdown:Number;

		private var mState:int;
		private var mOldState:int;
		private var mStateFrames:int;
		private var mFrames:int;
		private var mStateTime:Number;

		private var mScore:int;
		
		private var mPick:Point2D;
		private var mTile:Point2D;
		private var mActiveTile:Tile;
		private var mFirstClickedTile:Tile;
		private var mTilePos:Point2D;
		private var mDragDirection:int;
		
		private var mDefinition:Sprite;
		
		private var mUndoStack:Array;
		private var mUndoPointer:int;
		private var mUndoLength:int;
		
		private var mPoster:WebServiceRequest;
		private var mTimeRemaining:Number;
		public var mGameInfo:GameInfo;
		
		private var mWords:Array;	// all the words made so far
		private var mWordsText:TextField;
		
		private var mLeaderboardSize:uint;
		private var mLeaderboardPosition:uint;
		
		private static const kScoreBarTop:uint = 145;
		private static const kScoreBarHeight:uint = 180;
		
		private static var sBigDigitsFonts:code.Font = new code.Font("bigdigits");
		
		//////////////////////////////////////////////////////////////////////
		
		public function Game()
		{
			mGameInfo = new GameInfo();
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Preload():void
		{
			Globals.AddLoadItem("icon_close.png");
			Globals.AddLoadItem("undo.png");
			Globals.AddLoadItem("allColour.png");
			sBigDigitsFonts.Preload();
		}

		//////////////////////////////////////////////////////////////////////
		
		public override function Load():void
		{
			sBigDigitsFonts.Load();

			mPoster = new WebServiceRequest();
			mPoster.addEventListener(Event.COMPLETE, GamePosted);
			
			mBoard = new Board();
			addChild(mBoard);

			mTimer = new Sprite();
			mTimer.x = 560 + 30;
			mTimer.y = 20;
			addChild(mTimer);
			
			var bar:Sprite = new Sprite();
			bar.graphics.beginFill(0xffffff);
			bar.graphics.drawRoundRect(0, 0, 5, kScoreBarHeight, 5, 5);
			bar.graphics.endFill();
			bar.x = 720 - 6;
			bar.y = kScoreBarTop;
			addChild(bar);
			
			mScoreLabel = new TextField();
			mScoreLabel.x = Globals.StageWidth - 55;
			mScoreLabel.y = 205;
			mScoreLabel.width = 100;
			mScoreLabel.height = 26;
			mScoreLabel.text = "HELLO";
			mScoreLabel.defaultTextFormat = new TextFormat("Arial", 24, 0xffffff, true, true);
			mScoreLabel.antiAliasType = AntiAliasType.ADVANCED;
			mScoreLabel.mouseEnabled = false;
			mScoreLabel.autoSize = TextFieldAutoSize.NONE;
			addChild(mScoreLabel);
			
			mWordsText = new TextField();
			mWordsText.width = Globals.StageWidth;
			mWordsText.height = 200;
			mWordsText.x = 0;
			mWordsText.y = Globals.StageHeight - 200;
			mWordsText.background = true;
			mWordsText.backgroundColor = 0x103010;
			mWordsText.antiAliasType = AntiAliasType.ADVANCED;
			mWordsText.defaultTextFormat = new TextFormat("Arial", 14, 0xffffff);
			mWordsText.multiline = true;
			mWordsText.wordWrap = true;
			mWordsText.gridFitType = GridFitType.SUBPIXEL;
			
			var style:StyleSheet = new StyleSheet();
			
			var ws:Object = new Object();
			ws.fontFamily = "Arial";
			ws.fontSize = "18";
			ws.color = "#FFFF00";
			ws.fontWeight = "bold";
			style.setStyle("h1", ws);
			
			var ds:Object = new Object();
			ds.fontFamily = "Arial";
			ds.fontSize = "16";
			ds.color = "#FFFFFF";
			ds.fontWeight = "normal";
			ds.fontStyle = "italic";
			style.setStyle("h2", ds);
			
			mWordsText.styleSheet = style;
			addChild(mWordsText);

			mFrontdrop = new Sprite();
			mFrontdrop.graphics.beginFill(0, 0);
			mFrontdrop.graphics.drawRect(0, 0, Globals.StageWidth, Globals.StageHeight);
			mFrontdrop.graphics.endFill();
			addChild(mFrontdrop);
			
			mUndo = new SimpleButton();
			UIButton.CreateGraphicalButton(mUndo, Globals.Loader.getBitmap(Globals.URL("undo.png")).bitmapData);
			mUndo.x = 720 - (58 * 2);
			mUndo.y = 400 - 48;
			mUndo.addEventListener(MouseEvent.CLICK, OnUndo);
			addChild(mUndo);
			
			mQuit = new SimpleButton();
			UIButton.CreateGraphicalButton(mQuit, Globals.Loader.getBitmap(Globals.URL("icon_close.png")).bitmapData);
			mQuit.x = 720 - (58 * 1);
			mQuit.y = 400 - 48;
			mQuit.addEventListener(MouseEvent.CLICK, OnQuit);
			addChild(mQuit);
			
			mNumber = new Sprite();
			
			AddMiceEvents();
			
			Tile.SetTileSheet(Globals.Loader.getBitmap(Globals.URL("allColour.png")).bitmapData);
			
			mGameInfo.mGameRandomSeed = 0;
			mGameInfo.mBoardID = 0;	// TODO: allow resumption
			Init();
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Init(/*seed:uint, gameID:uint*/):void
		{
			mWords = [];
			if(contains(mNumber))
			{
				removeChild(mNumber);
			}
			mTimeRemaining = mGameInfo.mGameLength;
			InitUndo();
			mDefinition = null;
			ActiveTile = null;
			mFirstClickedTile = null;
			mTile = new Point2D();
			mTilePos = new Point2D();
			mPick = new Point2D();
			mState = GameState.Idle;
			mOldState = GameState.Idle;
			mFrames = 0;
			mStateFrames = 0;
			mStateTime = 0;
			mLastNumber = -1;
			mTimeCountdown = 0;
			mBoard.Init(mGameInfo.mGameRandomSeed, mGameInfo.mBoard);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function MarkAllWords():void
		{
			var score:int = mBoard.MarkAllWords();
			var l:uint = mBoard.mValidWords.length;
			for(var i:int = 0; i<l; ++i)
			{
				var word:Word = mBoard.mValidWords[i];
				var wordIndex:int = word.mWordIndex;
				if(mWords.indexOf(wordIndex) == -1)
				{
					var w:String = Dictionary.sWords[wordIndex];
					mWords.push(wordIndex);
					mWordsText.htmlText += "<h1>" + w + " (" + Letters.ScoreWord(w) + ")</h1><p><h2>" + Dictionary.GetDefinition(wordIndex).split("@").join("") + "</h2></p>";
					mWordsText.scrollV = mWordsText.maxScrollV;
				}
			}
			if(Globals.GameMode == GameMode.Drop)
			{
				mScore += score;
				if(score > 0)
				{
					mBoard.DropWordTiles();
					mState = GameState.FadeTiles;
				}
			}
			else
			{
				mScore = score;
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function OnQuit(evt:Event):void
		{
			Close(ScreenExitEvent.Quit);
		}

		//////////////////////////////////////////////////////////////////////
		
		private function MouseButtonDown(evt:Event):void
		{
			Globals.MouseHeld = true;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function MouseButtonLeave(evt:Event):void
		{
			Globals.MouseHeld = false;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function MouseButtonUp(evt:MouseEvent):void
		{
			Globals.MouseHeld = false;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function AddMiceEvents():void
		{
			mFrontdrop.addEventListener(MouseEvent.MOUSE_DOWN, MouseButtonDown);
			mFrontdrop.addEventListener(MouseEvent.MOUSE_UP, MouseButtonUp);
			addEventListener(Event.MOUSE_LEAVE, MouseButtonUp);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function OnUndo(evt:Event):void
		{
			PopUndo();
		}
		
		//////////////////////////////////////////////////////////////////////

		public function set ActiveTile(t:Tile):void
		{
			if (mActiveTile != null)
			{
				mActiveTile.Active = false;
			}
			mActiveTile = t;
			if (mActiveTile != null)
			{
				mActiveTile.Active = true;
			}
		}

		//////////////////////////////////////////////////////////////////////

		public function get ActiveTile():Tile
		{
			return mActiveTile;
		}

		//////////////////////////////////////////////////////////////////////
		
		private function InitUndo():void
		{
			var size:int = Board.kBoardSize * kUndoDepth;
			mUndoStack = new Array(size);	// ints
			for(var i:int=0; i<size; ++i)
			{
				mUndoStack[i] = int(0);
			}
			ResetUndo();
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function ResetUndo():void
		{
			mUndoPointer = 0;
			mUndoLength = 0;
		}

		//////////////////////////////////////////////////////////////////////
		
		private function PushUndo():void
		{
			for(var i:int=0; i<Board.kBoardSize; ++i)
			{
				if(mBoard.GetTileAt(i).mIndex != i)
				{
					var base:int = Board.kBoardSize * mUndoPointer;
					for(var t:int=0; t<Board.kBoardSize; ++t)
					{
						mUndoStack[base + t] = mBoard.GetTileAt(t).mIndex;
					}
					mUndoPointer = (mUndoPointer + 1) % kUndoDepth;
					mUndoLength += 1;
					if(mUndoLength > kUndoDepth)
					{
						mUndoLength = kUndoDepth;
					}
					break;
				}
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function PopUndo():void
		{
			if(mUndoLength > 0)
			{
				--mUndoLength;
				--mUndoPointer;
				
				if(mUndoPointer < 0)
				{
					mUndoPointer = kUndoDepth - 1;
				}
				
				if(mFirstClickedTile != null)
				{
					mFirstClickedTile.mLayer = 0;
					mFirstClickedTile.mState = TileState.Lerp;
				}
				
				mActiveTile = null;
				
				var keep:Array = new Array();
				var tile:Tile;
				var temp:TempTile;

				for(var i:int=0; i<Board.kBoardSize; ++i)
				{
					tile = mBoard.GetTileAt(i);
					temp = new TempTile();
					temp.mLetter = tile.mLetter;
					keep.push(temp);
				}
				
				for(i=0; i<Board.kBoardSize; ++i)
				{
					tile = mBoard.GetTileAt(i);
					temp = keep[i] as TempTile;

					temp.mIndex = i;
					tile.mIndex = mUndoStack[mUndoPointer * Board.kBoardSize + i];
					temp.mOrigin = new Point((i % Board.kWidth) * Tile.kWidth, int(i / Board.kWidth) * Tile.kHeight);
					tile.mOrigin.x = temp.mOrigin.x;
					tile.mOrigin.y = temp.mOrigin.y;
					tile.ResetPosition();
				}

				for(i=0; i<Board.kBoardSize; ++i)
				{
					tile = mBoard.GetTileAt(i);
					temp = keep[i] as TempTile;

					var src:int = temp.mIndex;
					var dst:int = tile.mIndex;

					if(src != dst)
					{
						var target:Tile = mBoard.GetTileAt(dst);
						var source:TempTile = keep[src] as TempTile;
						target.Position = source.mOrigin;
						target.Target = target.Origin;
						target.mLetter = source.mLetter;
						target.ResetWords();
					}
				}
				
				mBoard.ResetTileIndices();
				mState = GameState.Idle;
				mBoard.mSuspendMarking = false;
				MarkAllWords();
			}
		}

		//////////////////////////////////////////////////////////////////////
		
		private function GamePosted(evt:Event):void
		{
			var o:Object = (evt.target as WebServiceRequest).mResult;
			if("error" in o)
			{
				if(o["error"] == 5)
				{
					Close(ScreenExitEvent.OutOfTime);
				}
			}
			else
			{
				mLeaderboardPosition = o["leaderboard_position"];
				mLeaderboardSize = o["leaderboard_size"];
				mGameInfo.mBoardID = o["board_id"];
				if("final" in o)
				{
					Close(ScreenExitEvent.OutOfTime);
				}
				else
				{
					var end:Date = o["end_time"];
					var now:Date = o["current_time"];
					mTimeRemaining = (end.time - now.time) / 1000;
				}
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function DrawTimer():void
		{
			if(mTimeRemaining <= 5 && mTimeCountdown == 0)
			{
				mTimeCountdown = mTimeRemaining;
			}
			
			// show a big digit
			if(mTimeCountdown > 0)
			{
				mTimeCountdown -= Globals.TimeDelta;
				var mt:Number = mTimeCountdown % 5;
				
				var seconds:int = int(mt);
				var fraction:Number = mt - seconds;
				
				if(mLastNumber != seconds)
				{
					mLastNumber = seconds;
					var bmd:BitmapData = sBigDigitsFonts.GetBitmapData(seconds);
					mNumber.graphics.clear();
					mNumber.graphics.beginBitmapFill(bmd);
					mNumber.graphics.drawRect(0, 0, bmd.width, bmd.height);
					mNumber.graphics.endFill();
				}
				var mat:Matrix = new Matrix();
				var s:Number = 2 - (2 * (fraction * fraction * fraction));
				var w:Number = mNumber.width;
				var h:Number = mNumber.height;
				mat.scale(s, s);
				mat.translate(280 - w / 2, 200 - h / 2);
				mNumber.transform.matrix = mat;
				mNumber.alpha = fraction;
				mNumber.mouseEnabled = false;
				addChild(mNumber);
			}
			
			var percent:Number = mTimeRemaining * 360 / mGameInfo.mGameLength;
			var c:uint = 0x605040;
			if(mTimeRemaining < 20)
			{
				var m:Number = 5;
				if(mTimeRemaining < 10)
				{
					m = 15;
				}
				if(((Globals.Time * m) % 2) > 1)
				{
					c = 0xff00ffff;
				}
			}
			mTimer.graphics.clear();
			mTimer.graphics.beginFill(0xffffff);
			mTimer.graphics.drawCircle(50, 50, 50);
			mTimer.graphics.endFill();
			mTimer.graphics.beginFill(0);
			mTimer.graphics.drawCircle(50, 50, 45);
			mTimer.graphics.endFill();
			mTimer.graphics.beginFill(c);
			Util.drawWedge(mTimer.graphics, 50, 50, 45.1, percent, 90);
			mTimer.graphics.endFill();
			mTimer.graphics.beginFill(0xffffff);
			mTimer.graphics.drawCircle(50, 50, 25);
			mTimer.graphics.endFill();
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public override function Update():void
		{
			Globals.MouseX = mouseX;
			Globals.MouseY = mouseY;
			Globals.MousePosition = new Point(mouseX,mouseY);
			var mouseDelta:Boolean = Globals.MouseHeld != Globals.OldMouseHeld;
			Globals.MousePressed = mouseDelta && Globals.MouseHeld;
			Globals.MouseReleased = mouseDelta && ! Globals.MouseHeld;
			Globals.OldMouseHeld = Globals.MouseHeld;

			if (mBoard.Update())
			{
				MarkAllWords();
			}

			++mFrames;
			
			if(!mPoster.Busy() && mPoster.IdleTime() > 1)
			{
				if(mGameInfo.mGameID != 0)
				{
					mPoster.Execute("post", { "facebook_id": Globals.FacebookID, "board": mBoard.GetLetters(), "game_id": mGameInfo.mGameID, "board_id": mGameInfo.mBoardID });
				}
			}
			
			mTimeRemaining -= Globals.TimeDelta;

			if(mTimeRemaining <= 0)
			{
				Close(ScreenExitEvent.OutOfTime);
			}
			else
			{
				DrawTimer();
				
				if (mState != mOldState)
				{
					mStateTime = 0;
					mStateFrames = 0;
					mOldState = mState;
				}
				else
				{
					++mStateFrames;
					mStateTime +=  Globals.TimeDelta;
				}
	
				switch (mState)
				{
					case GameState.Idle :
						Idle();
						break;
	
					case GameState.PickDirection :
						PickDirection();
						break;
	
					case GameState.DragTile :
						DragTile();
						break;
	
					case GameState.SwapTiles :
						SwapTiles();
						break;
						
					case GameState.ShowDefinition:
						ShowDefinition();
						break;
					
					case GameState.WaveTileAround:
						WaveTileAround();
						break;
					
					case GameState.FadeTiles:
						FadeTiles();
						break;
					
					case GameState.DropTiles:
						DropTiles();
						break;
				}
			}
			mScoreLabel.text = mScore.toString();
			var leaderboardHeight:int = kScoreBarHeight - 10;
			var scorePos:int = 0;
			if(mLeaderboardSize > 0)
			{
				scorePos = mLeaderboardPosition * leaderboardHeight / mLeaderboardSize;
			}
			mScoreLabel.x = 720 - 20 - mScoreLabel.textWidth;
			mScoreLabel.y = scorePos + kScoreBarTop - mScoreLabel.textHeight / 2; 
		}

		//////////////////////////////////////////////////////////////////////

		private function Idle():void
		{
			if (mBoard.mAtRest)
			{
				if (Globals.MousePressed)
				{
					var t:Tile = mBoard.GetTileAtScreenPosition(Globals.MousePosition);
					if (t != null)
					{
						mPick.set(Globals.MouseX, Globals.MouseY);
						mBoard.ResetTileIndices();
						ActiveTile = t;
						ActiveTile.mLayer = 1;
						mTile.setFromPoint(ActiveTile.Origin);
						mTilePos.set(Globals.MouseX / Tile.kWidth, Globals.MouseY / Tile.kHeight);
						mState = GameState.PickDirection;
						mDragDirection = DragDirection.None;
					}
				}
			}
		}

		//////////////////////////////////////////////////////////////////////

		private function GetDirection():int
		{
			var d:Point2D = Point2D.FromPoint(Globals.MousePosition).subtract(mPick);
			d.x *=  2;
			d.y *=  2;

			var i:int = (d.x < -Tile.kWidth)?DragDirection.Left :
			(d.x >  Tile.kWidth)?DragDirection.Right :
			(d.y < -Tile.kHeight)?DragDirection.Up :
			(d.y >  Tile.kHeight)?DragDirection.Down :
			DragDirection.None;
			return i;
		}

		//////////////////////////////////////////////////////////////////////

		private function PickDirection():void
		{
			if (Globals.MouseReleased)
			{
				var t:Tile = mBoard.GetTileAtScreenPosition(Globals.MousePosition);

				if (t == ActiveTile)
				{
					if (mFirstClickedTile == null)
					{
						mFirstClickedTile = ActiveTile;
						if (ActiveTile != null)
						{
							ActiveTile.mState = TileState.Selected;
							ActiveTile.mHorizontalWordPosition = WordPosition.None;
							ActiveTile.mVerticalWordPosition = WordPosition.None;
							ActiveTile.UpdateTileBackground();
							for (var i:int=0; i<Board.kBoardSize; ++i)
							{
								(mBoard.mBoard[i] as Tile).mLayer = 0;
							}
							ActiveTile.mLayer = 1;
							mFirstClickedTile.mLayer = 2;
						}
					}
					else if (mFirstClickedTile !== ActiveTile)
					{
						mFirstClickedTile.Target = ActiveTile.Origin;
						ActiveTile.Target = mFirstClickedTile.Origin;

						mFirstClickedTile.mState = TileState.Lerp;
						ActiveTile.mState = TileState.Lerp;
						mFirstClickedTile.mLayer = 1;
						ActiveTile.mLayer = 2;
						ActiveTile.ResetWords();
						mState = GameState.SwapTiles;
						return;
					}
					else
					{
						mFirstClickedTile.mState = TileState.Lerp;
						mFirstClickedTile.mLayer = 0;
						mFirstClickedTile = null;
						mState = GameState.Idle;
						MarkAllWords();
					}
					ActiveTile = null;
					mState = GameState.Idle;
				}
				return;
			}
			
			if (mStateTime > 0.5)
			{
				if(mActiveTile.mHorizontalWord != null || mActiveTile.mVerticalWord != null)
				{
					SetupDefinition();
					addChild(mDefinition);
					mState = GameState.ShowDefinition;
					return;
				}
			}
			
			switch(Globals.GameMode)
			{
				case GameMode.Advanced:
					mDragDirection = GetDirection();
					if (mDragDirection != DragDirection.None)
					{
						if (mFirstClickedTile != null)
						{
							mFirstClickedTile.mLayer = 0;
							mFirstClickedTile = null;
						}
						mBoard.SetAllTileStates(TileState.Lerp);
						//assert(ActiveTile != null);
						ActiveTile.mLayer = 1;
						mState = GameState.DragTile;
						mBoard.mSuspendMarking = true;
					}
					break;
				
				case GameMode.Normal:
				case GameMode.Drop:
					var d:Point2D = Point2D.FromPoint(Globals.MousePosition).subtract(mPick);
					if(Math.abs(d.x) > 4 || Math.abs(d.y) > 4)
					{
						if (mFirstClickedTile != null)
						{
							mFirstClickedTile.mLayer = 0;
							mFirstClickedTile = null;
						}
						ActiveTile.mHorizontalWordPosition = WordPosition.None;
						ActiveTile.mVerticalWordPosition = WordPosition.None;
						mBoard.SetAllTileStates(TileState.Lerp);
						ActiveTile.mLayer = 1;
						mState = GameState.WaveTileAround;
					}
					break;
			}
		}

		//////////////////////////////////////////////////////////////////////
		
		private function Constrain(p:Point2D, min:Point2D, max:Point2D):Point2D
		{
			var r:Point2D = p.clone();
			if (r.x < min.x)
			{
				r.x = min.x;
			}
			if (r.x > max.x)
			{
				r.x = max.x;
			}
			if (r.y < min.y)
			{
				r.y = min.y;
			}
			if (r.y > max.y)
			{
				r.y = max.y;
			}
			return r;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function Min(p:Point2D, m:Point2D):Point2D
		{
			return new Point2D(p.x < m.x ? p.x : m.x, p.y < m.y ? p.y : m.y);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function Max(p:Point2D, m:Point2D):Point2D
		{
			return new Point2D(p.x > m.x ? p.x : m.x, p.y > m.y ? p.y : m.y);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function WaveTileAround():void
		{
			var bw:int = (Board.kWidth - 1) * Tile.kWidth;
			var bh:int = (Board.kHeight - 1) * Tile.kHeight;
			var bs:Point2D = new Point2D(bw,bh);
			var d:Point2D = mPick.subtract(mTile);
			d.x = Globals.MouseX - d.x;
			d.y = Globals.MouseY - d.y;
			d = Constrain(d, new Point2D(0, 0), bs);
			ActiveTile.Target = new Point(d.x, d.y);
			ActiveTile.Position = new Point(d.x, d.y);
			
			if(!Globals.MouseHeld)
			{
				// swap with the tile underneath the centre of the tile being waved about
				var c:Point = ActiveTile.Position.add(new Point(Tile.kWidth / 2, Tile.kHeight / 2));
				c.x /= Tile.kWidth;
				c.y /= Tile.kHeight;

				mFirstClickedTile = mBoard.GetTile(int(c.x), int(c.y));
				
				mFirstClickedTile.Target = ActiveTile.Origin;
				ActiveTile.Target = mFirstClickedTile.Origin;
				
				mFirstClickedTile.mState = TileState.Lerp;
				ActiveTile.mState = TileState.Lerp;
				mFirstClickedTile.mLayer = 1;
				ActiveTile.mLayer = 2;
				ActiveTile.ResetWords();
				mState = GameState.SwapTiles;
				return;
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function FadeTiles():void
		{
			if(mBoard.mAtRest)
			{
				for(var i:int=0; i<Board.kBoardSize; ++i)
				{
					(mBoard.mBoard[i] as Tile).mSpecial = false;
				}

				// faded out, drop in replacements
				for(var x:int=0; x<Board.kWidth; ++x)
				{
					var drop:Number = 0;

					for(var y:int = Board.kHeight-1; y>=0; --y)
					{
						var t:Tile = mBoard.GetTile(x, y);
						
						if(t.mEmpty)
						{
							for(var y2:int=y-1; y2 >= 0; --y2)
							{
								var t2:Tile = mBoard.GetTile(x, y2);

								if(!t2.mEmpty)
								{
									// copy t2 into t
									t.mPosition.y = t2.Position.y;
									t.Target = t.Origin;
									t.ResetWords();
									t.mDropVelocity = 400;
									t.mState = TileState.Drop;
									t.mEmpty = false;
									if(t.contains(t.mLetter.mLetterBitmap))
									{
										t.removeChild(t.mLetter.mLetterBitmap);
									}
									Tile.SwapLetters(t, t2);
									t2.mEmpty = true;
									break;
								}
							}
							if(t.mEmpty)
							{
								// didn't find one above, so invent
								drop -= Tile.kHeight;
								if(t.contains(t.mLetter.mLetterBitmap))
								{
									t.removeChild(t.mLetter.mLetterBitmap);
								}
								t.mLetter = new Letter(Letters.GetRandomChar(), Tile.kWidth, Tile.kHeight);
								t.mSpecial = true;
								t.mPosition.y = drop;
								t.Target = t.Origin;
								t.ResetWords();
								t.mDropVelocity = 400;
								t.mState = TileState.Drop;
								t.mEmpty = false;
							}
						}
						t.mAlpha = 1;
					}
				}
				var replaced:Boolean = true;
				while(replaced)
				{
					mBoard.MarkAllWords();
					replaced = false;
					for(i=0; i<Board.kBoardSize; ++i)
					{
						var tile:Tile = mBoard.mBoard[i] as Tile;
						if(tile.mSpecial)
						{
							if(tile.mHorizontalWord != null || tile.mVerticalWord != null)
							{
								tile.mLetter = new Letter(Letters.GetRandomChar(), Tile.kWidth, Tile.kHeight);
								replaced = true;
							}
						}
					}
				}
				for(i=0; i<Board.kBoardSize; ++i)
				{
					tile = mBoard.mBoard[i] as Tile;
					tile.ResetWords();
				}
				mState = GameState.DropTiles;
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function DropTiles():void
		{
			if(mBoard.mAtRest)
			{
				mState = GameState.Idle;
				MarkAllWords();
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function AddDefinition(word:uint):String
		{
			var text:String = "";
			var w:String = Dictionary.sWords[word]; 
			text += "<h1>";
			text += w;
			text += " (" + Letters.ScoreWord(w).toString(10) + " points)";
			text += "</h1>";
			text += "<p>&nbsp;</p>";
			text += "<h2>";
			text += Dictionary.GetDefinition(word).split("@").join("");	// for now I guess
			text += "</h2>";
			return text;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function SetupDefinition():void
		{
			mDefinition = new Sprite();

			// dim the board
			var backPane:Sprite = new Sprite();
			backPane.graphics.beginFill(0);
			backPane.graphics.drawRect(0, 00, Globals.StageWidth, Globals.StageHeight);
			backPane.graphics.endFill();
			backPane.alpha = 0.9;
			mDefinition.addChild(backPane);
			
			// add the definition text
			var s:String = "<body>"
			s += "<p>&nbsp;</p>";

			var t:TextField = new TextField();
			t.x = 10;
			t.y = 10;
			t.width = 560 - 20;
			t.height = Globals.StageHeight - 20;
			var horizIndex:uint = 0; 
			if(mActiveTile.mHorizontalWord)
			{
				horizIndex = mActiveTile.mHorizontalWord.mWordIndex;
				s += AddDefinition(horizIndex);
			}
			s += "<p>&nbsp;</p>";
			if(mActiveTile.mVerticalWord != null && mActiveTile.mVerticalWord.mWordIndex != horizIndex)
			{
				s += "<p>&nbsp;</p>";
				s += AddDefinition(mActiveTile.mVerticalWord.mWordIndex);
			}
			
			s += "</body>";

			var style:StyleSheet = new StyleSheet();

			var ws:Object = new Object();
			ws.fontFamily = "Arial";
			ws.fontSize = "26";
			ws.color = "#FFFF00";
			ws.fontWeight = "bold";
			style.setStyle("h1", ws);

			var ds:Object = new Object();
			ds.fontFamily = "Arial";
			ds.fontSize = "22";
			ds.color = "#FFFFFF";
			ds.fontWeight = "normal";
			ds.fontStyle = "italic";
			style.setStyle("h2", ds);
			
			t.antiAliasType = AntiAliasType.ADVANCED;
			t.wordWrap = true;
			t.styleSheet = style;
			t.htmlText = s;
			
			mDefinition.addChild(t);

			// capture mouse on top of the definition text
			var frontPane:Sprite = new Sprite();
			frontPane.graphics.beginFill(0xffffff);
			frontPane.graphics.drawRect(0, 0, Globals.StageWidth, Globals.StageHeight);
			frontPane.graphics.endFill();
			frontPane.alpha = 0;
			mDefinition.addChild(frontPane);

			// close button
			var closeButton:SimpleButton = new SimpleButton();
			UIButton.CreateGraphicalButton(closeButton, Globals.Loader.getBitmap(Globals.URL("icon_close.png")).bitmapData);
			closeButton.x = 600;
			closeButton.y = 10;
			closeButton.addEventListener(MouseEvent.CLICK, DismissDefinition);
			mDefinition.addChild(closeButton);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function IsHorizontal(d:int):Boolean
		{
			return d == DragDirection.Left || d == DragDirection.Right;
		}

		//////////////////////////////////////////////////////////////////////

		private function IsVertical(d:int):Boolean
		{
			return d == DragDirection.Up || d == DragDirection.Down;
		}

		//////////////////////////////////////////////////////////////////////

		private function Sgn(p:Point2D):Point2D
		{
			return new Point2D((p.x < 0) ? -1 : ((p.x > 0) ? 1 : 0), (p.y < 0) ? -1 : ((p.y > 0) ? 1 : 0));
		}

		//////////////////////////////////////////////////////////////////////

		private function ShuntTiles(dragDirection:int, tilePos:Point2D, whichTile:Point2D, direction:Point2D):void
		{
			var letter:Letter = ActiveTile.TheLetter;
			var li:int = ActiveTile.mIndex;

			var t1:Tile;
			var t2:Tile;

			if (IsHorizontal(dragDirection))
			{
				for (var x:int = tilePos.x; x != whichTile.x; x -= direction.x)
				{
					t1 = mBoard.GetTile(x,whichTile.y);
					t2 = mBoard.GetTile(x - direction.x, whichTile.y);
					t1.TheLetter = t2.TheLetter;
					t1.mIndex = t2.mIndex;
					t1.ResetPosition();
				}
			}
			else
			{
				for (var y:int = tilePos.y; y != whichTile.y; y -= direction.y)
				{
					t1 = mBoard.GetTile(whichTile.x,y);
					t2 = mBoard.GetTile(whichTile.x,y - direction.y);
					t1.TheLetter = t2.TheLetter;
					t1.mIndex = t2.mIndex;
					t1.ResetPosition();
				}
			}

			var td:Tile = mBoard.GetTile(whichTile.x,whichTile.y);
			td.TheLetter = letter;
			td.mIndex = li;
			td.ResetPosition();

			ActiveTile.ResetPosition();
			ActiveTile.mLayer = 0;
			ActiveTile = null;
		}

		//////////////////////////////////////////////////////////////////////

		private function DragTile():void
		{
			var bw:int = (Board.kWidth - 1) * Tile.kWidth;
			var bh:int = (Board.kHeight - 1) * Tile.kHeight;
			var bs:Point2D = new Point2D(bw,bh);

			var inputMouse:Point2D = new Point2D(Globals.MouseX,Globals.MouseY);
			var d:Point2D = mPick.subtract(mTile);
			var p:Point2D = Constrain(inputMouse.subtract(d),new Point2D(0,0),bs);
			var dir:Point2D = Point2D.FromPoint(ActiveTile.Position).subtract(p);

			if (IsHorizontal(mDragDirection))
			{
				ActiveTile.mPosition.x = p.x;
			}
			else
			{
				ActiveTile.mPosition.y = p.y;
			}
			ActiveTile.Target = ActiveTile.Position;

			var mid:Point2D = Point2D.FromPoint(ActiveTile.mPosition).add(new Point2D(Tile.kWidth / 2,Tile.kHeight / 2));

			var whichTile:Point2D = Constrain(mid.divideBy(mBoard.mTileSize),new Point2D(0,0),mBoard.mSize.subtract(new Point2D(1,1)));
			var diff:Point2D = mTilePos.subtract(whichTile);

			var direction:Point2D = Sgn(diff);

			var mn:Point2D = Min(whichTile,mTilePos);
			var mx:Point2D = Max(whichTile,mTilePos);

			var newDir:int = DragDirection.None;

			if (IsHorizontal(mDragDirection))
			{
				if (dir.y > (mBoard.mTileSize.y / 2))
				{
					newDir = DragDirection.Down;
				}
				else if (dir.y < -(mBoard.mTileSize.y / 2))
				{
					newDir = DragDirection.Up;
				}

				if (newDir != DragDirection.None)
				{
					mPick.x = inputMouse.x;
					mPick.y = (int)(ActiveTile.mPosition.y + mBoard.mTileSize.y / 2);
				}
			}
			else
			{
				if (dir.x > (mBoard.mTileSize.x / 2))
				{
					newDir = DragDirection.Right;
				}
				else if (dir.x < -(mBoard.mTileSize.x / 2))
				{
					newDir = DragDirection.Left;
				}

				if (newDir != DragDirection.None)
				{
					mPick.x = (int)(ActiveTile.mPosition.x + mBoard.mTileSize.x / 2);
					mPick.y = inputMouse.y;
				}
			}

			if (newDir != DragDirection.None)
			{
				ShuntTiles(mDragDirection, mTilePos, whichTile, direction);
				mDragDirection = newDir;
				mTilePos = Constrain(mPick.divideBy(mBoard.mTileSize),new Point2D(0,0),mBoard.mSize.subtract(new Point2D(1,1)));
				ActiveTile = mBoard.GetTile(mTilePos.x,mTilePos.y);
				ActiveTile.ResetPosition();
				ActiveTile.mHorizontalWordPosition = WordPosition.None;
				ActiveTile.mVerticalWordPosition = WordPosition.None;
				mTile = Point2D.FromPoint(ActiveTile.mOrigin);
				ActiveTile.mLayer = 1;
			}

			if (Globals.MouseReleased)
			{
				ShuntTiles(mDragDirection, mTilePos, whichTile, direction);
				mBoard.ResetTilePositions();
				mBoard.mSuspendMarking = false;
				PushUndo();
				MarkAllWords();
				mState = GameState.Idle;
			}
			else
			{
				ActiveTile.mHorizontalWordPosition = WordPosition.None;
				ActiveTile.mVerticalWordPosition = WordPosition.None;

				var t:Tile;
				var x:int;
				var y:int;
				if (IsHorizontal(mDragDirection))
				{
					for (x = 0; x < mn.x; ++x)
					{
						t = mBoard.GetTile(x,mTilePos.y);
						t.Target = t.Origin;
					}
					for (x = mx.x + 1; x < Board.kWidth; ++x)
					{
						t = mBoard.GetTile(x,mTilePos.y);
						t.Target = t.Origin;
					}
					for (x = whichTile.x; x != mTilePos.x; x += direction.x)
					{
						t = mBoard.GetTile(x,mTilePos.y);
						t.mTarget.x = t.mOrigin.x + mBoard.mTileSize.x * direction.x;
					}
				}
				else
				{
					for (y = 0; y < mn.y; ++y)
					{
						t = mBoard.GetTile(mTilePos.x,y);
						t.Target = t.Origin;
					}
					for (y = mx.y + 1; y < Board.kHeight; ++y)
					{
						t = mBoard.GetTile(mTilePos.x,y);
						t.Target = t.Origin;
					}
					for (y = whichTile.y; y != mTilePos.y; y += direction.y)
					{
						t = mBoard.GetTile(mTilePos.x,y);
						t.mTarget.y = t.mOrigin.y + mBoard.mTileSize.y * direction.y;
					}
				}
			}
		}

		//////////////////////////////////////////////////////////////////////

		private function SwapTiles():void
		{
			if (mBoard.mAtRest)
			{
				Tile.SwapLetters(ActiveTile, mFirstClickedTile);
				ActiveTile.ResetPosition();
				mFirstClickedTile.ResetPosition();
				ActiveTile.mState = TileState.Lerp;
				mFirstClickedTile.mState = TileState.Lerp;
				ActiveTile.mLayer = 0;
				mFirstClickedTile.mLayer = 0;
				ActiveTile = null;
				mFirstClickedTile = null;
				PushUndo();
				mState = GameState.Idle;
				MarkAllWords();
			}
		}

		//////////////////////////////////////////////////////////////////////
		
		private function ShowDefinition():void
		{
			if(Globals.MousePressed)
			{
				removeChild(mDefinition);
				mDefinition = null;
				mState = GameState.Idle;
			}
		}

		//////////////////////////////////////////////////////////////////////
		
		private function DismissDefinition(evt:MouseEvent):void
		{
			removeChild(mDefinition);
			mDefinition = null;
			Globals.MouseHeld = false;
			mState = GameState.Idle;
		}	
	}
}

import flash.geom.Point;

import code.Letter;

class TempTile
{
	public var mOrigin:Point;
	public var mLetter:Letter;
	public var mIndex:int;
}

