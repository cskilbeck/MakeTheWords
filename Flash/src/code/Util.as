package code
{
	import flash.display.Graphics;

	public class Util
	{
		//////////////////////////////////////////////////////////////////////
		
		public static function DumpObject(o:Object):void
		{
			for(var s:String in o)
			{
				trace(s + " = " + o[s]);
			}
		}

		//////////////////////////////////////////////////////////////////////
		
		public static function drawWedge(target:Graphics, x:Number, y:Number, radius:Number, arc:Number, startAngle:Number=0, yRadius:Number=0):void
		{
			// if yRadius is undefined, yRadius = radius
			if (yRadius == 0)
			{
				yRadius = radius;
			}
			
			// move to x,y position
			target.moveTo(x, y);
			// if yRadius is undefined, yRadius = radius
			if (yRadius == 0)
			{
				yRadius = radius;
			}
			// Init vars
			var segAngle:Number, theta:Number, angle:Number, angleMid:Number, segs:Number, ax:Number, ay:Number, bx:Number, by:Number, cx:Number, cy:Number;
			// limit sweep to reasonable numbers
			if (Math.abs(arc) > 360)
			{
				arc = 360;
			}
			// Flash uses 8 segments per circle, to match that, we draw in a maximum
			// of 45 degree segments. First we calculate how many segments are needed
			// for our arc.
			segs = Math.ceil(Math.abs(arc) / 45);
			// Now calculate the sweep of each segment.
			segAngle = arc / segs;
			// The math requires radians rather than degrees. To convert from degrees
			// use the formula (degrees/180)*Math.PI to get radians.
			theta = -(segAngle / 180) * Math.PI;
			// convert angle startAngle to radians
			angle = -(startAngle / 180) * Math.PI;
			// draw the curve in segments no larger than 45 degrees.
			if (segs > 0)
			{
				// draw a line from the center to the start of the curve
				ax = x + Math.cos(startAngle / 180 * Math.PI) * radius;
				ay = y + Math.sin(-startAngle / 180 * Math.PI) * yRadius;
				target.lineTo(ax, ay);
				// Loop for drawing curve segments
				for (var i:int = 0; i < segs; ++i)
				{
					angle += theta;
					angleMid = angle - (theta / 2);
					bx = x + Math.cos(angle) * radius;
					by = y + Math.sin(angle) * yRadius;
					cx = x + Math.cos(angleMid) * (radius / Math.cos(theta / 2));
					cy = y + Math.sin(angleMid) * (yRadius / Math.cos(theta / 2));
					target.curveTo(cx, cy, bx, by);
				}
				// close the wedge by drawing a line to the center
				target.lineTo(x, y);
			}
		}		
	}
}