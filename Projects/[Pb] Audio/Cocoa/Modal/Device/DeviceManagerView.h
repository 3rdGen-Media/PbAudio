//
//  DeviceManagerView.h
//  C4Commander
//
//  Created by Joe Moulton on 11/24/23.
//

#import <TargetConditionals.h>

#import <Cocoa/Cocoa.h>
//#import "CocoaTextField.h"

#import "CMSoftThruConnection.h"
#import "CMHardwareDevice.h"

#import "DeviceGroupView.h"

#if TARGET_OS_OSX
#define CocoaTextField NSTextField
#else
#import <UIKit/UITextField.h>
#define CocoaTextField UITextField
#endif

//#import "NSCheckboxView.h"

NS_ASSUME_NONNULL_BEGIN

typedef enum DeviceViewModes
{
    ModalView_Control,
    ModalView_Instruments
}DeviceViewModes;

@interface DeviceManagerView : NSVisualEffectView <NSTextFieldDelegate, NSCollectionViewItemDelegate>

-(void)updateDocumentView;

-(id)initWithFrame:(NSRect)frame andActiveDevice:(CMHardwareDevice* _Nullable)device;

//@property (nonatomic, readonly) CMThruConnection * dataModel;
//@property (nonatomic, readonly)

@property (nonatomic, readonly) NSButton* cancelButton;


@end

NS_ASSUME_NONNULL_END
