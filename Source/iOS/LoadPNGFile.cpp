//////////////////////////////////////////////////////////////////////

#include "pch.h"

//////////////////////////////////////////////////////////////////////

bool LoadPNGFile(char const *filename, uint8 * &pixels, Size2D &size, Size2D &tsize)
{
	NSString *fullFileName = [[NSString alloc] initWithCString:filename encoding:NSASCIIStringEncoding];
	NSString *fileName = [[fullFileName lastPathComponent] stringByDeletingPathExtension];
	NSString *extension = [fullFileName pathExtension];
	NSString *path = [[NSBundle mainBundle] pathForResource:fileName ofType:extension];

	if(path == null)
	{
		return false;
	}
	else
	{
		NSData *texData = [[NSData alloc] initWithContentsOfFile:path];
		UIImage *image = [[UIImage alloc] initWithData:texData];

		if(image == nil)
		{
			return false;
		}
		
		int width = CGImageGetWidth(image.CGImage);
		int height = CGImageGetHeight(image.CGImage);
		
		size.w = width;
		size.h = height;
		
		tsize.w = NextPowerOf2(width);
		tsize.h = NextPowerOf2(height);
		
		CGColorSpaceRef colorSpace = CGColorSpaceCreateDeviceRGB();

		pixels = (uint8 *)malloc(tsize.w * tsize.h * 4);
		
		CGContextRef imgcontext = CGBitmapContextCreate(pixels, tsize.w, tsize.h, 8, 4 * tsize.w, colorSpace, kCGImageAlphaPremultipliedLast | kCGBitmapByteOrder32Big);
		
		CGColorSpaceRelease(colorSpace);

		CGContextClearRect(imgcontext, CGRectMake(0, 0, tsize.w, tsize.h));
		CGContextDrawImage(imgcontext, CGRectMake(0, tsize.h - size.h, size.w, size.h), image.CGImage);
		
		CGContextRelease(imgcontext);

		[image release];
		[texData release];

		return true;
	}
}

//////////////////////////////////////////////////////////////////////

