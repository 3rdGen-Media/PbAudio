//
//  AudioUnitView.m
//  [Pb] Audio
//
//  Created by Joe Moulton on 10/23/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#import "AudioUnitView.h"

@interface AudioUnitView()
{
    //Global Abstract View/Window Definition Properties
    CGFloat _backingWidth, _backingHeight;  //the size of the default framebuffer that renders to the screen
    CGFloat _retinaScale;                   //all iOS devices use retina scaling these days

    AUViewLayer * _auLayer;
    CGRect     _trackingRect;
    /*
    mach_timebase_info_data_t  _timeBaseInfo;
#if TARGET_OS_OSX
    NSMutableDictionary *      _activeKeysDict;
    CGEventTimestamp           _lastKeyUpEventTimeCode;
    unsigned short             _lastKeyUpEventKeyCode;  //definitive last key up event key code
    unsigned short             _lastSystemEventKeyCode; //last system generated key down event key code
#endif
    
    cr_control_event           control_events[MAX_CONTROL_EVENTS];
    cr_control_event           button_events[MAX_BUTTON_EVENTS];
    unsigned int               controlIndex;
    unsigned int               buttonIndex;
    */

}

@end

@implementation AudioUnitView

-(void)dealloc
{
    NSLog(@"CRGLView::dealloc");
}

#if TARGET_OS_OSX
@synthesize trackingRect = _trackingRect;
//-(void)setNeedsDisplay { [self setNeedsDisplay:YES]; }
#endif

#pragma mark -- Layer Delegate Methods

//IMPORTANT:  override the view's backing draw layer type for an egl/cgl surface layer
//+(Class)    layerClass       { return [AUViewLayer class]; }
//-(CALayer *)makeBackingLayer { return [AUViewLayer layer]; }

#pragma mark -- Layout Subviews Callback

-(void)layout
{
    //[super layout];
    //_glLayer.bounds  = self.frame;
    
#if TARGET_OS_OSX
    if( self.trackingArea )
    {
        if(self.trackingRect.size.width != _auLayer.bounds.size.width || self.trackingRect.size.height != _auLayer.bounds.size.height)
        {
            [self removeTrackingArea:self.trackingArea];
            _trackingRect = _auLayer.bounds;
            self.trackingArea = [[NSTrackingArea alloc] initWithRect:_trackingRect options: NSTrackingMouseEnteredAndExited | NSTrackingMouseMoved | NSTrackingActiveWhenFirstResponder | NSTrackingActiveAlways owner: self userInfo: nil];
            [self addTrackingArea:self.trackingArea];
        }
    }
#endif
}

#pragma mark -- View/Layer Content Scale

- (CGFloat)contentScaleFactor { return _retinaScale; }

-(void)setScaleFactor:(CGFloat)scale
{
    self.layer.contentsScale = _retinaScale;
#if TARGET_OS_OSX
    
#else
    self.contentScaleFactor = _retinaScale;//[[UIScreen main
#endif
    _auLayer.contentsScale = _retinaScale;//_retinaScale;
}

-(void)customizeLayerProperties
{
#if TARGET_OS_OSX
    self.wantsLayer = YES;                     //Tell NSView do allocate memory for custom backed layer drawing
    _auLayer = (AUViewLayer*)self.layer;         //Tell NSView which layer to use for custom backed layer drawing
    //self.layer.contentsScale = _retinaScale; //Layer Scaling
    //self.layer.backgroundColor = [NSColor redColor].CGColor; //NSView color will have no effect until after replaced by GL Layer
    

#else
    _auLayer = (AUViewLayer*)self.layer;
    _auLayer.contentsScale = _retinaScale;
    
    self.backgroundColor = UIColor.redColor;
#endif
    _auLayer.opaque = YES; //render as opaque for best performance on iOS (ie. don't blend with any layers underneath)

    self.clipsToBounds         = YES;
    self.layer.masksToBounds   = YES;
    _auLayer.masksToBounds     = YES;
    
    //IMPORTANT: Must explicitly set color to nil to override any defaults
    _auLayer.backgroundColor   = nil;//CocoaColor.darkGrayColor.CGColor; // Like on Win32 don't set a clear color on a system bitmap surface
    self.layer.backgroundColor = nil;//CocoaColor.darkGrayColor.CGColor; // if you don't want a default draw/paint callback to occur!
    
    //Debug with colors
    //self.layer.backgroundColor = [NSColor redColor].CGColor; //NSView color will have no effect until after replaced by GL Layer
    //self.layer.backgroundColor = [UIColor redColor].CGColor; //UIView color will change after replaced by GL Layer

    //Scale (should be handled by callbacks, no?)
    //if we are on retina display we will create a render buffer of size (view.frame.width * screenScale, view.frame.size.height * screenScale)
    //setting both layer.contentScaleFactor and _eaglLayer.contentScale should be equivalent
    
    //self.layer.contentsScale = _retinaScale;
    //self.contentScaleFactor  = _retinaScale;//[[UIScreen mainScreen] scale];
    //_glLayer.contentsScale   = _retinaScale;//[[UIScreen mainScreen] scale];
    
}


-(id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if( self )
    {
        //retina scaling gives us additional MSAA under the hood
        //we create and opengl framebuffer of the "retina" screen size
#if TARGET_OS_OSX
        //_retinaScale = NSApp.mainWindow.backingScaleFactor; //this returns 0
        NSWindow * appWindow = [[NSApplication sharedApplication] windows].firstObject;
        _retinaScale = appWindow.backingScaleFactor;
#else
        _retinaScale = UIScreen.mainScreen.scale;
#endif
        
        NSLog(@"AudioUnitView::init:frame+scale (%g, %g, %g)\n", frame.size.width, frame.size.height, _retinaScale);

        //calculate the "size" of the framebuffer in iOS screen pixels
        //ie this does not reflect opengl provided multisampling factors
        _backingWidth  = self.frame.size.width  * _retinaScale;
        _backingHeight = self.frame.size.height * _retinaScale;
        
        NSLog(@"AudioUnitView::init:backingSize (%d, %d)\n", (int)_backingWidth, (int)_backingHeight);

        //[self customizeLayerProperties];    //Create the GL Context for the layer and assign it to the view
        //[self requestHumanInterfaceDevice]; //Init GameController Events for View
        //[self createTrackingArea];

        //Needlessly triggering CALayer Cocoa Draw Pipeline for GL backed Layers is just extra overhead
        //[_glLayer setNeedsDisplay];  //Tell the system to repaint/redraw the layer
        //[self     setNeedsDisplay];  //Tell the system to repaint/redraw the view's layer configuration
                
        //mach_timebase_info(&_timeBaseInfo);
        //controlIndex = 0;
        //buttonIndex  = 0;
    }
    return self;
}

#pragma mark -- Draw to View (Won't Dispatch when Subclass Layer/DrawInContext has been set/defined on View)

#define COCOA_DRAW //Leave enabled to verify Cocoa Draw Pipeline is not dispatched
#ifdef  COCOA_DRAW

#if TARGET_OS_OSX
#define CocoaContext NSGraphicsContext.currentContext.graphicsPort
#define CocoaColor NSColor
#define CocoaRect  NSRect
#else
#define CocoaContext UIGraphicsGetCurrentContext()
#define CocoaColor UIColor
#define CocoaRect  CGRect
#endif

- (void)drawRect:(CocoaRect)rect
{
    //this function still gets triggered once on startup and once on shutdown when a CGLLayer is installed/removed
    NSLog(@"AudioUnitView::drawRect(%f, %f, %f, %f)", rect.origin.x, rect.origin.y, rect.size.width, rect.size.height);
    [super drawRect:rect];

    //but when a layer is installed you can't get a context that isn't 0x0...
    //CGContextRef ctx = CocoaContext;

    // Use green color for identifying/debugging OpenGL Views
    //CGContextSetFillColorWithColor(ctx, CocoaColor.greenColor.CGColor);
    //CGContextFillRect(ctx, rect);
}

#endif //COCOA_DRAW


@end
