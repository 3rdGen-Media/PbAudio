#import <TargetConditionals.h>

//--------------------------------------------------//
//
//  CRCAEAGLLayer.h
//  CRViewer
//
//  Created by Joe Moulton on 2/3/19.
//  Copyright © 2019 Abstract Embedded. All rights reserved.
//
//--------------------------------------------------//

#if TARGET_OS_OSX //-------------------------------//
#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#define CocoaEvent               NSEvent
#else  //------------------------------------------//
#import <UIKit/UIKit.h>          //needed for UIScreen for initial layer contentsScale
#import <QuartzCore/QuartzCore.h>
#define CocoaEvent               UIEvent
#endif //-----------------------------------------//

//--------------------------------------------------//

@interface AUViewLayer : CALayer


@end

//--------------------------------------------------//
