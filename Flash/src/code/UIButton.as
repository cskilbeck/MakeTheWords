package code
{
	//////////////////////////////////////////////////////////////////////
	
	import flash.display.BitmapData;
	import flash.display.Shape;
	import flash.display.SimpleButton;
	import flash.geom.ColorTransform;
	import flash.geom.Matrix;
	import flash.geom.Rectangle;
	import flash.text.TextField;
	import flash.text.TextFieldAutoSize;
	import flash.text.TextFormat;

	//////////////////////////////////////////////////////////////////////
	
	public class UIButton
	{
		//////////////////////////////////////////////////////////////////////
		
		public static function CreateTextShape(forecolor:uint, backcolor:uint, text:String, width:int, height:int, fontSize:int, font:String):Shape
		{
			var tf:TextFormat = new TextFormat();
			tf.font = font;
			tf.color = forecolor;
			tf.size = fontSize;
			tf.bold = true;
			var t:TextField = new TextField();
			t.text = text;
			t.width = width;
			t.height = height;
			t.textColor = forecolor;
			t.autoSize = TextFieldAutoSize.NONE;
			t.setTextFormat(tf);
			var b:BitmapData = new BitmapData(width, height, true, 0);
			var m:Matrix = new Matrix();
			var tw:Number = t.textWidth * 1.1;
			var th:Number = t.textHeight * 1.35;
			m.translate(width / 2 - tw / 2, height / 2 - th / 2);
			b.draw(t, m);
			var s:Shape = new Shape();
			s.graphics.beginFill(backcolor, 1);
			s.graphics.drawRoundRect(0, 0, width, height, height, height);
			s.graphics.endFill();
			s.graphics.beginBitmapFill(b);
			s.graphics.drawRect(0, 0, width, height);
			s.graphics.endFill();
			return s;
		}

		//////////////////////////////////////////////////////////////////////
		
		public static function CreateImageShape(image:BitmapData, scale:Number=1, cs:Number = 0):Shape
		{
			var s:Shape = new Shape();
			var w:Number = image.width;
			var h:Number = image.height;
			var m:Matrix = new Matrix();
			var ox:Number = image.width * ((1 - scale) / 2);
			var oy:Number = image.height * ((1 - scale) / 2);
			m.scale(scale, scale);
			m.translate(ox, oy);
			var nd:BitmapData = image.clone();
			nd.colorTransform(new Rectangle(0, 0, w, h), new ColorTransform(1, 1, 1, 1, cs, cs, cs, 0));
			s.graphics.beginBitmapFill(nd, m, false);
			s.graphics.drawRect(ox, oy, w * scale, h * scale);
			s.graphics.endFill();
			return s;
		}

		//////////////////////////////////////////////////////////////////////
		
		public static function CreateTextButton(button:SimpleButton, text:String,
											w:int = 80,
											h:int = 24,
											fontSize:int = 14,
											font:String="Arial",
											textcolor:uint=0x000000, backcolor:uint=0xFFFFFF,
											pressedtextcolor:uint=0, pressedbackcolor:uint=0xd0d0d0,
											hovertextcolor:uint=0x303030, hoverbackcolor:uint=0xe0e0e0):void
		{
			button.upState = CreateTextShape(textcolor, backcolor, text, w, h, fontSize, font);
			button.overState = CreateTextShape(hovertextcolor, hoverbackcolor, text, w, h, fontSize, font);
			button.downState = CreateTextShape(pressedtextcolor, pressedbackcolor, text, w, h, fontSize, font);
			button.hitTestState = CreateTextShape(hovertextcolor, hovertextcolor, text, w, h, fontSize, font);
		}
		
		//////////////////////////////////////////////////////////////////////
		
		public static function CreateGraphicalButton(button:SimpleButton, image:BitmapData):void
		{
			button.upState = CreateImageShape(image);
			button.overState = CreateImageShape(image, 1, 32);
			button.downState = CreateImageShape(image, 0.95, 32);
			button.hitTestState = CreateImageShape(image);
		}
	}
}
