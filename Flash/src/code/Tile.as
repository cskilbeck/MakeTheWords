package code
{
	//////////////////////////////////////////////////////////////////////
	// flash word score when new word created
	// & flash score when it increases/decreases
	
	import flash.display.Bitmap;
	import flash.display.BitmapData;
	import flash.display.Shape;
	import flash.display.Sprite;
	import flash.geom.Matrix;
	import flash.geom.Point;
	import flash.geom.Rectangle;

	//////////////////////////////////////////////////////////////////////
	
	public class Tile extends Sprite
	{
		//////////////////////////////////////////////////////////////////////
		
		public static const kWidth: int = 80;	// source bitmap tile data must conform, no scaling is done
		public static const kHeight: int = 80;

		public var mLetter: Letter;
		public var mAtRest: Boolean;
		public var mEmpty: Boolean;
		public var mState: int;				// see TileState
		public var mOldState: int;
		public var mStateFrames: int;
		public var mIndex: int;
		public var mLayer: int;
		public var mHorizontalWord: Word;
		public var mVerticalWord: Word;
		public var mHorizontalIndex: int;
		public var mVerticalIndex: int;
		public var mHorizontalWordPosition: int;
		public var mVerticalWordPosition: int;
		public var mAlpha:Number;

		public var mSpecial: Boolean;
		
		public var mPosition: Point;
		public var mTarget: Point;
		public var mOrigin: Point;
		public var mDropVelocity: Number;

		public var mScale: Number;
		public var mRotation: Number;

		private var mTileSourceX: int;
		private var mTileSourceY: int;
		private var mTileScore:int;
		private var mScoreBitmap:Bitmap;
		private var mTileBitmap:Bitmap;

		private static var sNumLetters: int = 0;
		private static var sAllLetters: Array = new Array();

		private static var sActiveTile: Tile;

		private static var sTileSheet: BitmapData;

		//////////////////////////////////////////////////////////////////////
		
		public function set Active(a:Boolean):void
		{
			mSpecial = a;
		}

		//////////////////////////////////////////////////////////////////////

		public function get Position():Point
		{
			return new Point(mPosition.x, mPosition.y);
		}

		public function set Position(p:Point):void
		{
			mPosition.x = p.x;
			mPosition.y = p.y;
		}

		//////////////////////////////////////////////////////////////////////

		public function get Target():Point
		{
			return new Point(mTarget.x, mTarget.y);
		}

		public function set Target(p:Point):void
		{
			mTarget.x = p.x;
			mTarget.y = p.y;
		}

		//////////////////////////////////////////////////////////////////////

		public function get Origin():Point
		{
			return new Point(mOrigin.x, mOrigin.y);
		}

		public function set Origin(p:Point):void
		{
			mOrigin.x = p.x;
			mOrigin.y = p.y;
		}

		//////////////////////////////////////////////////////////////////////

		public function ResetPosition():void
		{
			Position = Origin;
			Target = Origin;
			mAtRest = true;
		}

		//////////////////////////////////////////////////////////////////////

		public static function SetTileSheet(tileSheet:BitmapData):void
		{
			sTileSheet = tileSheet;
		}

		//////////////////////////////////////////////////////////////////////

		private static function abs( value:Number ):Number
		{
			return value < 0 ? -value : value;
		}

		//////////////////////////////////////////////////////////////////////

		private function SetupTransform():void
		{
			var m:Matrix = new Matrix();
			m.translate(-(kWidth / 2), -(kHeight / 2));
			m.scale(mScale, mScale);
			m.rotate(mRotation);
			m.translate((kWidth / 2) + mPosition.x, (kHeight / 2) + mPosition.y);
			transform.matrix = m;
		}
		//////////////////////////////////////////////////////////////////////

		private function sgn(n:Number):int
		{
			return (n < 0) ? -1 : ((n > 0) ? 1 : 0);
		}

		//////////////////////////////////////////////////////////////////////

		public function Update():Boolean
		{
			var deltaTime:Number = Globals.TimeDelta;

			if (mState != mOldState)
			{
				mStateFrames = 0;
				mOldState = mState;
			}
			else
			{
				mStateFrames +=  1;
			}

			switch (mState)
			{
				case TileState.Lerp :
					transform.matrix = new Matrix();
					mScale = 1;
					mRotation = 0;
					var diff:Point = Target.subtract(Position);
					if (abs(diff.x) < 4 && abs(diff.y) < 4)
					{
						Position = Target;
						mAtRest = true;
					}
					else
					{
						mAtRest = false;
						diff.x *=  deltaTime * 20;
						diff.y *=  deltaTime * 20;

						// if overshooting, just set it

						var nx:Number = mPosition.x + diff.x;
						var ny:Number = mPosition.y + diff.y;

						if (sgn(Target.x - nx) != sgn(Target.x - mPosition.x))
						{
							mPosition.x = mTarget.x;
						}
						else
						{
							mPosition.x = nx;
						}
						if (sgn(Target.y - ny) != sgn(Target.y - mPosition.y))
						{
							mPosition.y = mTarget.y;
						}
						else
						{
							mPosition.y = ny;
						}

					}
					break;

				case TileState.Selected :
					Position = Origin;
					mScale = Math.cos(Globals.Time * 23) * 0.05 + 1.333;
					mRotation = Math.sin(Globals.Time * 25) * 0.1;
					break;

				case TileState.Drop :
					alpha = 1;
					mScale = 1;
					mRotation = 0;
					mPosition.y += mDropVelocity * deltaTime;
					mDropVelocity += 2000 * deltaTime;
					if (Position.y > Target.y)
					{
						Position = Target;
						mState = TileState.Lerp;
						mLayer = 0;
						mAtRest = true;
					}
					else
					{
						mAtRest = false;
					}
					break;
				
				case TileState.FadeOut:
					mAlpha -= Globals.TimeDelta * 4;
					mAtRest = (mAlpha <= 0);
					if(mAtRest)
					{
						mAlpha = 0;
						mEmpty = true;
					}
					break;
			}
			
			if(mLetter.mLetterBitmap.parent != this)
			{
				if(mLetter.mLetterBitmap.parent != null)
				{
					mLetter.mLetterBitmap.parent.removeChild(mLetter.mLetterBitmap);
				}
				var w:Number = GetLetterBitmap.width;
				addChildAt(GetLetterBitmap, 1);
				GetLetterBitmap.x = 38 - w / 2;
				
				// add score as well...
				UpdateScoreBitmap();
			}
			SetupTransform();
			UpdateTileBackground();
			alpha = mAlpha;
			return mAtRest;
		}

		//////////////////////////////////////////////////////////////////////

		public function SetTile(x:int, y:int):void
		{
			if (x != mTileSourceX || y != mTileSourceY)
			{
				mTileSourceX = x;
				mTileSourceY = y;
				mTileBitmap.bitmapData.copyPixels(sTileSheet, new Rectangle(x * kWidth, y * kHeight, kWidth, kHeight), new Point(0, 0), null, null, false);
			}
		}

		//////////////////////////////////////////////////////////////////////

		public function ResetWords():void
		{
			mTileSourceX = 0;
			mTileSourceY = 0;
			mHorizontalWord = null;
			mVerticalWord = null;
		}

		//////////////////////////////////////////////////////////////////////

		public function SetWord(word:Word, index:int):void
		{
			var pos:int = (index == 0) ? WordPosition.Beginning:(index == word.mLength - 1) ? WordPosition.End:WordPosition.Middle;

			switch (word.mDirection)
			{
				case WordDirection.Horizontal :
					mHorizontalWord = word;
					mHorizontalIndex = index;
					mHorizontalWordPosition = pos;
					break;

				case WordDirection.Vertical :
					mVerticalWord = word;
					mVerticalIndex = index;
					mVerticalWordPosition = pos;
					break;
			}
		}

		//////////////////////////////////////////////////////////////////////

		public function UpdateTileBackground():void
		{
			if (mHorizontalWord != null || mVerticalWord != null)
			{
				SetTile((mHorizontalWord == null) ? 0 : mHorizontalWordPosition, (mVerticalWord == null) ? 0 : mVerticalWordPosition);
			}
			else
			{
				SetTile(0, 4);
			}
		}

		//////////////////////////////////////////////////////////////////////

		override public function toString():String
		{
			return String.fromCharCode(Letter) + mPosition.toString();
		}

		//////////////////////////////////////////////////////////////////////

		public static function SwapLetters(a:Tile, b:Tile):void
		{
			var ai:int = a.mIndex;
			var al:Letter = a.mLetter;
			a.mIndex = b.mIndex;
			a.mLetter = b.mLetter;
			b.mIndex = ai;
			b.mLetter = al;
		}

		//////////////////////////////////////////////////////////////////////
		
		public function set TheLetter(l:Letter):void
		{
			mLetter = l;
		}

		//////////////////////////////////////////////////////////////////////

		public function get TheLetter():Letter
		{
			return mLetter;
		}

		//////////////////////////////////////////////////////////////////////
		
		public function get GetLetter():int
		{
			return mLetter.mLetter;
		}

		//////////////////////////////////////////////////////////////////////

		public function get GetLetterBitmap():Bitmap
		{
			return mLetter.mLetterBitmap;
		}

		//////////////////////////////////////////////////////////////////////
		
		private function UpdateScoreBitmap():void
		{
			// create a score bitmap if one doesn't exist already
			var score:int = Letters.GetLetterScore(mLetter.mLetter);
			if(score != mTileScore)
			{
				if(mScoreBitmap != null && contains(mScoreBitmap))
				{
					removeChild(mScoreBitmap);
				}
				mTileScore = score;
				mScoreBitmap = new Bitmap();
				mScoreBitmap.bitmapData = new BitmapData(20, 16, true, 0);

				var s:Shape = UIButton.CreateTextShape(0xffffffff, 0, mTileScore.toString(10), 20, 16, 12, "Arial");
				mScoreBitmap.bitmapData.draw(s);
				mScoreBitmap.alpha = 0.45
				mScoreBitmap.x = 52;
				mScoreBitmap.y = 58;
				addChildAt(mScoreBitmap, 2);
			}
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function Tile(letter:int)
		{
			mAlpha = 1;
			mSpecial = false;
			mDropVelocity = 400;
			mTileScore = 0;
			mSpecial = false;
			mLayer = 0;
			mAtRest = true;
			mEmpty = false;
			mState = mOldState = TileState.Lerp;
			mStateFrames = 0;
			ResetWords();
			mTileBitmap = new Bitmap();
			mTileBitmap.bitmapData = new BitmapData(kWidth,kHeight);
			mPosition = new Point();
			mTarget = new Point();
			mOrigin = new Point();
			SetTile(0, 0);
			addChildAt(mTileBitmap, 0);
			TheLetter = new Letter(letter, kWidth, kHeight);
		}
	}
}