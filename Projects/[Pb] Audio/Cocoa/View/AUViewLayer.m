#import "AUViewLayer.h"

//
//  CREAGLContext.m
//  CRViewer
//
//  Created by Joe Moulton on 2/3/19.
//  Copyright © 2019 Abstract Embedded. All rights reserved.
//


#if !TARGET_OS_OSX
#define CocoaColor UIColor
#else
#define CocoaColor NSColor
#endif

@interface AUViewLayer()
{

    CGFloat _retinaScale;
}
@end

@implementation AUViewLayer


-(void)dealloc
{
    NSLog(@"AUViewLayer::dealloc");
}

//overriding this to always return retina scale seems to be the only way to prevent UIKit
//from rescaling the layer to point size when it is added to a superview
- (CGFloat)contentsScale
{
    return _retinaScale;//[[UIScreen mainScreen] scale];
}

-(void)setContentsScale:(CGFloat)contentsScale
{
    NSLog(@"AUViewLayer::setContentsScale = %f", contentsScale);
    [super setContentsScale:contentsScale];
}

//this is called when the CA layer is resized
- (void)setBounds:(CGRect)bounds
{
    CGRect oldBounds = self.bounds;
    [super setBounds:bounds];
    NSLog(@"AUViewLayer setBounds:  %g %g", bounds.size.width, bounds.size.height);

#if TARGET_OS_OSX
    //_crView.contentRect.width  = bounds.size.width  * _retinaScale;
    //_crView.contentRect.height = bounds.size.height * _retinaScale;
#else
    //if( oldBounds.size.width != bounds.size.width || oldBounds.size.height != bounds.size.height || _retinaScale != self.contentsScale )
    //    [self createMandatoryBuffers:CGSizeMake(bounds.size.width,bounds.size.height) scale:_retinaScale];
    _retinaScale = self.contentsScale;
    

#endif
    
 
}




//override init
- (id)init
{
    self = [super init];
    if (self)
    {
        NSLog(@"AUViewLayer init\n");

       
#if TARGET_OS_OSX
        self.needsDisplayOnBoundsChange = NO;
        //self.asynchronous = YES;
        //self.shouldUpdate = YES;
        //self.limitFPS = NO;
        _retinaScale = [NSScreen mainScreen].backingScaleFactor;
#else
        _retinaScale = [UIScreen mainScreen].scale;
        //self.opaque = YES; //render as opaque for best performance on iOS (ie. don't blend with any layers underneath)
#endif
        
        // we can choose to allocate a platform provided multisampled framebuffer that will be
        // downsampled into a render buffer which gets presented to the screen
        //_useMSAA = NO;
        //_sampleFactor = 2;
        
        // we can create an FBO to render to a texture before rendering that texture to the framebuffer
        // the FBO can be multisampled against the size of the framebuffer (even if the framebuffer is already multisampled?)
        //_useFBO = NO; //render off-screen using a frame buffer object
        
    }
    return self;
}


#pragma mark -- Core Render


#pragma mark -- Draw to Layer (Won't Dispatch if Layer is backed by CGLContext)

#define COCOA_DRAW //Leave enabled to verify Cocoa Draw Pipeline is not dispatched
#ifdef  COCOA_DRAW

- (void)drawInContext:(CGContextRef)ctx
{
    NSLog(@"CRGLLayer::drawInContext");
    //CGContextSaveGState(ctx);

    // Use green for identifying/debugging OpenGL layers
    CGContextSetFillColorWithColor(ctx, CocoaColor.greenColor.CGColor);
    CGContextFillRect(ctx, self.bounds);

    //CGContextRestoreGState(ctx);
}

- (void)displayLayer:(CALayer *)layer
{
    NSLog(@"CRGLLayer::displayLayer");
    //[self drawRect:self.bounds];
}

- (void)drawLayer:(CALayer *)layer inContext:(CGContextRef)ctx
{
    NSLog(@"CRGLLayer::drawLayer:inContext:");
    [self drawInContext:ctx];
}

#endif


@end
