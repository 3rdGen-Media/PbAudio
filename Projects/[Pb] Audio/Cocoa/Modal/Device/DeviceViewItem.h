//
//  DeviceViewItem.h
//  MCUCommander
//
//  Created by Joe Moulton on 11/27/23.
//

#import <Cocoa/Cocoa.h>
#import "HighlightImageView.h"

NS_ASSUME_NONNULL_BEGIN

@protocol NSCollectionViewItemDelegate <NSObject>
@optional
-(void)itemClicked:(NSCollectionViewItem*)item;
@end

@interface DeviceViewItem : NSCollectionViewItem

@property (nonatomic, weak) id <NSCollectionViewItemDelegate> delegate;

@property (nonatomic, retain) HighlightImageView* deviceImageView;
@property (nonatomic, retain) NSTextField* deviceLabel;

@end

NS_ASSUME_NONNULL_END
