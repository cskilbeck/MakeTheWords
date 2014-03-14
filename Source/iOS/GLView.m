//////////////////////////////////////////////////////////////////////

#import "GLView.h"

//////////////////////////////////////////////////////////////////////

static Timer	sTimer;
static double	sOldTime;

//////////////////////////////////////////////////////////////////////

@implementation GLView

//////////////////////////////////////////////////////////////////////

+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

//////////////////////////////////////////////////////////////////////

- (void)setupLayer
{
    _eaglLayer = (CAEAGLLayer*) self.layer;
    _eaglLayer.opaque = YES;
}

//////////////////////////////////////////////////////////////////////

- (void)setupContext
{
    EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
    _context = [[EAGLContext alloc] initWithAPI:api];
    if (!_context)
	{
        NSLog(@"Failed to initialize OpenGLES 2.0 context");
        exit(1);
    }
    
    if (![EAGLContext setCurrentContext:_context])
	{
        NSLog(@"Failed to set current OpenGL context");
        exit(1);
    }
}

//////////////////////////////////////////////////////////////////////

- (void)setupRenderBuffer
{
    glGenRenderbuffers(1, &_colorRenderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderBuffer);
    [_context renderbufferStorage:GL_RENDERBUFFER fromDrawable:_eaglLayer];
}

//////////////////////////////////////////////////////////////////////

//- (void)setupDepthBuffer {
//    glGenRenderbuffers(1, &_depthRenderBuffer);
//    glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, self.frame.size.width, self.frame.size.height);
//}

//////////////////////////////////////////////////////////////////////

- (void)setupFrameBuffer
{
    GLuint framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderBuffer);
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
    glViewport(0, 0, self.frame.size.width, self.frame.size.height);
}

//////////////////////////////////////////////////////////////////////

- (void)render:(CADisplayLink*)displayLink
{
	g_Time = sTimer.GetElapsed();
	g_DeltaTime = g_Time - sOldTime;
	sOldTime = g_Time;
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	g_App->Update();
	g_App->Draw();
	SpriteList::SubmitAll();
    [_context presentRenderbuffer:GL_RENDERBUFFER];
	TouchInput::Update();
}

//////////////////////////////////////////////////////////////////////

- (void)setupDisplayLink
{
    CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
    [displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

//////////////////////////////////////////////////////////////////////

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self)
	{
		[self setupLayer];
		[self setupContext];
		//[self setupDepthBuffer];
		[self setupRenderBuffer];
		[self setupFrameBuffer];

		g_DebugSpriteList = SpriteList::Create();
		g_DebugFont = Font::Load("debug");
	
    	assert(g_App != null);
		g_App->Init();
		
		g_Time = sTimer.GetElapsed();
		sOldTime = g_Time;
		
		[self setupDisplayLink];
	}
    return self;
}

//////////////////////////////////////////////////////////////////////

- (void)dealloc
{
    CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:self selector:@selector(render:)];
	[displayLink invalidate];
    [_context release];
    _context = nil;
    [super dealloc];
}

-(IBAction)onButtonPress:(id)sender
{
    TRACE("BVUTTON!\n");
}
@end
