//
//  AudioUnitView.h
//  [Pb] Audio
//
//  Created by Joe Moulton on 10/23/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//


#import <TargetConditionals.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#define CocoaView                NSView
#define CocoaViewController      AUViewController
#define CocoaScreenSize          NSScreen.mainScreen.frame.size
#define CocoaWindow              NSWindow
#define CocoaColor               NSColor
#else
#import <UIKit/UIKit.h>
#define CocoaView                UIView
#define CocoaViewController      AUViewController
#define CocoaScreenSize          UIScreen.mainScreen.bounds.size
#define CocoaWindow              UIWindow
#define CocoaColor               UIColor
#endif

#import "AUViewLayer.h"

NS_ASSUME_NONNULL_BEGIN

@interface AudioUnitView : CocoaView

#if TARGET_OS_OSX
@property (readonly)                  CGRect             trackingRect;
@property (nonatomic, retain) NSTrackingArea * _Nullable trackingArea;
#endif

-(void)setScaleFactor:(CGFloat)scale;

@end

NS_ASSUME_NONNULL_END
