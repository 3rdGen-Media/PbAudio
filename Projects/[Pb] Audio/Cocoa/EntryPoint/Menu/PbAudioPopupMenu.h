//
//  MCUCommanderMainMenu.h
//  MCUCommander
//
//  Created by Joe Moulton on 12/1/23.
//

#import <Cocoa/Cocoa.h>

NS_ASSUME_NONNULL_BEGIN


@interface PbAudioPopupMenu : NSMenu <NSMenuItemValidation>

+ (id)sharedInstance;

@end

NS_ASSUME_NONNULL_END
