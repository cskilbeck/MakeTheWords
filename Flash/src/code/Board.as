package code
{
	//////////////////////////////////////////////////////////////////////
	
	import flash.display.Sprite;
	import flash.geom.Point;
	
	//////////////////////////////////////////////////////////////////////
	
	public class Board extends Sprite
	{
		//////////////////////////////////////////////////////////////////////
		
		public static const kWidth:int = 7;
		public static const kHeight:int = 5;
		public static const kBoardSize:int = kWidth * kHeight;
		
		public var mBoard:Array;		// the tiles
		public var mValidWords:Array;	// the words it found
		public var mSize:Point2D;
		public var mTileSize:Point2D;
		public var mAtRest:Boolean;
		public var mSuspendMarking:Boolean;
		
		private var mFoundWords:Array;
		private var mLastBoardPostTime:Number;
		
		//////////////////////////////////////////////////////////////////////
		
		public function GetTileAt(index:int):Tile
		{
			return mBoard[index] as Tile;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function GetTile(x:int, y:int):Tile
		{
			return mBoard[x + y * kWidth] as Tile;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function ResetTilePositions():void
		{
			for(var y:int=0; y<kHeight; ++y)
			{
				for(var x:int=0; x<kWidth; ++x)
				{
					var t:Tile = GetTile(x,y);
					t.mOrigin = new Point(x * Tile.kWidth, y * Tile.kHeight);
					t.Target = t.Origin;
					t.Position = t.Origin;
					t.mAtRest = true;
					t.mLayer = 0;
					t.mOldState = TileState.Invalid;
					t.mState = TileState.Lerp;
				}
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function ResetTileIndices():void
		{
			for(var i:int=0; i<kBoardSize; ++i)
			{
				GetTileAt(i).mIndex = i;
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function GetTileAtScreenPosition(p:Point):Tile
		{
			if (p.x < kWidth * Tile.kWidth && p.y < kHeight * Tile.kHeight)
			{
				return GetTile(p.x / Tile.kWidth, p.y / Tile.kHeight);
			}
			return null;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function MarkWordPass(direction:int, offsetVector:int, limit:int, xMul:int, yMul:int):void
		{
			var checkString:String = new String();
			
			var xLim:int = kWidth - 2 * xMul;
			var yLim:int = kHeight - 2 * yMul;
			
			for(var y:int=0; y < yLim; ++y)
			{
				for(var x:int=0; x < xLim; ++x)
				{
					var n:int = x + y * kWidth;
					var t:int = x * xMul + y * yMul;
					
					for(var e:int=3; e + t <= limit; ++e)
					{
						checkString = "";
						
						var m:int = n;
						for(var i:int=0; i<e; ++i)
						{
							checkString +=  String.fromCharCode(mBoard[m].GetLetter + 32);
							m +=  offsetVector;
						}
						var w:int = Dictionary.WordIndex(checkString);
						if (w != -1)
						{
							mFoundWords.push(new Word(x, y, e, direction, w));
						}
					}
				}
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function layerSorter(a:int, b:int):int
		{
			if (mBoard[a].mLayer == mBoard[b].mLayer)
			{
				return a - b;
			}
			else
			{
				return mBoard[a].mLayer - mBoard[b].mLayer;
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function Update():Boolean
		{
			var index:Array = new Array();
			for(var i:int=0; i<kBoardSize; ++i)
			{
				index[i] = i;
			}
			index.sort(layerSorter);
			
			for(i=0; i<kBoardSize; ++i)
			{
				setChildIndex(mBoard[index[i]], i);
			}
			
			var oar:Boolean = mAtRest;
			mAtRest = true;
			for(i=0; i<kBoardSize; ++i)
			{
				var ar:Boolean = (mBoard[i] as Tile).Update();
				mAtRest = mAtRest && ar;
			}			
			return !mSuspendMarking && (!oar && mAtRest);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function GetWordTile(w:Word, index:int):Tile
		{
			var yo:int = w.mDirection;// YoinK! don't touch that enum...
			var xo:int = 1 - yo;
			return mBoard[(w.mX + xo * index) + (w.mY + yo * index) * kWidth];
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function SetWordTile(w:Word, index:int, t:Tile):void
		{
			var yo:int = w.mDirection;// YoinK! don't touch that enum...
			var xo:int = 1 - yo;
			mBoard[(w.mX + xo * index) + (w.mY + yo * index) * kWidth] = t;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		private function CompareWords(a:Word, b:Word):int
		{
			if(a.mScore != b.mScore)
			{
				return b.mScore - a.mScore;
			}
			else if(a.mLength != b.mLength)
			{
				return b.mLength - a.mLength;
			}
			else
			{
				return b.mWordIndex - a.mWordIndex;
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function InitializeTileLetters(seed:int, board:String = null):void
		{
			Letters.SetRandomSeed(seed);
			
			for(var i:int=0; i<mBoard.length; ++i)
			{
				if(mBoard[i])
				{
					removeChild(mBoard[i]);
					delete mBoard[i];
				}
			}
			
			var c:int=0;
			for(var y:int=0; y<kHeight; ++y)
			{
				for(var x:int=0; x<kWidth; ++x)
				{
					var t:Tile;
					if(board)
					{
						t = new Tile(board.charCodeAt(c++));
					}
					else
					{
						t = new Tile(Letters.GetRandomChar());
					}
					var X:Number = x * Tile.kWidth;
					var Y:Number = y * Tile.kHeight;
					t.SetTile(0, 0);
					var p:Point = new Point(X,Y);
					t.Origin = p;
					t.Position = p;
					t.Target = p;
					t.x = X;
					t.y = Y;
					addChild(t);
					mBoard[x + y * kWidth] = t;
				}
			}
			
			var r:Random = new Random(0);
			
			while(MarkAllWords() > 0)
			{
				for(i = 0; i<mValidWords.length; ++i)
				{
					var w:Word = mValidWords[i] as Word;
					var l:int = r.Next() % w.mLength;
					var o:Tile = GetWordTile(w, l);
					o.mLetter = new Letter(Letters.GetRandomChar(), Tile.kWidth, Tile.kHeight);
				}
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function MarkAllWords():int
		{
			mFoundWords = [];
			
			for(var i:int=0; i<kBoardSize; ++i)
			{
				mBoard[i].ResetWords();
			}
			MarkWordPass(WordDirection.Horizontal, 1, kWidth, 1, 0);
			MarkWordPass(WordDirection.Vertical, kWidth, kHeight, 0, 1);
			
			mFoundWords.sort(CompareWords);
			
			var totalScore:int = 0;
			
			mValidWords = [];
			
			for(var w:int=0; w<mFoundWords.length; ++w)
			{
				var valid:Boolean = true;
				var word:Word = mFoundWords[w];
				
				for(i=0; i<word.mLength; ++i)
				{
					var t:Tile = GetWordTile(word,i);
					
					if (t.mVerticalWord != null && word.mDirection == WordDirection.Vertical)
					{
						valid = false;
						break;
					}
					if (t.mHorizontalWord != null && word.mDirection == WordDirection.Horizontal)
					{
						valid = false;
						break;
					}
				}
				
				if (valid)
				{
					mValidWords.push(word);
					for(i=0; i<word.mLength; ++i)
					{
						var t2:Tile = GetWordTile(word,i);
						t2.SetWord(word, i);
					}
					totalScore +=  word.mScore;
				}
			}
			
			mFoundWords = [];
			
			for(i=0; i<kBoardSize; ++i)
			{
				mBoard[i].UpdateTileBackground();
			}
			
			return totalScore;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function SetAllTileStates(state:int):void
		{
			for(var i:int=0; i<kBoardSize; ++i)
			{
				mBoard[i].mState = state;
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function DropWordTiles():void
		{
			for(var i:int=0; i<kBoardSize; ++i)
			{
				var t:Tile = mBoard[i] as Tile;
				if(t.mHorizontalWord != null || t.mVerticalWord != null)
				{
					t.mState = TileState.FadeOut;
				}
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function MovePiece(index:int, x:int, y:int):void
		{
			(mBoard[index] as Tile).x = x;
			(mBoard[index] as Tile).y = y;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function GetChar(x:int, y:int):String
		{
			return String.fromCharCode(mBoard[x + y * kWidth].GetLetter);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function GetLetters():String
		{
			var s:String = "";
			for(var i:int=0; i<kBoardSize; ++i)
			{
				s += String.fromCharCode(mBoard[i].GetLetter);
			}
			return s;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function Board()
		{
			mTileSize = new Point2D(Tile.kWidth,Tile.kHeight);
			mSize = new Point2D(kWidth,kHeight);
			mValidWords = [];
			mBoard = new Array(kBoardSize);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function Init(seed:uint, board:String):void
		{
			mLastBoardPostTime = Globals.Time;
			InitializeTileLetters(seed, board);
			ResetTileIndices();
			mSuspendMarking = false;
			mAtRest = true;
		}
	}
}