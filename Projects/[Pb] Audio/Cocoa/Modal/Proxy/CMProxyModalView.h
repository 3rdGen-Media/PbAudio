//
//  CMProxyModalView.h
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 12/28/22.
//

#import <TargetConditionals.h>

#import <Cocoa/Cocoa.h>
//#import "CocoaTextField.h"

#import "CMProxyConnection.h"


#if TARGET_OS_OSX
#define CocoaTextField NSTextField
#else
#import <UIKit/UITextField.h>
#define CocoaTextField UITextField
#endif

#import "NSCheckboxView.h"

NS_ASSUME_NONNULL_BEGIN

typedef enum ModalViewModes
{
    ModalView_Create,
    ModalView_Modify
}ModalViewModes;

@interface CMProxyModalView : NSVisualEffectView <NSTextFieldDelegate, NSCheckboxViewDelegate>

-(void)updateDocumentView;

-(id)initWithFrame:(NSRect)frame andProxy:(NSString* _Nullable)thruID;

@property (nonatomic, readonly) CMProxyConnection * dataModel;
//@property (nonatomic, readonly)

@property (nonatomic, readonly) NSButton* cancelButton;


@end

NS_ASSUME_NONNULL_END
