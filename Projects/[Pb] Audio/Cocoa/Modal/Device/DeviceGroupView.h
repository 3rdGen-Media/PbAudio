//
//  DeviceGroupView.h
//  MCUCommander
//
//  Created by Joe Moulton on 11/27/23.
//

#import <Cocoa/Cocoa.h>
#import "DeviceViewItem.h"

NS_ASSUME_NONNULL_BEGIN

@interface DeviceGroupView : NSCollectionView <NSCollectionViewDelegate, NSCollectionViewDataSource, NSCollectionViewItemDelegate>

@property (nonatomic, weak) id <NSCollectionViewItemDelegate> itemDelegate;

@end

NS_ASSUME_NONNULL_END
