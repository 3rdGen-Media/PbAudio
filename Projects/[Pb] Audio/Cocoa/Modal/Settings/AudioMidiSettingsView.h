//
//  AudioMidiSettingsView.h
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/10/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#import <Cocoa/Cocoa.h>

#if TARGET_OS_OSX
#define CocoaTextField NSTextField
#else
#import <UIKit/UITextField.h>
#define CocoaTextField UITextField
#endif

#import "NSCheckboxView.h"

NS_ASSUME_NONNULL_BEGIN

@interface AudioMidiSettingsView : NSVisualEffectView <NSTextFieldDelegate, NSCheckboxViewDelegate>

//-(void)updateDocumentView;

//-(id)initWithFrame:(NSRect)frame andProxy:(NSString* _Nullable)thruID;
//@property (nonatomic, readonly) CMProxyConnection * dataModel;

//-(id)initWithFrame:(NSRect)frame andActiveDevice:(CMHardwareDevice* _Nullable)device;


@property (nonatomic, readonly) NSButton* cancelButton;

@end

NS_ASSUME_NONNULL_END
