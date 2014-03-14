package code
{
	//////////////////////////////////////////////////////////////////////

	import flash.display.Bitmap;
	import flash.display.BitmapData;
	import flash.events.EventDispatcher;
	import flash.geom.Matrix;
	import flash.geom.Point;
	import flash.geom.Rectangle;
	
	import br.com.stimuli.loading.BulkLoader;
	
	//////////////////////////////////////////////////////////////////////
	
	public class Font extends EventDispatcher
	{
		//////////////////////////////////////////////////////////////////////
	
		private var mName:String;
		private var mGlyphs:Array;
		private var mXML:XML;
		private var mBitmap:Bitmap;
		private var mScale:Number;

		//////////////////////////////////////////////////////////////////////
	
		public function Font(name:String, scale:Number=1)
		{
			mName = name;
			mGlyphs = null;
			mXML = null;
			mScale = scale;
			mBitmap = null;
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function GetGlyph(index:int):Glyph
		{
			return mGlyphs[index];
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function GetBitmapData(index:int):BitmapData
		{
			var g:Glyph = GetGlyph(index);
			var bmd:BitmapData = new BitmapData(g.w, g.h);
			bmd.copyPixels(mBitmap.bitmapData, new Rectangle(g.x, g.y, g.w, g.h), new Point(0, 0));
			return bmd;
		}

		//////////////////////////////////////////////////////////////////////
		
		public function Preload():void
		{
			Globals.AddLoadItem(mName + "0.png");
			Globals.AddLoadItem(mName + ".bitmapfont", false, BulkLoader.TYPE_XML);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public function Load():void
		{
			mBitmap = Globals.Loader.getBitmap(Globals.URL(mName + "0.png"));
			
			if(mScale != 1)
			{
				var temp:BitmapData = new BitmapData(mBitmap.width * mScale, mBitmap.height * mScale, true, 0);
				var mat:Matrix = new Matrix();
				mat.scale(mScale, mScale);
				temp.draw(mBitmap, mat, null, null, null, true);
				mBitmap = new Bitmap(temp);
			}
			
			mBitmap.smoothing = true;
			mXML = Globals.Loader.getXML(Globals.URL(mName + ".bitmapfont"));
			mGlyphs = new Array();
			var glyphList:XMLList = mXML.Glyphs;
			var glyphs:XMLList = glyphList.children();
			var t:int = glyphs.length();
			for(var i:int=0; i<t; i++)
			{
				var glyph:XML = glyphs[i];
				var graphic:XML = glyph.Graphic[0];
				var g:Glyph = new Glyph();
				g.x = graphic.@x * mScale;
				g.y = graphic.@y * mScale;
				g.w = graphic.@w * mScale;
				g.h = graphic.@h * mScale;
				g.offsetX = graphic.@offsetX * mScale;
				g.offsetY = graphic.@offsetY * mScale;
				//trace(g.x + "," + g.y + "," + g.w + "," + g.h + "," + g.offsetX + "," + g.offsetY);
				mGlyphs.push(g);
			}
		}
	}
}