//
//  DeviceGroupView.m
//  MCUCommander
//
//  Created by Joe Moulton on 11/27/23.
//

#import "DeviceGroupView.h"

#import "CMSoftThruConnection.h"
#import "CMHardwareDevice.h"
#import "PbAudioAppDelegate.h"



@implementation DeviceGroupView

CGFloat inset = 20;

- (void)collectionView:(NSCollectionView *)collectionView didSelectItemsAtIndexPaths:(NSSet<NSIndexPath *> *)indexPaths
{
    
    //DeviceViewItem* deviceViewItem = [collectionView makeItemWithIdentifier:@"DeviceViewItem" forIndexPath:indexPath];
    //[item setSelected:true];
}



- (void)registerClass:(Class)itemClass forItemWithIdentifier:(NSUserInterfaceItemIdentifier)identifier
{
    //NSString * identifier =  @"Undefined";
    //[self registerClass:itemClass forItemWithIdentifier:identifier];
    //[self registerClass:itemClass forItemWithIdentifier:identifier];
}


- (NSCollectionViewItem *)collectionView:(NSCollectionView *)collectionView itemForRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath
{
    
    NSString * identifier =  @"Undefined";
    NSString * label      =  @"New Device";
    NSString * imageNameString = nil;
    NSImage  * image = nil;
    if( CMDevice.keys.count > 0 && indexPath.item < CMDevice.keys.count )
    {
        identifier = [CMDevice.keys objectAtIndex:indexPath.item];
        
        //find the corresponding device label associated with this device type
        
        CMHardwareDevice * device  = [CMDevice.dictionary objectForKey:identifier];
        label = [NSString stringWithFormat:@"%s", cm_device_names[device.deviceDescription->type]];
        imageNameString = [NSString stringWithFormat:@"%s", cm_device_image_names[device.deviceDescription->type]];
        image = [NSImage imageNamed:imageNameString];
    }
    
    DeviceViewItem* deviceViewItem = [collectionView makeItemWithIdentifier:@"DeviceViewItem" forIndexPath:indexPath];
    deviceViewItem.delegate = self;
    deviceViewItem.deviceLabel.stringValue = label;
    deviceViewItem.deviceLabel.maximumNumberOfLines = 2;
    if( image) deviceViewItem.deviceImageView.image = image;
    return deviceViewItem;
}

-(void)itemClicked:(NSCollectionViewItem*)item
{
    //NSLog(@"DeviceGroupView::itemClicked:");
    if( self.itemDelegate && [self.itemDelegate respondsToSelector:@selector(itemClicked:)] )
    {
        //forward up the delegate chain
        [self.itemDelegate itemClicked:item];
    }
}


- (void)collectionView:(NSCollectionView *)collectionView willDisplayItem:(NSCollectionViewItem *)item forRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath
{

}

- (void)collectionView:(NSCollectionView *)collectionView didEndDisplayingItem:(NSCollectionViewItem *)item forRepresentedObjectAtIndexPath:(NSIndexPath *)indexPath
{
    
    
}

- (NSSize)collectionView:(NSCollectionView *)collectionView layout:(NSCollectionViewLayout *)collectionViewLayout sizeForItemAtIndexPath:(NSIndexPath *)indexPath
{
    //CGFloat aspect = 1556./1071.;//1.69;
    //CGFloat height = self.frame.size.height - (inset * 2.0) ;
    //CGFloat width = height/aspect;//self.frame.size.height;

    CGFloat width = self.frame.size.width / 4.0;
    CGFloat height = self.frame.size.height - (inset) ;

    return NSMakeSize(width,height);
}


- (NSInteger)collectionView:(NSCollectionView *)collectionView numberOfItemsInSection:(NSInteger)section
{
    return CMDevice.keys.count + 1;
    
}

- (NSInteger)numberOfSectionsInCollectionView:(NSCollectionView *)collectionView
{
    return 1;
}

- (id)initWithFrame:(NSRect)frame {

    self = [super initWithFrame:frame];
    if (self) {
        // setup the initial properties of the
        // draggable item
        
        //[self setWantsLayer:NO];
        self.layer.borderWidth = 0.0f;
        self.layer.masksToBounds = YES;
        [self.layer setBorderWidth:0];
        //self.appearance = [NSAppearance appearanceNamed: NSAppearanceNameDarkAqua];
        //self.wantsLayer = YES;
        //self.layer.backgroundColor = [NSColor colorWithRed:164./255. green:186./255. blue:202./255. alpha:1.0].CGColor;
        //self.layer.backgroundColor = [NSColor colorWithRed:255./255. green:0 blue:0 alpha:1.0].CGColor;

        
        self.delegate = self;
        self.dataSource = self;//dataSource; ??
        
        //[self.dataSource ];
        //[self reloadData];
        
        [self registerClass:[DeviceViewItem class] forItemWithIdentifier:@"DeviceViewItem"];

        self.allowsEmptySelection = NO;
        self.allowsMultipleSelection = NO;
        
        NSCollectionViewFlowLayout * flowLayout = [[NSCollectionViewFlowLayout alloc] init];
        flowLayout.minimumInteritemSpacing = 30.0;
        flowLayout.minimumLineSpacing = 30.0;
        flowLayout.sectionInset = NSEdgeInsetsMake( 20.0, 20.0, 0.0, 20.0);
        self.collectionViewLayout = flowLayout;

        //nscollectionview mouse clicks seem to stall the app so we have to move the gesture interaction to the collectionviewitem manually
        //self.selectable = YES;
        
        //NSSet* indexSet = [[NSSet alloc] initWithArray:@[[NSIndexPath indexPathForItem:0 inSection:0]]];
        //[self selectItemsAtIndexPaths:indexSet scrollPosition:NSCollectionViewScrollPositionNearestHorizontalEdge];
        
    }
    return self;
}


/*
- (void)drawRect:(NSRect)rect
{
    //[NSGraphicsContext saveGraphicsState];
    [super drawRect:rect];
    
    NSPoint center = NSMakePoint(self.bounds.size.width/2., self.bounds.size.height/2.);

    // erase the background by drawing white
    [[NSColor redColor] set];
    [NSBezierPath fillRect:[self bounds]];
    
    // Drawing code here.
}
*/

@end
