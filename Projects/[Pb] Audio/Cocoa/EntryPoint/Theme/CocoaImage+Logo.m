//
//  NSImage+Logo.m
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 1/4/23.
//

#import "CocoaImage+Logo.h"
#import "NSColor+3rdGen.h"

@implementation CocoaImage (Logo)



+(void)createAppIconImages
{
    CGFloat imgScale = 0.85;
    CGSize logoImageSize = CGSizeMake(1024 * imgScale, 1024 * imgScale);
    CocoaImage * logoImage = [CocoaImage logoImage:logoImageSize];

    for(int size = 1024; size>7; size/=2)
    {
        CGSize imgSize = CGSizeMake(size, size);
        NSImage *image = [[NSImage alloc] initWithSize:imgSize];
        //[image setScalesWhenResized:YES];

        // Lock focus of image, make it a destination for drawing
        [image lockFocus];

        // Set background color to white
        [[NSColor clearColor] set];
        NSRectFill( NSMakeRect( 0, 0, imgSize.width, imgSize.height ));
        CGPoint center = CGPointMake(imgSize.width / 2., imgSize.height/2.);

        // Construct and draw path as you would in drawRect:

        //CGContextTranslateCTM(context, self.bounds.size.width/2. - circleWidth2/2.0, self.bounds.size.height/2. - circleWidth2/2.0);
        //CGContextRef context = NSGraphicsContext.currentContext.CGContext;
        //CGContextScaleCTM(context, imgScale, imgScale);
        //[[NSBezierPath bezierPathWithRect:fullBounds] setClip];

        /***************************/
        
        CGSize embedImgSize = CGSizeMake(imgSize.width * imgScale, imgSize.height * imgScale);
        CGFloat yOffset = 6./54. * embedImgSize.height;
        [logoImage drawInRect:CGRectMake(center.x - embedImgSize.width/2., center.y - embedImgSize.height/2. - yOffset/2., embedImgSize.width, embedImgSize.height)];

        
        /********************/
        
        [image unlockFocus];
        
        NSString * imgPath = [NSString stringWithFormat:@"/Users/jmoulton/Pictures/Logo%dx%d.png", (int)imgSize.width*2, (int)imgSize.height*2];
        [CocoaImage saveImage:image atPath:imgPath];
    }
}

/*
+(void)createAppIconImages
{
    for(int size = 1024; size>15; size/=2)
    {
        CGSize imgSize = CGSizeMake(size, size);
        NSString * imgPath = [NSString stringWithFormat:@"/Users/jmoulton/Pictures/Logo%dx%d.png", size, size];
        CocoaImage * image = [CocoaImage logoImage:imgSize];
        [CocoaImage saveImage:image atPath:imgPath];
    }
}
*/

+ (void)saveImage:(NSImage *)image atPath:(NSString *)path {

   CGImageRef cgRef = [image CGImageForProposedRect:NULL
                                            context:nil
                                              hints:nil];
   NSBitmapImageRep *newRep = [[NSBitmapImageRep alloc] initWithCGImage:cgRef];
   [newRep setSize:[image size]];   // if you want the same resolution
   NSData *pngData = [newRep representationUsingType:NSBitmapImageFileTypePNG properties:[NSDictionary new]];
   [pngData writeToFile:path atomically:YES];
   //[newRep autorelease];
}

+(CocoaImage *)logoImage:(CGSize)imgSize
{
    //CGRect imgRect = CGRectMake(0,0,imgSize.width, imgSize.height);
    
#if TARGET_OS_OSX
    //CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
    
    NSBitmapImageRep *offscreenRep = [[NSBitmapImageRep alloc]
       initWithBitmapDataPlanes:NULL
       pixelsWide:imgSize.width
       pixelsHigh:imgSize.height
       bitsPerSample:8
       samplesPerPixel:4
       hasAlpha:YES
       isPlanar:NO
       colorSpaceName:NSDeviceRGBColorSpace
       bitmapFormat:NSBitmapFormatAlphaFirst
       bytesPerRow:0
       bitsPerPixel:0];// autorelease];

    // set offscreen context
    NSGraphicsContext *nsContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:offscreenRep];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:nsContext];

 
    // Drawing code here.
   
    CGFloat strokeWidth = 0.5 / 54. * imgSize.height;
    NSPoint center = NSMakePoint(imgSize.width/2., imgSize.height/2.);
    NSRect  boundsCircle = CGRectMake( 0,0, imgSize.width, imgSize.height );
    CGRect  insetCircle = CGRectMake(strokeWidth, strokeWidth, imgSize.width - strokeWidth*2., imgSize.height-strokeWidth*2.);

    // erase the background by drawing white
    [[NSColor clearColor] set];
    [NSBezierPath fillRect:CGRectMake(0,0, imgSize.width, imgSize.height)];
    //[NSGraphicsContext restoreGraphicsState];
        
    // Drawing code here.
    NSColor * strokeColor = [NSColor colorWithWhite:0.25 alpha:1.0]; //[NSColor blackColor];//[NSColor colorWithWhite:130./255. alpha:1.];
    NSBezierPath * strokePath = [NSBezierPath bezierPathWithOvalInRect:boundsCircle];
    //strokePath.lineWidth = strokeWidth;

    CGFloat yOffset = 6./54. * imgSize.height;
    NSBezierPath *clipPath = [NSBezierPath bezierPathWithRect:CGRectMake(0,yOffset,imgSize.width, imgSize.height - yOffset)];
    //clipPath.usesEvenOddFillRule = YES;
    [clipPath addClip];
    
    [strokeColor set];
    [strokePath fill];
    
    //Draw the background circle for the rotary encoder with clip
    NSColor * backgroundColor = NSColor.darkGrayColor;//[NSColor colorWithRed:129./255. green:130./255. blue:132./255. alpha:1.];//[NSColor colorWithWhite:130./255. alpha:1.];
    NSBezierPath * backgroundCircle = [NSBezierPath bezierPathWithOvalInRect:insetCircle];
    
    clipPath = [NSBezierPath bezierPathWithRect:CGRectMake(0,yOffset + strokeWidth, imgSize.width, imgSize.height - (yOffset + strokeWidth*2.))];
    //clipPath.usesEvenOddFillRule = YES;
    [clipPath addClip];

    [backgroundColor set];
    [backgroundCircle fill];

    //[strokeColor setStroke];
    //[strokePath stroke];
    
    CGFloat circleWidth = floor(imgSize.height / 88.0 * 38);
    CGFloat strokeCircleWidth = floor(imgSize.height / 88.0 * 38) * 1.2;
    NSRect  strokeCircleRect = CGRectMake( imgSize.height/2. - strokeCircleWidth/2.,  imgSize.height/2. - strokeCircleWidth/2., strokeCircleWidth, strokeCircleWidth );
    NSRect  blackCircleRect = CGRectMake( imgSize.height/2. - circleWidth/2.,  imgSize.height/2. - circleWidth/2., circleWidth, circleWidth );
    CGFloat circleWidth2 = floor(imgSize.height / 88.0 * 34.);
    //NSRect blackCircleRect2 = CGRectMake( self.bounds.size.height/2. - circleWidth2/2.,  self.bounds.size.height/2. - circleWidth2/2., circleWidth2, circleWidth2 );
   
    //// Draw Stroke Circle Surrounding Black Circle Surrounding Knub
    NSBezierPath * strokeCirclePath = [NSBezierPath bezierPathWithOvalInRect:strokeCircleRect];
    strokeCirclePath.lineWidth = strokeWidth;
    [strokeColor set];
    [strokeCirclePath stroke];

    //// Draw Black Circle Surrounding Knub
    [NSGraphicsContext saveGraphicsState];

    NSBezierPath* blackCirclePath = [NSBezierPath bezierPathWithOvalInRect: blackCircleRect];
    [NSColor.blackColor setFill];
    [blackCirclePath fill];

    [NSGraphicsContext restoreGraphicsState];

    //// Bezier Encoder Knub Drawing
    CGContextRef context = NSGraphicsContext.currentContext.CGContext;
    NSColor*     knubColor = [NSColor colorWithRed: 0.655 green: 0.663 blue: 0.675 alpha: 1];
    CGFloat scale = circleWidth2/75.;//self.bounds.size.height/75.;//* self.bounds.size.width;
    [NSGraphicsContext saveGraphicsState];
    
    
    //Translate drawing context to be centered on view bounds origin (so we can rotate)
    //CGContextTranslateCTM(context, -circleWidth2/2.0, - circleWidth2/2.0);
    
    /*
    if( _control )
    {
        CGFloat rotationStep = _control->state;
        CGFloat rotationAngle = (rotationStep * 11.25) * M_PI / 180.;

        CGContextRotateCTM(context, rotationAngle);

        
        
    }
    */
    CGContextTranslateCTM(context, imgSize.width/2. - circleWidth2/2.0, imgSize.height/2. - circleWidth2/2.0);

    CGContextScaleCTM(context, scale, scale);
    //[[NSBezierPath bezierPathWithRect:fullBounds] setClip];

    NSBezierPath* bezier772Path = [NSBezierPath bezierPath];//WithRect:CGRectMake(0, 0, 75, 75)];
    
    NSAffineTransform *xform = [NSAffineTransform transform];
    [xform translateXBy:-75./2. yBy:-75/2.0 ];
    
    [bezier772Path moveToPoint: NSMakePoint(70.93, 37.89)];
    [bezier772Path curveToPoint: NSMakePoint(74.88, 45.6) controlPoint1: NSMakePoint(70.93, 41.07) controlPoint2: NSMakePoint(72.5, 43.86)];
    [bezier772Path curveToPoint: NSMakePoint(73.58, 50.18) controlPoint1: NSMakePoint(73.45, 52.14) controlPoint2: NSMakePoint(73.58, 50.18)];
    [bezier772Path curveToPoint: NSMakePoint(66.82, 53.58) controlPoint1: NSMakePoint(71.04, 50.34) controlPoint2: NSMakePoint(68.59, 51.49)];
    [bezier772Path curveToPoint: NSMakePoint(65.42, 63.61) controlPoint1: NSMakePoint(64.4, 56.48) controlPoint2: NSMakePoint(63.98, 60.38)];
    [bezier772Path curveToPoint: NSMakePoint(62.37, 66.53) controlPoint1: NSMakePoint(64.46, 64.64) controlPoint2: NSMakePoint(63.44, 65.61)];
    [bezier772Path curveToPoint: NSMakePoint(51.28, 69.16) controlPoint1: NSMakePoint(58.6, 64.91) controlPoint2: NSMakePoint(54.06, 65.85)];
    [bezier772Path curveToPoint: NSMakePoint(49.17, 73.88) controlPoint1: NSMakePoint(50.11, 70.56) controlPoint2: NSMakePoint(49.42, 72.2)];
    [bezier772Path curveToPoint: NSMakePoint(47.43, 74.44) controlPoint1: NSMakePoint(48.59, 74.06) controlPoint2: NSMakePoint(48.03, 74.28)];
    [bezier772Path curveToPoint: NSMakePoint(44.83, 75) controlPoint1: NSMakePoint(46.56, 74.68) controlPoint2: NSMakePoint(45.69, 74.83)];
    [bezier772Path curveToPoint: NSMakePoint(43.59, 73.66) controlPoint1: NSMakePoint(44.45, 74.53) controlPoint2: NSMakePoint(44.07, 74.06)];
    [bezier772Path curveToPoint: NSMakePoint(30.1, 74.84) controlPoint1: NSMakePoint(39.53, 70.26) controlPoint2: NSMakePoint(33.5, 70.79)];
    [bezier772Path curveToPoint: NSMakePoint(30, 74.96) controlPoint1: NSMakePoint(30.06, 74.88) controlPoint2: NSMakePoint(30.04, 74.92)];
    [bezier772Path curveToPoint: NSMakePoint(24.33, 73.36) controlPoint1: NSMakePoint(28.06, 74.57) controlPoint2: NSMakePoint(26.17, 74.04)];
    [bezier772Path curveToPoint: NSMakePoint(21.04, 67.29) controlPoint1: NSMakePoint(24.02, 71.08) controlPoint2: NSMakePoint(22.94, 68.89)];
    [bezier772Path curveToPoint: NSMakePoint(11.67, 65.63) controlPoint1: NSMakePoint(18.34, 65.03) controlPoint2: NSMakePoint(14.77, 64.53)];
    [bezier772Path curveToPoint: NSMakePoint(8.86, 62.75) controlPoint1: NSMakePoint(10.69, 64.72) controlPoint2: NSMakePoint(9.76, 63.76)];
    [bezier772Path curveToPoint: NSMakePoint(6.18, 51.77) controlPoint1: NSMakePoint(10.4, 58.99) controlPoint2: NSMakePoint(9.46, 54.52)];
    [bezier772Path curveToPoint: NSMakePoint(1.32, 49.65) controlPoint1: NSMakePoint(4.75, 50.57) controlPoint2: NSMakePoint(3.06, 49.88)];
    [bezier772Path curveToPoint: NSMakePoint(0.6, 47.4) controlPoint1: NSMakePoint(1.07, 48.9) controlPoint2: NSMakePoint(0.8, 48.16)];
    [bezier772Path curveToPoint: NSMakePoint(0.18, 45.45) controlPoint1: NSMakePoint(0.42, 46.74) controlPoint2: NSMakePoint(0.32, 46.1)];
    [bezier772Path curveToPoint: NSMakePoint(0.42, 45.27) controlPoint1: NSMakePoint(0.26, 45.39) controlPoint2: NSMakePoint(0.34, 45.33)];
    [bezier772Path curveToPoint: NSMakePoint(1.6, 31.77) controlPoint1: NSMakePoint(4.48, 41.87) controlPoint2: NSMakePoint(5.01, 35.83)];
    [bezier772Path curveToPoint: NSMakePoint(0, 30.35) controlPoint1: NSMakePoint(1.13, 31.21) controlPoint2: NSMakePoint(0.56, 30.77)];
    [bezier772Path curveToPoint: NSMakePoint(1.23, 25.55) controlPoint1: NSMakePoint(0.32, 28.72) controlPoint2: NSMakePoint(0.71, 27.11)];
    [bezier772Path curveToPoint: NSMakePoint(6.19, 23.41) controlPoint1: NSMakePoint(2.99, 25.32) controlPoint2: NSMakePoint(4.72, 24.64)];
    [bezier772Path curveToPoint: NSMakePoint(8.79, 12.28) controlPoint1: NSMakePoint(9.51, 20.62) controlPoint2: NSMakePoint(10.43, 16.07)];
    [bezier772Path curveToPoint: NSMakePoint(11.73, 9.29) controlPoint1: NSMakePoint(9.71, 11.23) controlPoint2: NSMakePoint(10.7, 10.24)];
    [bezier772Path curveToPoint: NSMakePoint(21.7, 7.87) controlPoint1: NSMakePoint(14.96, 10.7) controlPoint2: NSMakePoint(18.83, 10.28)];
    [bezier772Path curveToPoint: NSMakePoint(25.06, 1.38) controlPoint1: NSMakePoint(23.73, 6.17) controlPoint2: NSMakePoint(24.84, 3.82)];
    [bezier772Path curveToPoint: NSMakePoint(27.64, 0.55) controlPoint1: NSMakePoint(25.91, 1.09) controlPoint2: NSMakePoint(26.76, 0.79)];
    [bezier772Path curveToPoint: NSMakePoint(28.54, 0.36) controlPoint1: NSMakePoint(27.94, 0.47) controlPoint2: NSMakePoint(28.25, 0.43)];
    [bezier772Path curveToPoint: NSMakePoint(35.76, 4.78) controlPoint1: NSMakePoint(30.1, 2.78) controlPoint2: NSMakePoint(32.68, 4.51)];
    [bezier772Path curveToPoint: NSMakePoint(44.9, 0) controlPoint1: NSMakePoint(39.6, 5.11) controlPoint2: NSMakePoint(43.1, 3.14)];
    [bezier772Path curveToPoint: NSMakePoint(48.68, 0.95) controlPoint1: NSMakePoint(46.18, 0.25) controlPoint2: NSMakePoint(47.44, 0.57)];
    [bezier772Path curveToPoint: NSMakePoint(49.16, 3.14) controlPoint1: NSMakePoint(48.76, 1.68) controlPoint2: NSMakePoint(48.9, 2.42)];
    [bezier772Path curveToPoint: NSMakePoint(61.44, 8.86) controlPoint1: NSMakePoint(50.98, 8.11) controlPoint2: NSMakePoint(56.48, 10.67)];
    [bezier772Path curveToPoint: NSMakePoint(62.39, 8.46) controlPoint1: NSMakePoint(61.77, 8.74) controlPoint2: NSMakePoint(62.08, 8.61)];
    [bezier772Path curveToPoint: NSMakePoint(66.1, 12.12) controlPoint1: NSMakePoint(62.39, 8.46) controlPoint2: NSMakePoint(61.85, 7.34)];
    [bezier772Path curveToPoint: NSMakePoint(68.05, 22.8) controlPoint1: NSMakePoint(64.52, 15.64) controlPoint2: NSMakePoint(65.16, 19.91)];
    [bezier772Path curveToPoint: NSMakePoint(73.8, 25.52) controlPoint1: NSMakePoint(69.66, 24.41) controlPoint2: NSMakePoint(71.7, 25.3)];
    [bezier772Path curveToPoint: NSMakePoint(74.47, 27.6) controlPoint1: NSMakePoint(74.03, 26.22) controlPoint2: NSMakePoint(74.28, 26.89)];
    [bezier772Path curveToPoint: NSMakePoint(75, 30.08) controlPoint1: NSMakePoint(74.7, 28.43) controlPoint2: NSMakePoint(74.84, 29.25)];
    [bezier772Path curveToPoint: NSMakePoint(70.93, 37.89) controlPoint1: NSMakePoint(72.55, 31.81) controlPoint2: NSMakePoint(70.93, 34.65)];
    
    //Important:  the trick to applying translation prior to rotating a bezier path
    //is to perform the translation on the curve values prior to closing the path!!!
    [bezier772Path transformUsingAffineTransform:xform];
    [bezier772Path closePath];


    NSAffineTransform *xform2 = [NSAffineTransform transform];
    [xform2 translateXBy:75./2. yBy:75./2.];
    [bezier772Path transformUsingAffineTransform:xform2];
    
    //[xform translateXBy:-1*scale  yBy:-1*scale];
     
    [knubColor setFill];
    [bezier772Path fill];

    [NSGraphicsContext restoreGraphicsState];

    [NSGraphicsContext saveGraphicsState];

    //draw the rotary leds
    CGFloat innerRadius = strokeCircleRect.size.height/2.;//(blackCircleRect.size.height +6)/2.;
    CGFloat LEDWidth = imgSize.width / 20. - 0.25;
    CGFloat LEDHeight = (imgSize.height/2. - innerRadius);
    CGFloat offset = LEDHeight/4.5;
    LEDHeight -= offset;
    
    /*
    NSBezierPath* LEDPath = [NSBezierPath bezierPath];//WithRect:CGRectMake(0, 0, LEDWidth, LEDHeight)];
    
    NSPoint point = NSMakePoint(self.bounds.size.width/2. - LEDWidth/2., self.bounds.size.height/2. + innerRadius + offset/3.);//LEDHeight/2.);
    [LEDPath moveToPoint: point];
    [LEDPath lineToPoint:NSMakePoint(point.x, point.y + LEDHeight)];
    [LEDPath lineToPoint:NSMakePoint(point.x + LEDWidth, point.y + LEDHeight)];
    [LEDPath lineToPoint:NSMakePoint(point.x + LEDWidth, point.y)];
    [LEDPath closePath];
    */
    
    NSPoint point = NSMakePoint(imgSize.width/2. - LEDWidth/2., imgSize.height/2. + innerRadius + offset/3.);//LEDHeight/2.);
    NSBezierPath* LEDPath = [NSBezierPath bezierPathWithRoundedRect:CGRectMake(point.x, point.y, LEDWidth, LEDHeight) xRadius:LEDWidth/2. yRadius:LEDWidth/2.];
    [LEDPath closePath];
    
    [[NSColor.systemRedColor colorWithAlphaComponent:0.9] setFill];
    

        CGFloat rotAngle = 125.;
        CGFloat angleDelta = 25.;
        for( int ledIndex = 0; ledIndex < 11; ledIndex++)
        {
            NSBezierPath* LEDPathCopy = [LEDPath copy];
            // Move origin to center of the circle
            NSAffineTransform *ledXform = [NSAffineTransform transform];
            [ledXform translateXBy:-center.x yBy:-center.y];
            [LEDPathCopy transformUsingAffineTransform:ledXform];
            
            NSAffineTransform *rot = [NSAffineTransform transform];
            
            CGFloat angle = rotAngle;
            if( angle < 0.0 ) angle += 360.;
            [rot rotateByRadians:angle * M_PI / 180.];
            rotAngle -= angleDelta;
            
            [LEDPathCopy transformUsingAffineTransform:rot];
            
            // Move origin back to original location
            NSAffineTransform *ledXform2 = [NSAffineTransform transform];
            [ledXform2 translateXBy:center.x yBy:center.y];
            [LEDPathCopy transformUsingAffineTransform:ledXform2];
            
            // Draw the V-POT LED ring strip
            [LEDPathCopy fill];
        }
    
    
    CGFloat ledCircleWidth = imgSize.height /17.5;
    NSBezierPath * LEDCircle = [NSBezierPath bezierPathWithOvalInRect:CGRectMake( imgSize.width/2. - ledCircleWidth/2., imgSize.height * 0.15, ledCircleWidth, ledCircleWidth )];
    [LEDCircle fill];
    [NSGraphicsContext restoreGraphicsState];

    
     // done drawing, so set the current context back to what it was
     [NSGraphicsContext restoreGraphicsState];

     // create an NSImage and add the rep to it
     NSImage *image = [[NSImage alloc] initWithSize:imgSize];// autorelease];
     [image addRepresentation:offscreenRep];

     // then go on to save or view the NSImage
     
    
    
    return image;

#else
    UIGraphicsBeginImageContext(rect.size);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    CGContextSetFillColorWithColor(context, [color CGColor]);
    //CGContextFillRect(context, rect);
    
    CGContextFillEllipseInRect(context, circleRect);
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
#endif

    return image;
}


+(CocoaImage *)logoImage2:(CGSize)imgSize
{
    //CGRect imgRect = CGRectMake(0,0,imgSize.width, imgSize.height);
    
#if TARGET_OS_OSX
    //CGContextRef context = (CGContextRef)[[NSGraphicsContext currentContext] CGContext];
    
    NSBitmapImageRep *offscreenRep = [[NSBitmapImageRep alloc]
       initWithBitmapDataPlanes:NULL
       pixelsWide:imgSize.width
       pixelsHigh:imgSize.height
       bitsPerSample:8
       samplesPerPixel:4
       hasAlpha:YES
       isPlanar:NO
       colorSpaceName:NSDeviceRGBColorSpace
       bitmapFormat:NSBitmapFormatAlphaFirst
       bytesPerRow:0
       bitsPerPixel:0];// autorelease];

    // set offscreen context
    NSGraphicsContext *nsContext = [NSGraphicsContext graphicsContextWithBitmapImageRep:offscreenRep];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:nsContext];

 
    // Drawing code here.
   
    CGFloat strokeWidth = 0.5 / 54. * imgSize.height;
    NSPoint center = NSMakePoint(imgSize.width/2., imgSize.height/2.);
    NSRect  boundsCircle = CGRectMake( 0,0, imgSize.width, imgSize.height );
    CGRect  insetCircle = CGRectMake(strokeWidth, strokeWidth, imgSize.width - strokeWidth*2., imgSize.height-strokeWidth*2.);

    // erase the background by drawing white
    [[NSColor clearColor] set];
    [NSBezierPath fillRect:CGRectMake(0,0, imgSize.width, imgSize.height)];
    //[NSGraphicsContext restoreGraphicsState];
        
    // Drawing code here.
    NSColor * strokeColor = [NSColor colorWithWhite:0.25 alpha:1.0]; //[NSColor blackColor];//[NSColor colorWithWhite:130./255. alpha:1.];
    NSBezierPath * strokePath = [NSBezierPath bezierPathWithOvalInRect:boundsCircle];
    //strokePath.lineWidth = strokeWidth;

    CGFloat yOffset = 6./54. * imgSize.height;
    NSBezierPath *clipPath = [NSBezierPath bezierPathWithRect:CGRectMake(0,yOffset,imgSize.width, imgSize.height - yOffset)];
    //clipPath.usesEvenOddFillRule = YES;
    [clipPath addClip];
    
    [strokeColor set];
    [strokePath fill];
    
    //Draw the background circle for the rotary encoder with clip
    NSColor * backgroundColor = NSColor.darkGrayColor;//[NSColor colorWithRed:129./255. green:130./255. blue:132./255. alpha:1.];//[NSColor colorWithWhite:130./255. alpha:1.];
    NSBezierPath * backgroundCircle = [NSBezierPath bezierPathWithOvalInRect:insetCircle];
    
    clipPath = [NSBezierPath bezierPathWithRect:CGRectMake(0,yOffset + strokeWidth, imgSize.width, imgSize.height - (yOffset + strokeWidth*2.))];
    //clipPath.usesEvenOddFillRule = YES;
    [clipPath addClip];

    [backgroundColor set];
    [backgroundCircle fill];

    //[strokeColor setStroke];
    //[strokePath stroke];
    
    CGFloat circleWidth = floor(imgSize.height / 88.0 * 38);
    CGFloat strokeCircleWidth = floor(imgSize.height / 88.0 * 38) * 1.2;
    NSRect  strokeCircleRect = CGRectMake( imgSize.height/2. - strokeCircleWidth/2.,  imgSize.height/2. - strokeCircleWidth/2., strokeCircleWidth, strokeCircleWidth );
    NSRect  blackCircleRect = CGRectMake( imgSize.height/2. - circleWidth/2.,  imgSize.height/2. - circleWidth/2., circleWidth, circleWidth );
    CGFloat circleWidth2 = floor(imgSize.height / 88.0 * 34.);
    //NSRect blackCircleRect2 = CGRectMake( self.bounds.size.height/2. - circleWidth2/2.,  self.bounds.size.height/2. - circleWidth2/2., circleWidth2, circleWidth2 );
   
    //// Draw Stroke Circle Surrounding Black Circle Surrounding Knub
    NSBezierPath * strokeCirclePath = [NSBezierPath bezierPathWithOvalInRect:strokeCircleRect];
    strokeCirclePath.lineWidth = strokeWidth;
    [strokeColor set];
    [strokeCirclePath stroke];

    //// Draw Black Circle Surrounding Knub
    [NSGraphicsContext saveGraphicsState];

    NSBezierPath* blackCirclePath = [NSBezierPath bezierPathWithOvalInRect: blackCircleRect];
    [NSColor.blackColor setFill];
    [blackCirclePath fill];

    [NSGraphicsContext restoreGraphicsState];

    //// Bezier Encoder Knub Drawing
    CGContextRef context = NSGraphicsContext.currentContext.CGContext;
    NSColor*     knubColor = [NSColor colorWithRed: 0.655 green: 0.663 blue: 0.675 alpha: 1];
    CGFloat scale = circleWidth2/75.;//self.bounds.size.height/75.;//* self.bounds.size.width;
    [NSGraphicsContext saveGraphicsState];
    
    
    //Translate drawing context to be centered on view bounds origin (so we can rotate)
    //CGContextTranslateCTM(context, -circleWidth2/2.0, - circleWidth2/2.0);
    
    /*
    if( _control )
    {
        CGFloat rotationStep = _control->state;
        CGFloat rotationAngle = (rotationStep * 11.25) * M_PI / 180.;

        CGContextRotateCTM(context, rotationAngle);

        
        
    }
    */
    CGContextTranslateCTM(context, imgSize.width/2. - circleWidth2/2.0, imgSize.height/2. - circleWidth2/2.0);

    CGContextScaleCTM(context, scale, scale);
    //[[NSBezierPath bezierPathWithRect:fullBounds] setClip];

    NSBezierPath* bezier772Path = [NSBezierPath bezierPath];//WithRect:CGRectMake(0, 0, 75, 75)];
    
    NSAffineTransform *xform = [NSAffineTransform transform];
    [xform translateXBy:-75./2. yBy:-75/2.0 ];
    
    [bezier772Path moveToPoint: NSMakePoint(70.93, 37.89)];
    [bezier772Path curveToPoint: NSMakePoint(74.88, 45.6) controlPoint1: NSMakePoint(70.93, 41.07) controlPoint2: NSMakePoint(72.5, 43.86)];
    [bezier772Path curveToPoint: NSMakePoint(73.58, 50.18) controlPoint1: NSMakePoint(73.45, 52.14) controlPoint2: NSMakePoint(73.58, 50.18)];
    [bezier772Path curveToPoint: NSMakePoint(66.82, 53.58) controlPoint1: NSMakePoint(71.04, 50.34) controlPoint2: NSMakePoint(68.59, 51.49)];
    [bezier772Path curveToPoint: NSMakePoint(65.42, 63.61) controlPoint1: NSMakePoint(64.4, 56.48) controlPoint2: NSMakePoint(63.98, 60.38)];
    [bezier772Path curveToPoint: NSMakePoint(62.37, 66.53) controlPoint1: NSMakePoint(64.46, 64.64) controlPoint2: NSMakePoint(63.44, 65.61)];
    [bezier772Path curveToPoint: NSMakePoint(51.28, 69.16) controlPoint1: NSMakePoint(58.6, 64.91) controlPoint2: NSMakePoint(54.06, 65.85)];
    [bezier772Path curveToPoint: NSMakePoint(49.17, 73.88) controlPoint1: NSMakePoint(50.11, 70.56) controlPoint2: NSMakePoint(49.42, 72.2)];
    [bezier772Path curveToPoint: NSMakePoint(47.43, 74.44) controlPoint1: NSMakePoint(48.59, 74.06) controlPoint2: NSMakePoint(48.03, 74.28)];
    [bezier772Path curveToPoint: NSMakePoint(44.83, 75) controlPoint1: NSMakePoint(46.56, 74.68) controlPoint2: NSMakePoint(45.69, 74.83)];
    [bezier772Path curveToPoint: NSMakePoint(43.59, 73.66) controlPoint1: NSMakePoint(44.45, 74.53) controlPoint2: NSMakePoint(44.07, 74.06)];
    [bezier772Path curveToPoint: NSMakePoint(30.1, 74.84) controlPoint1: NSMakePoint(39.53, 70.26) controlPoint2: NSMakePoint(33.5, 70.79)];
    [bezier772Path curveToPoint: NSMakePoint(30, 74.96) controlPoint1: NSMakePoint(30.06, 74.88) controlPoint2: NSMakePoint(30.04, 74.92)];
    [bezier772Path curveToPoint: NSMakePoint(24.33, 73.36) controlPoint1: NSMakePoint(28.06, 74.57) controlPoint2: NSMakePoint(26.17, 74.04)];
    [bezier772Path curveToPoint: NSMakePoint(21.04, 67.29) controlPoint1: NSMakePoint(24.02, 71.08) controlPoint2: NSMakePoint(22.94, 68.89)];
    [bezier772Path curveToPoint: NSMakePoint(11.67, 65.63) controlPoint1: NSMakePoint(18.34, 65.03) controlPoint2: NSMakePoint(14.77, 64.53)];
    [bezier772Path curveToPoint: NSMakePoint(8.86, 62.75) controlPoint1: NSMakePoint(10.69, 64.72) controlPoint2: NSMakePoint(9.76, 63.76)];
    [bezier772Path curveToPoint: NSMakePoint(6.18, 51.77) controlPoint1: NSMakePoint(10.4, 58.99) controlPoint2: NSMakePoint(9.46, 54.52)];
    [bezier772Path curveToPoint: NSMakePoint(1.32, 49.65) controlPoint1: NSMakePoint(4.75, 50.57) controlPoint2: NSMakePoint(3.06, 49.88)];
    [bezier772Path curveToPoint: NSMakePoint(0.6, 47.4) controlPoint1: NSMakePoint(1.07, 48.9) controlPoint2: NSMakePoint(0.8, 48.16)];
    [bezier772Path curveToPoint: NSMakePoint(0.18, 45.45) controlPoint1: NSMakePoint(0.42, 46.74) controlPoint2: NSMakePoint(0.32, 46.1)];
    [bezier772Path curveToPoint: NSMakePoint(0.42, 45.27) controlPoint1: NSMakePoint(0.26, 45.39) controlPoint2: NSMakePoint(0.34, 45.33)];
    [bezier772Path curveToPoint: NSMakePoint(1.6, 31.77) controlPoint1: NSMakePoint(4.48, 41.87) controlPoint2: NSMakePoint(5.01, 35.83)];
    [bezier772Path curveToPoint: NSMakePoint(0, 30.35) controlPoint1: NSMakePoint(1.13, 31.21) controlPoint2: NSMakePoint(0.56, 30.77)];
    [bezier772Path curveToPoint: NSMakePoint(1.23, 25.55) controlPoint1: NSMakePoint(0.32, 28.72) controlPoint2: NSMakePoint(0.71, 27.11)];
    [bezier772Path curveToPoint: NSMakePoint(6.19, 23.41) controlPoint1: NSMakePoint(2.99, 25.32) controlPoint2: NSMakePoint(4.72, 24.64)];
    [bezier772Path curveToPoint: NSMakePoint(8.79, 12.28) controlPoint1: NSMakePoint(9.51, 20.62) controlPoint2: NSMakePoint(10.43, 16.07)];
    [bezier772Path curveToPoint: NSMakePoint(11.73, 9.29) controlPoint1: NSMakePoint(9.71, 11.23) controlPoint2: NSMakePoint(10.7, 10.24)];
    [bezier772Path curveToPoint: NSMakePoint(21.7, 7.87) controlPoint1: NSMakePoint(14.96, 10.7) controlPoint2: NSMakePoint(18.83, 10.28)];
    [bezier772Path curveToPoint: NSMakePoint(25.06, 1.38) controlPoint1: NSMakePoint(23.73, 6.17) controlPoint2: NSMakePoint(24.84, 3.82)];
    [bezier772Path curveToPoint: NSMakePoint(27.64, 0.55) controlPoint1: NSMakePoint(25.91, 1.09) controlPoint2: NSMakePoint(26.76, 0.79)];
    [bezier772Path curveToPoint: NSMakePoint(28.54, 0.36) controlPoint1: NSMakePoint(27.94, 0.47) controlPoint2: NSMakePoint(28.25, 0.43)];
    [bezier772Path curveToPoint: NSMakePoint(35.76, 4.78) controlPoint1: NSMakePoint(30.1, 2.78) controlPoint2: NSMakePoint(32.68, 4.51)];
    [bezier772Path curveToPoint: NSMakePoint(44.9, 0) controlPoint1: NSMakePoint(39.6, 5.11) controlPoint2: NSMakePoint(43.1, 3.14)];
    [bezier772Path curveToPoint: NSMakePoint(48.68, 0.95) controlPoint1: NSMakePoint(46.18, 0.25) controlPoint2: NSMakePoint(47.44, 0.57)];
    [bezier772Path curveToPoint: NSMakePoint(49.16, 3.14) controlPoint1: NSMakePoint(48.76, 1.68) controlPoint2: NSMakePoint(48.9, 2.42)];
    [bezier772Path curveToPoint: NSMakePoint(61.44, 8.86) controlPoint1: NSMakePoint(50.98, 8.11) controlPoint2: NSMakePoint(56.48, 10.67)];
    [bezier772Path curveToPoint: NSMakePoint(62.39, 8.46) controlPoint1: NSMakePoint(61.77, 8.74) controlPoint2: NSMakePoint(62.08, 8.61)];
    [bezier772Path curveToPoint: NSMakePoint(66.1, 12.12) controlPoint1: NSMakePoint(62.39, 8.46) controlPoint2: NSMakePoint(61.85, 7.34)];
    [bezier772Path curveToPoint: NSMakePoint(68.05, 22.8) controlPoint1: NSMakePoint(64.52, 15.64) controlPoint2: NSMakePoint(65.16, 19.91)];
    [bezier772Path curveToPoint: NSMakePoint(73.8, 25.52) controlPoint1: NSMakePoint(69.66, 24.41) controlPoint2: NSMakePoint(71.7, 25.3)];
    [bezier772Path curveToPoint: NSMakePoint(74.47, 27.6) controlPoint1: NSMakePoint(74.03, 26.22) controlPoint2: NSMakePoint(74.28, 26.89)];
    [bezier772Path curveToPoint: NSMakePoint(75, 30.08) controlPoint1: NSMakePoint(74.7, 28.43) controlPoint2: NSMakePoint(74.84, 29.25)];
    [bezier772Path curveToPoint: NSMakePoint(70.93, 37.89) controlPoint1: NSMakePoint(72.55, 31.81) controlPoint2: NSMakePoint(70.93, 34.65)];
    
    //Important:  the trick to applying translation prior to rotating a bezier path
    //is to perform the translation on the curve values prior to closing the path!!!
    [bezier772Path transformUsingAffineTransform:xform];
    [bezier772Path closePath];


    NSAffineTransform *xform2 = [NSAffineTransform transform];
    [xform2 translateXBy:75./2. yBy:75./2.];
    [bezier772Path transformUsingAffineTransform:xform2];
    
    //[xform translateXBy:-1*scale  yBy:-1*scale];
     
    [knubColor setFill];
    [bezier772Path fill];

    [NSGraphicsContext restoreGraphicsState];

    [NSGraphicsContext saveGraphicsState];

    //draw the rotary leds
    CGFloat innerRadius = strokeCircleRect.size.height/2.;//(blackCircleRect.size.height +6)/2.;
    CGFloat LEDWidth = imgSize.width / 20. - 0.25;
    CGFloat LEDHeight = (imgSize.height/2. - innerRadius);
    CGFloat offset = LEDHeight/4.5;
    LEDHeight -= offset;
    
    /*
    NSBezierPath* LEDPath = [NSBezierPath bezierPath];//WithRect:CGRectMake(0, 0, LEDWidth, LEDHeight)];
    
    NSPoint point = NSMakePoint(self.bounds.size.width/2. - LEDWidth/2., self.bounds.size.height/2. + innerRadius + offset/3.);//LEDHeight/2.);
    [LEDPath moveToPoint: point];
    [LEDPath lineToPoint:NSMakePoint(point.x, point.y + LEDHeight)];
    [LEDPath lineToPoint:NSMakePoint(point.x + LEDWidth, point.y + LEDHeight)];
    [LEDPath lineToPoint:NSMakePoint(point.x + LEDWidth, point.y)];
    [LEDPath closePath];
    */
    
    NSPoint point = NSMakePoint(imgSize.width/2. - LEDWidth/2., imgSize.height/2. + innerRadius + offset/3.);//LEDHeight/2.);
    NSBezierPath* LEDPath = [NSBezierPath bezierPathWithRoundedRect:CGRectMake(point.x, point.y, LEDWidth, LEDHeight) xRadius:LEDWidth/2. yRadius:LEDWidth/2.];
    [LEDPath closePath];
    
    [[NSColor.blackColor colorWithAlphaComponent:0.9] setFill];
    

        CGFloat rotAngle = 125.;
        CGFloat angleDelta = 25.;
        for( int ledIndex = 0; ledIndex < 0; ledIndex++)
        {
            NSBezierPath* LEDPathCopy = [LEDPath copy];
            // Move origin to center of the circle
            NSAffineTransform *ledXform = [NSAffineTransform transform];
            [ledXform translateXBy:-center.x yBy:-center.y];
            [LEDPathCopy transformUsingAffineTransform:ledXform];
            
            NSAffineTransform *rot = [NSAffineTransform transform];
            
            CGFloat angle = rotAngle;
            if( angle < 0.0 ) angle += 360.;
            [rot rotateByRadians:angle * M_PI / 180.];
            rotAngle -= angleDelta;
            
            [LEDPathCopy transformUsingAffineTransform:rot];
            
            // Move origin back to original location
            NSAffineTransform *ledXform2 = [NSAffineTransform transform];
            [ledXform2 translateXBy:center.x yBy:center.y];
            [LEDPathCopy transformUsingAffineTransform:ledXform2];
            
            // Draw the V-POT LED ring strip
            [LEDPathCopy fill];
        }
    
    
    CGFloat ledCircleWidth = imgSize.height /17.5;
    NSBezierPath * LEDCircle = [NSBezierPath bezierPathWithOvalInRect:CGRectMake( imgSize.width/2. - ledCircleWidth/2., imgSize.height * 0.15, ledCircleWidth, ledCircleWidth )];
    [LEDCircle fill];
    [NSGraphicsContext restoreGraphicsState];

    
     // done drawing, so set the current context back to what it was
     [NSGraphicsContext restoreGraphicsState];

     // create an NSImage and add the rep to it
     NSImage *image = [[NSImage alloc] initWithSize:imgSize];// autorelease];
     [image addRepresentation:offscreenRep];

     // then go on to save or view the NSImage
     
    
    
    return image;

#else
    UIGraphicsBeginImageContext(rect.size);
    CGContextRef context = UIGraphicsGetCurrentContext();
    
    CGContextSetFillColorWithColor(context, [color CGColor]);
    //CGContextFillRect(context, rect);
    
    CGContextFillEllipseInRect(context, circleRect);
    
    UIImage *image = UIGraphicsGetImageFromCurrentImageContext();
    UIGraphicsEndImageContext();
#endif

    return image;
}



@end
