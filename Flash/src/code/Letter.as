package code
{
	import flash.display.Bitmap;
	import flash.display.BitmapData;
	import flash.geom.Point;

	//////////////////////////////////////////////////////////////////////
	
	public class Letter
	{
		//////////////////////////////////////////////////////////////////////
		
		public static var sFont:code.Font;

		//////////////////////////////////////////////////////////////////////
		
		public var mLetter:int;
		public var mLetterBitmap:Bitmap;

		private var mOffset:Point;

		//////////////////////////////////////////////////////////////////////
		
		public static function Preload():void
		{
			sFont = new Font("letters");
			sFont.Preload();
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public static function Load():void
		{
			sFont.Load();
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function CreateLetter(letter:int, w:int, h:int):Bitmap
		{
			var a:int = "A".charCodeAt(0);
			var letterBmp:Bitmap = new Bitmap();
			letterBmp.bitmapData = new BitmapData(w, h, true, 0);
			var glyph:Glyph = sFont.GetGlyph(letter - a);
			letterBmp.bitmapData = sFont.GetBitmapData(letter - a);
			return letterBmp;
		}

		//////////////////////////////////////////////////////////////////////

		public function Letter(l:int, w:int, h:int)
		{
			mLetter = l;
			mLetterBitmap = CreateLetter(l,w,h);
		}
	}
}