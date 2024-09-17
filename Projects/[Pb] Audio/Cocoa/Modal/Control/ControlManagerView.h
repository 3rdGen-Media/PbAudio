//
//  ControlManagerModalView.h
//  MCUCommander
//
//  Created by Joe Moulton on 12/6/23.
//


#import <TargetConditionals.h>
#import <Cocoa/Cocoa.h>
//#import "CocoaTextField.h"

#import "CMSoftThruConnection.h"
#import "CMHardwareDevice.h"

#if TARGET_OS_OSX
#define CocoaTextField NSTextField
#else
#import <UIKit/UITextField.h>
#define CocoaTextField UITextField
#endif

//#import "NSCheckboxView.h"

NS_ASSUME_NONNULL_BEGIN

typedef enum ControlViewModes
{
    ModalView_ControlAssign,
    ModalView_ControlDisplay
}ControlViewModes;


@interface ControlManagerView : NSVisualEffectView <NSTextFieldDelegate>

-(void)updateDocumentView;

-(id)initWithFrame:(NSRect)frame andControl:(CMControl*)control;

//@property (nonatomic, readonly) CMThruConnection * dataModel;
//@property (nonatomic, readonly)

@property (nonatomic, readonly) NSButton* cancelButton;


@end

NS_ASSUME_NONNULL_END

