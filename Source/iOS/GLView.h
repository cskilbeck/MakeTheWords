//
//  GLView.h
//  Make The Words
//
//  Created by Charlie Skilbeck on 28/10/2012.
//  Copyright (c) 2012 Ice Cream Software. All rights reserved.
//

#import <UIKit/UIKit.h>
#import <QuartzCore/QuartzCore.h>
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>

@interface GLView : UIView
{
    CAEAGLLayer *	_eaglLayer;
    EAGLContext *	_context;
    GLuint			_colorRenderBuffer;
};

@end
