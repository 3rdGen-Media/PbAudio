//
//  DeviceManagerView.m
//  C4Commander
//
//  Created by Joe Moulton on 11/24/23.
//

#import "DeviceManagerView.h"
#import "PbAudioAppInterface.h"

#import <Foundation/NSObjcRuntime.h>
#import <objc/runtime.h>

#import "PbAudioAppDelegate.h"
//#import "CMThruScrollListView.h"

#import "CustomIndicatorView.h"
#import "NSTextFieldButton.h"

//#import "NSCheckboxView.h"

@interface DeviceManagerView()
{
    
    //CMThruScrollListView * _scrollListView;
    NSSegmentedControl * _radioControl;
    DeviceGroupView    * _deviceGroupView;
    
    NSTextField *_descriptionLabel;
    NSTextField *_inputDeviceLabel;
    NSTextField *_outputDeviceLabel;
    NSTextField *_surfaceTypeLabel;

    NSMutableDictionary * _inputFields;
    NSMutableDictionary * _fieldButtons;
    
    CocoaTextField *_csvField;
    CocoaTextField *_dirField;
    
    NSButton * _submitButton;
    NSButton * _deleteButton;
    NSButton * _cancelButton;
    
    volatile bool _userInteractionEnabled;
}


//@property (nonatomic, retain) NSImageView * imageView;
//@property (nonatomic, retain) CMLogoView * imageView;

@property (nonatomic, retain) NSSegmentedControl * radioControl;
@property (nonatomic, retain) DeviceGroupView    * deviceGroupView;

//Labels
@property (nonatomic, retain) NSTextField * descriptionLabel;
@property (nonatomic, retain) NSTextField * inputDeviceLabel;
@property (nonatomic, retain) NSTextField * outputDeviceLabel;
@property (nonatomic, retain) NSTextField * surfaceTypeLabel;
@property (nonatomic, retain) NSTextField * incrementLabel;

//TextFields
@property (nonatomic, retain) NSPopUpButton * inputPopupButton;
@property (nonatomic, retain) NSPopUpButton * outputPopupButton;
@property (nonatomic, retain) NSPopUpButton * surfaceTypePopupButton;

//@property (nonatomic, retain) NSCheckboxView * filterView;
@property (nonatomic, retain) NSTextField      * incrementField;

//Toolbar
@property (nonatomic, retain) NSView * bottomToolbar;


@property (nonatomic, retain) NSScrollView * scrollView;
@property (nonatomic, retain) NSView * scrollDocumentView;

@property (nonatomic) int modalViewMode;
@property (nonatomic) int type;

@property (nonatomic, retain) CMSoftThruConnection * dataModel;

@property (atomic) bool userInteractionEnabled;


@property (nonatomic, retain) NSMutableArray * inputFieldKeys;
@property (nonatomic, retain) NSMutableDictionary *inputFields;
@property (nonatomic, retain) NSMutableDictionary *fieldButtons;

@property (nonatomic, retain) NSMutableArray * modalButtons;

@property (nonatomic, retain) NSButton * submitButton;
@property (nonatomic, retain) NSButton * deleteButton;

@property (nonatomic, retain) NSButton * helpButton;

@property (nonatomic, retain) NSButton * cancelButton;

@property (nonatomic, retain) CustomIndicatorView * indicatorView;

@end

@implementation DeviceManagerView

@synthesize deviceGroupView = _deviceGroupView;

@synthesize descriptionLabel = _descriptionLabel;
@synthesize inputDeviceLabel = _inputDeviceLabel;
@synthesize outputDeviceLabel = _outputDeviceLabel;
@synthesize surfaceTypeLabel = _surfaceTypeLabel;

@synthesize inputFields = _inputFields;

@synthesize cancelButton = _cancelButton;




-(void)createRadioControl
{
    CGFloat controlHeight = 36.;
    
    //self.imageView = [[NSImageView alloc] initWithFrame:CGRectMake(0,0,150, 150)];
    //self.imageView.image = [NSImage imageNamed:NSImageNameNetwork];
    
    
    NSArray *itemArray = [NSArray arrayWithObjects: @"Control C4", @"Instruments", nil];
    self.radioControl = [NSSegmentedControl segmentedControlWithLabels:itemArray trackingMode:NSSegmentSwitchTrackingSelectOne target:self action:nil];
    //self.radioControl.segmentStyle = NSSegmentedControlStyle
    self.radioControl.selectedSegment = 0;
    //self.radioControl = [[NSSegmentedControl alloc] initWithFrame:CGRectMake(0,0,self.frame.size.width/2., controlHeight)];1
    
    self.radioControl.wantsLayer = YES;
    self.radioControl.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.imageView.layer.cornerRadius = self.imageView.frame.size.width/2.0;
    
    
    [self addSubview:self.radioControl];
    
    
    self.radioControl.translatesAutoresizingMaskIntoConstraints = NO;
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem:self.radioControl
                                                                attribute: NSLayoutAttributeTop
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeTop
                                                                multiplier:1.0f constant:14];

    NSLayoutConstraint * CenterX = [NSLayoutConstraint constraintWithItem:self.radioControl
                                                                attribute: NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeCenterX
                                                                 multiplier:1.0 constant:0.0f];

    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem:self.radioControl
                                                                attribute: NSLayoutAttributeWidth
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeWidth
                                                                 multiplier:0.5 constant:0.0f];
    
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem:self.radioControl
                                                                attribute: NSLayoutAttributeHeight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeHeight
                                                              multiplier:0 constant:controlHeight];
    [self addConstraints:@[ Top, CenterX, Width, Height]];
    
}

-(void)createDeviceCollectionView
{
    self.deviceGroupView = [[DeviceGroupView alloc] initWithFrame:CGRectZero];
    self.deviceGroupView.itemDelegate = self;
    //[self.deviceGroupView setContent:@[@"New Device"]];

    //collectionView.enclosingScrollView?.borderType = .noBorder

    
    [self addSubview:self.deviceGroupView];
    
    
    self.deviceGroupView.translatesAutoresizingMaskIntoConstraints = NO;
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem:self.deviceGroupView
                                                                attribute: NSLayoutAttributeTop
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeTop
                                                                multiplier:1.0f constant:PbAudioAppDelegate.sharedInstance.window.contentView.safeAreaInsets.top];

    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem:self.deviceGroupView
                                                                attribute: NSLayoutAttributeLeft
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeLeft
                                                                 multiplier:1.0 constant:0.0f];

    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem:self.deviceGroupView
                                                                attribute: NSLayoutAttributeRight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeRight
                                                                 multiplier:1.0 constant:0.0f];
    
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem:self.deviceGroupView
                                                                attribute: NSLayoutAttributeHeight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeHeight
                                                              multiplier:0.375 constant:0];
    [self addConstraints:@[ Top, Left, Right, Height]];
    
}

-(void)setInputFieldsForDevice:(CMHardwareDevice*)device
{
    CMSoftThruConnection * ThruConnection = [CMSoftThru.dictionary objectForKey:device.primaryKey];
    assert(ThruConnection);
    //find the index of the popup button midi source input
    int srcIndex = 0;
    int dstIndex = 0;
    for( srcIndex = 0; srcIndex < CMClient.numSources; srcIndex++)
    {
        if( CMClient.sources[srcIndex].uniqueID == ThruConnection.connection->source.uniqueID) break;
    }
    for( dstIndex = 0; dstIndex < CMClient.numDestinations; dstIndex++)
    {
        if( CMClient.destinations[dstIndex].uniqueID == ThruConnection.connection->destination.uniqueID) break;
    }
    
    
    if( srcIndex >= CMClient.numSources ) srcIndex = 0;
    if( dstIndex >= CMClient.numDestinations ) dstIndex = 0;

    [self.inputPopupButton selectItemAtIndex:srcIndex];
    [self.outputPopupButton selectItemAtIndex:dstIndex];
    [self.surfaceTypePopupButton selectItemAtIndex:device.deviceDescription->type];
    
    //[self.inputPopupButton setNeedsDisplay:YES];
    //[self.outputPopupButton setNeedsDisplay:YES];
    //[self.surfaceTypePopupButton setNeedsDisplay:YES];
}

-(void)modifyDeviceWithInputFields:(CMHardwareDevice*)device
{
    //Find the fields that have changed
    
    CMSoftThruConnection * ThruConnection = [CMSoftThru.dictionary objectForKey:device.primaryKey];
    assert(ThruConnection);
    //find the index of the popup button midi source input
    int srcIndex = (int)self.inputPopupButton.indexOfSelectedItem;
    int dstIndex = (int)self.outputPopupButton.indexOfSelectedItem;
    int surfaceType = (int)self.surfaceTypePopupButton.indexOfSelectedItem;

    if( ThruConnection.connection->source.uniqueID != CMClient.sources[srcIndex].uniqueID )
    {
        NSLog(@"DeviceManagerView::modifyDeviceWithInputFields() device source has changed!!!");
        assert(1==0);
    }
    else if( ThruConnection.connection->destination.uniqueID != CMClient.destinations[dstIndex].uniqueID )
    {
        NSLog(@"DeviceManagerView::modifyDeviceWithInputFields() device destnation has changed!!!");
        assert(1==0);
    }
    else if( device.deviceDescription->type != surfaceType )
    {
        NSLog(@"DeviceManagerView::modifyDeviceWithInputFields() device surface type has changed!!!");
        assert(1==0);
    }
    else
    {
        NSLog(@"DeviceManagerView::modifyDeviceWithInputFields() no device changes.");
    }

}

-(void)itemClicked:(NSCollectionViewItem*)item
{
    NSLog(@"DeviceManagerView::itemClicked:");
    
    //get the previously selected item(s)
    //NSIndexSet* indexSet = [self.deviceGroupView selectionIndexes];
    NSSet * selectedIndexPaths = [self.deviceGroupView selectionIndexPaths];
    //NSCollectionViewItem* selectedItem = nil;
    
    NSIndexPath* indexPath = [self.deviceGroupView indexPathForItem:item];
    NSIndexPath* selectedIndexPath = nil;
    
    if( selectedIndexPaths.count > 0 )
    {
        assert(selectedIndexPaths.count == 1); //we only allow one selected item at a time in this app for now
        selectedIndexPath = selectedIndexPaths.allObjects.firstObject;
    }
    
    if( indexPath != selectedIndexPath ) //we have a new valid item to select
    {
        //unhighlight selected item
        if( selectedIndexPath )
        {
            [self.deviceGroupView deselectItemsAtIndexPaths:selectedIndexPaths];
        }
        
        //highlight selected item
        selectedIndexPaths = [[NSSet alloc] initWithArray:@[indexPath]];
        [self.deviceGroupView selectItemsAtIndexPaths:selectedIndexPaths scrollPosition:NSCollectionViewScrollPositionNearestHorizontalEdge];

        //get the index of the selected item
        //indexPath.
        //id obj = [item representedObject];
        //NSInteger index = [[self.deviceGroupView contents] indexOfObject:obj];
        NSInteger index = indexPath.item;//[[self.deviceGroupView content] indexOfObject:[item representedObject]];
        
        if( index >= CMDevice.documents.count ) // new device
        {
            //clear input fields
            [self.inputPopupButton selectItemAtIndex:0];
            [self.outputPopupButton selectItemAtIndex:0];
            [self.surfaceTypePopupButton selectItemAtIndex:0];
            self.incrementField.stringValue = @"1";
        }
        else
        {
            //find selected device
            CMHardwareDevice * newActiveDevice = [CMDevice.documents objectAtIndex:index];
            //CMSoftThruConnection * ThruConnection = [CMSoftThru.dictionary objectForKey:newActiveDevice.primaryKey];
            //[[PbAudioAppDelegate sharedInstance] setActiveDevice:newActiveDevice];
            [self setInputFieldsForDevice:newActiveDevice];
        }

    }
    
}

//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createDescriptionLabel
{
    self.descriptionLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.descriptionLabel.focusRingType = NSFocusRingTypeDefault;
    self.descriptionLabel.stringValue  = @"Configure your Control Devices.";
    //_descriptionLabel.font = [NSFont systemFontOfSize:36.0];
    self.descriptionLabel.wantsLayer = YES;
    //self.descriptionLabel.bordered = YES;
    
    //self.descriptionLabel.enabled = YES;
    self.descriptionLabel.editable = NO;
    self.descriptionLabel.drawsBackground = YES;
    self.descriptionLabel.layer.masksToBounds = NO;
    
    //self.descriptionLabel.usesSingleLineMode = NO;
    //self.descriptionLabel.maximumNumberOfLines = 1;
    self.descriptionLabel.textColor = NSColor.whiteColor;
    self.descriptionLabel.backgroundColor = [NSColor clearColor];
    self.descriptionLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.descriptionLabel.alignment = NSTextAlignmentLeft;
    
    _descriptionLabel.bezeled         = NO;
    _descriptionLabel.editable        = NO;
    _descriptionLabel.drawsBackground = NO;
    
    [self addSubview:self.descriptionLabel];
    

    self.descriptionLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.descriptionLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:14.f];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.descriptionLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.deviceGroupView attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:14.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.descriptionLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self attribute:NSLayoutAttributeWidth
                                                             multiplier:1. constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createInputDeviceLabel
{
    self.inputDeviceLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.inputDeviceLabel.focusRingType = NSFocusRingTypeDefault;
    self.inputDeviceLabel.stringValue  = @"Surface Type:";
    self.inputDeviceLabel.wantsLayer = YES;
    
    self.inputDeviceLabel.editable = YES;
    self.inputDeviceLabel.drawsBackground = YES;
    self.inputDeviceLabel.layer.masksToBounds = NO;

    self.inputDeviceLabel.textColor = NSColor.whiteColor;
    self.inputDeviceLabel.backgroundColor = [NSColor clearColor];
    self.inputDeviceLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.inputDeviceLabel.alignment = NSTextAlignmentLeft;
    
    _inputDeviceLabel.bezeled         = NO;
    _inputDeviceLabel.editable        = NO;
    _inputDeviceLabel.drawsBackground = NO;
    
    [self addSubview:self.inputDeviceLabel];
    [self.inputDeviceLabel sizeToFit];

    CGRect frame = self.inputDeviceLabel.frame;

    self.inputDeviceLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.inputDeviceLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.inputDeviceLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.inputDeviceLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.descriptionLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:0 constant:frame.size.width];

    
    [self addConstraints:@[ Left, Top, Width ]];
    
        
    self.inputDeviceLabel.stringValue  = @"MCU Input:";
    //self.inputDeviceLabel.frame =  frame;
}


//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createOutputDeviceLabel
{
    self.outputDeviceLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.outputDeviceLabel.focusRingType = NSFocusRingTypeDefault;
    self.outputDeviceLabel.stringValue  = @"MCU Return:";
    self.outputDeviceLabel.wantsLayer = YES;
    
    //self.outputDeviceLabel.lineBreakMode = NSLineBreakByClipping;
    //self.outputDeviceLabel.usesSingleLineMode = YES;
    //self.outputDeviceLabel.maximumNumberOfLines = YES;
    
    self.outputDeviceLabel.editable = YES;
    self.outputDeviceLabel.drawsBackground = YES;
    self.outputDeviceLabel.layer.masksToBounds = NO;
    
    self.outputDeviceLabel.textColor = NSColor.whiteColor;
    self.outputDeviceLabel.backgroundColor = [NSColor clearColor];
    self.outputDeviceLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.outputDeviceLabel.alignment = NSTextAlignmentLeft;

    _outputDeviceLabel.bezeled         = NO;
    _outputDeviceLabel.editable        = NO;
    _outputDeviceLabel.drawsBackground = NO;
    
    [self addSubview:self.outputDeviceLabel];
    [self.outputDeviceLabel sizeToFit];
    //CGRect frame = self.outputDeviceLabel.frame;


    self.outputDeviceLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.outputDeviceLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.outputDeviceLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.inputDeviceLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.outputDeviceLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.inputDeviceLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
}

/*
-(void)createThruIDTextField
{
    self.thruIDField = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.thruIDField.focusRingType = NSFocusRingTypeDefault;
    self.thruIDField.stringValue  = self.dataModel.ThruID ? self.dataModel.ThruID : @"";
    self.thruIDField.placeholderString = @"Enter ThruID";
    //_descriptionLabel.font = [NSFont systemFontOfSize:36.0];
    self.thruIDField.wantsLayer = YES;
    //self.descriptionLabel.bordered = YES;
    
    //self.descriptionLabel.enabled = YES;
    self.thruIDField.editable = YES;
    self.thruIDField.drawsBackground = YES;
    self.thruIDField.layer.masksToBounds = NO;
    
    self.thruIDField.textColor = NSColor.blackColor;
    //self.thruIDField.backgroundColor = [NSColor clearColor];
    //self.thruIDField.layer.backgroundColor = [NSColor clearColor].CGColor;
    self.thruIDField.delegate = self;
    self.thruIDField.alignment = NSTextAlignmentLeft;
    
    _thruIDField.bezeled         = YES;
    _thruIDField.editable        = YES;
    _thruIDField.drawsBackground = YES;
    
    [self addSubview:self.thruIDField];
    

    self.thruIDField.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.thruIDField
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.thruIDField
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.thruIDField
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.descriptionLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}
*/

-(void)devicePopupButtonClicked:(id)sender
{
    //NSLog(@"inputPopupButton select: %@", sender.)
    
    //if the string associated with the selected index does not equal our existing string
    
    //assert( self.modalViewMode == ModalView_Control );
    
    {
        if( sender == self.inputPopupButton )
        {
            //int selectedIndex = (int)self.inputPopupButton.indexOfSelectedItem;
            //NSString * inputDeviceName = [NSString stringWithUTF8String:CMClient.sources[selectedIndex].name];
            //if( [_dataModel.Input localizedCompare:inputDeviceName] != NSOrderedSame ) self.submitButton.enabled = YES;

        }
        else if( sender == self.outputPopupButton )
        {
            //int selectedIndex = (int)self.outputPopupButton.indexOfSelectedItem;
            //NSString * deviceName = [NSString stringWithUTF8String:CMClient.destinations[selectedIndex].name];
            //if( [_dataModel.Output localizedCompare:deviceName] != NSOrderedSame ) self.submitButton.enabled = YES;
        }
        else if( sender == self.surfaceTypePopupButton )
        {
            //int selectedIndex = (int)self.surfaceTypePopupButton.indexOfSelectedItem;
            //NSString * surfaceTypeName = [NSString stringWithUTF8String:cm_device_names[selectedIndex]];
            //if( [_dataModel.Output localizedCompare:deviceName] != NSOrderedSame ) self.submitButton.enabled = YES;
        }
        
    }
}


-(void)createInputPopupButton
{
    self.inputPopupButton = [[NSPopUpButton alloc] initWithFrame:CGRectZero pullsDown:NO];
    self.inputPopupButton.enabled = YES;
    
    [self.inputPopupButton setAction:@selector(devicePopupButtonClicked:)];
    self.inputPopupButton.target = self; //without this popup items will appear grayed out if no other ui to make first responder
    int srcIndex = 0;
    for( int i = 0; i < CMClient.numSources; i++)
    {
        [self.inputPopupButton addItemWithTitle:[NSString stringWithUTF8String:CMClient.sources[i].name]];
        //if( strcmp(_dataModel.connection->source.name, CMClient.sources[i].name) == 0 ) srcIndex = i;
    }
    
    if( self.modalViewMode == ModalView_Control) [self.inputPopupButton selectItemAtIndex:srcIndex];
    [self addSubview:self.inputPopupButton];
    

    self.inputPopupButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.inputPopupButton
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.inputDeviceLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.inputPopupButton
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.inputPopupButton
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.inputDeviceLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}



-(void)createOutputPopupButton
{
    self.outputPopupButton = [[NSPopUpButton alloc] initWithFrame:CGRectZero pullsDown:NO];
    self.outputPopupButton.enabled = YES;
    [self.outputPopupButton setAction:@selector(devicePopupButtonClicked:)];
    self.outputPopupButton.target = self; //without this popup items will appear grayed out if no other ui to make first responder

    int dstIndex=0;
    for( int i = 0; i < CMClient.numDestinations; i++)
    {
        [self.outputPopupButton addItemWithTitle:[NSString stringWithUTF8String:CMClient.destinations[i].name]];
        //if(strcmp(_dataModel.connection->destination.name, CMClient.destinations[i].name) == 0 )dstIndex = i;
    }
    
    if( self.modalViewMode == ModalView_Control) [self.outputPopupButton selectItemAtIndex:dstIndex];
    else if( self.outputPopupButton.numberOfItems > 1 ) [self.outputPopupButton selectItemAtIndex:1];

    [self addSubview:self.outputPopupButton];
    

    self.outputPopupButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.outputPopupButton
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.outputPopupButton
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.outputPopupButton
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}


-(void)createSurfaceTypeLabel
{
    self.surfaceTypeLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.surfaceTypeLabel.focusRingType = NSFocusRingTypeDefault;
    self.surfaceTypeLabel.stringValue  = @"Surface Type:";
    self.surfaceTypeLabel.wantsLayer = YES;
    
    self.surfaceTypeLabel.editable = YES;
    self.surfaceTypeLabel.drawsBackground = YES;
    self.surfaceTypeLabel.layer.masksToBounds = NO;
    
    self.surfaceTypeLabel.textColor = NSColor.whiteColor;
    self.surfaceTypeLabel.backgroundColor = [NSColor clearColor];
    self.surfaceTypeLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.surfaceTypeLabel.delegate = self;
    self.surfaceTypeLabel.alignment = NSTextAlignmentLeft;

    self.surfaceTypeLabel.bezeled         = NO;
    self.surfaceTypeLabel.editable        = NO;
    self.surfaceTypeLabel.drawsBackground = NO;
    
    [self addSubview:self.surfaceTypeLabel];
    [self.surfaceTypeLabel sizeToFit];

    //CGRect frame = self.surfaceTypeLabel.frame;

    self.surfaceTypeLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.surfaceTypeLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.surfaceTypeLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.surfaceTypeLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.outputDeviceLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
    
        
    //self.inputDeviceLabel.stringValue  = @"Input Device:";

}



-(void)createSurfaceTypePopupButton
{
    self.surfaceTypePopupButton = [[NSPopUpButton alloc] initWithFrame:CGRectZero pullsDown:NO];
    self.surfaceTypePopupButton.enabled = YES;
    [self.surfaceTypePopupButton setAction:@selector(devicePopupButtonClicked:)];
    self.surfaceTypePopupButton.target = self; //without this popup items will appear grayed out if no other ui to make first responder

    
    int dstIndex=0;
    for( int i = 0; i < cm_device_list_size; i++)
    {
        [self.surfaceTypePopupButton addItemWithTitle:[NSString stringWithUTF8String:cm_device_names[i]]];
        //if(strcmp(_dataModel.connection->destination.name, CMClient.destinations[i].name) == 0 )dstIndex = i;
    }
    
    if( self.modalViewMode == ModalView_Control) [self.surfaceTypePopupButton selectItemAtIndex:dstIndex];
    else if( self.surfaceTypePopupButton.numberOfItems > 1 ) [self.surfaceTypePopupButton selectItemAtIndex:1];

    [self addSubview:self.surfaceTypePopupButton];
    

    self.surfaceTypePopupButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.surfaceTypePopupButton
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.surfaceTypeLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.surfaceTypePopupButton
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.surfaceTypePopupButton
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.surfaceTypeLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}


-(void)createIncrementLabel
{
    self.incrementLabel = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
    self.incrementLabel.focusRingType = NSFocusRingTypeDefault;
    self.incrementLabel.stringValue  = @"Shift Increment: ";
    self.incrementLabel.wantsLayer = YES;
    
    self.incrementLabel.editable = YES;
    self.incrementLabel.drawsBackground = YES;
    self.incrementLabel.layer.masksToBounds = NO;
    
    self.incrementLabel.textColor = NSColor.whiteColor;
    self.incrementLabel.backgroundColor = [NSColor clearColor];
    self.incrementLabel.layer.backgroundColor = [NSColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    self.incrementLabel.alignment = NSTextAlignmentLeft;

    self.incrementLabel.bezeled         = NO;
    self.incrementLabel.editable        = NO;
    self.incrementLabel.drawsBackground = NO;
    
    [self addSubview:self.incrementLabel];
    [self.incrementLabel sizeToFit];

    self.incrementLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.incrementLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.surfaceTypeLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.incrementLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.surfaceTypeLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.];
    
    /*
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.incrementLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.outputDeviceLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];
     */
    
    [self addConstraints:@[ Left, Top]];
}


-(NSTextField*)createTextField:(NSString*)string placeholder:(NSString*)placeholder
{
  NSTextField* textField = [[NSTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
  //[textField setCell: [[CMTextFieldCell alloc] init]];

  textField.focusRingType = NSFocusRingTypeDefault;
  textField.stringValue  = @"";//@"TBD";
  textField.wantsLayer = NO;
  textField.placeholderString = placeholder;//@"Enter Value [1-127]";
  textField.editable = YES;
  textField.drawsBackground = NO;
  textField.layer.masksToBounds = NO;
    
  textField.textColor = NSColor.whiteColor;
  textField.backgroundColor = [NSColor whiteColor];
  textField.layer.backgroundColor = [NSColor whiteColor].CGColor;
  textField.delegate = self;
  textField.alignment = NSTextAlignmentLeft;

  textField.bezeled         = YES;
  textField.editable        = YES;
  textField.drawsBackground = NO;
    
   
    [textField sizeToFit];

    return textField;
}

-(void)createIncrementField
{
    self.incrementField = [self createTextField:@"" placeholder:@"Enter Value [1-127]"];//
    
    [self addSubview:self.incrementField];
    [self.incrementField sizeToFit];

    self.incrementField.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.incrementField
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.incrementLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:14.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.incrementField
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.incrementField
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.incrementLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
    
    self.incrementField.enabled = YES;
}

/*
-(void)buttonToggledAtIndex:(int)index;
{
    NSLog(@"CMThruModalView::buttonToggledAtIndex(%d)", index);
    if( self.modalViewMode == ModalView_Modify ) self.submitButton.enabled = YES;
}

-(void) createFilterView
{
    NSArray * filterIdentifiers = @[@"CC", @"SysEx", @"MTC", @"Beat Clock", @"Tune Request"];
    self.filterView = [[NSCheckboxView alloc] initWithIdentifiers:filterIdentifiers Title:@"Filter:  " Justification:NSTextAlignmentRight];
    self.filterView.delegate = self;
    
    if( self.modalViewMode == ModalView_Modify )
    {
        //for( NSButton * button in self.filterView.buttons ) button.enabled = NO;
        if( _dataModel.connection->params.filterOutAllControls ) ((NSButton*)[self.filterView.buttons objectAtIndex:0]).state = NSControlStateValueOn;
        if( _dataModel.connection->params.filterOutSysEx ) ((NSButton*)[self.filterView.buttons objectAtIndex:1]).state = NSControlStateValueOn;
        if( _dataModel.connection->params.filterOutMTC ) ((NSButton*)[self.filterView.buttons objectAtIndex:2]).state = NSControlStateValueOn;
        if( _dataModel.connection->params.filterOutBeatClock ) ((NSButton*)[self.filterView.buttons objectAtIndex:3]).state = NSControlStateValueOn;
        if( _dataModel.connection->params.filterOutTuneRequest ) ((NSButton*)[self.filterView.buttons objectAtIndex:4]).state = NSControlStateValueOn;
    }
    
    [self addSubview:self.filterView];
    self.filterView.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.filterView
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.filterView
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:-14.f];
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.filterView
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:21.f];
    
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem: self.filterView
                                                                attribute: NSLayoutAttributeHeight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputPopupButton attribute:NSLayoutAttributeHeight
                                                               multiplier:1.0 constant:0.f];

    
    [self addConstraints:@[ Top, Left, Right, Height ]];
    
}
*/

-(void)createBottomToolbar
{
    
    self.bottomToolbar = [[NSView alloc] initWithFrame:CGRectMake(0,0, self.frame.size.width, 50)];
    
    self.bottomToolbar.wantsLayer = YES;
    self.bottomToolbar.layer.backgroundColor = [NSColor colorWithWhite:1.0 alpha:0.2].CGColor;
    
    //self.bottomToolbar.superview.wantsLayer = true
    self.bottomToolbar.shadow = [[NSShadow alloc] init];
    //self.bottomToolbar.layer.backgroundColor = [NSColor redColor].CGColor;
    //self.bottomToolbar.layer.cornerRadius = 5.0;
    self.bottomToolbar.layer.shadowOpacity = 1.0;
    self.bottomToolbar.layer.shadowColor = [NSColor colorWithWhite:0.3 alpha:1.0].CGColor;
    self.bottomToolbar.layer.shadowOffset = NSMakeSize(0, 0);
    self.bottomToolbar.layer.shadowRadius = 10;

    self.bottomToolbar.layer.borderWidth = 0.25;
    self.bottomToolbar.layer.borderColor = [NSColor colorWithWhite:0.7 alpha:1.0].CGColor;
    
    [self addSubview:self.bottomToolbar];
    
    //self.bottomToolbar.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Bottom = [NSLayoutConstraint constraintWithItem: self.bottomToolbar
                                                                attribute: NSLayoutAttributeBottom
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeBottom
                                                               multiplier:1 constant:0];
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.bottomToolbar
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0];

    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.bottomToolbar
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:0];

  

    
    [self addConstraints:@[ Bottom, Left, Right ]];
}


-(void)createDeleteButton
{
    self.deleteButton = [[NSButton alloc] initWithFrame:CGRectMake(0,0,80,30)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    self.deleteButton.enabled = NO;
    
    [self.deleteButton setTitle:@"Delete"];
    [self.deleteButton setTarget:self];
    //[self.deleteButton setAction:@selector(deleteButtonClicked:)];
    
    self.deleteButton.wantsLayer = YES;
    self.deleteButton.layer.backgroundColor = NSColor.clearColor.CGColor;
    self.deleteButton.layer.borderColor = NSColor.clearColor.CGColor;
    self.deleteButton.layer.borderWidth = 0;
    //self.submitButton.layer.cornerRadius = 4.0f;

    self.deleteButton.shadow = [[NSShadow alloc] init];
    //self.bottomToolbar.layer.backgroundColor = [NSColor redColor].CGColor;
    //self.bottomToolbar.layer.cornerRadius = 5.0;
    self.deleteButton.layer.shadowOpacity = 1.0;
    self.deleteButton.layer.shadowColor = [NSColor colorWithWhite:0.3 alpha:1.0].CGColor;
    self.deleteButton.layer.shadowOffset = NSMakeSize(0, 0.5);
    self.deleteButton.layer.shadowRadius = 0.5;

    
    self.deleteButton.contentTintColor = [NSColor colorWithWhite:0.9 alpha:1.0];
    self.deleteButton.bezelStyle = NSBezelStyleRounded;
    
    //self.layer.backgroundColor =
    self.deleteButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.bottomToolbar addSubview:self.deleteButton];
    
    self.deleteButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.deleteButton
                                                           attribute: NSLayoutAttributeRight
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: self.bottomToolbar attribute:NSLayoutAttributeRight
                                                          multiplier:1.0f constant:-16.0f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.deleteButton
                                                            attribute: NSLayoutAttributeCenterY
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.bottomToolbar attribute:NSLayoutAttributeCenterY
                                                           multiplier:1.0 constant:0];
    
    
    [self addConstraints:@[ Right, CenterY]];
    
}

-(void)createSubmitButton
{
    self.submitButton = [[NSButton alloc] initWithFrame:CGRectMake(0,0,80,30)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    self.submitButton.enabled = NO;
    
    [self.submitButton setTitle:@"Apply"];
    [self.submitButton setTarget:self];
    [self.submitButton setAction:@selector(submitButtonClicked:)];
    
    self.submitButton.wantsLayer = YES;
    self.submitButton.layer.backgroundColor = NSColor.clearColor.CGColor;
    self.submitButton.layer.borderColor = NSColor.clearColor.CGColor;
    self.submitButton.layer.borderWidth = 0;
    //self.submitButton.layer.cornerRadius = 4.0f;

    self.submitButton.shadow = [[NSShadow alloc] init];
    //self.bottomToolbar.layer.backgroundColor = [NSColor redColor].CGColor;
    //self.bottomToolbar.layer.cornerRadius = 5.0;
    self.submitButton.layer.shadowOpacity = 1.0;
    self.submitButton.layer.shadowColor = [NSColor colorWithWhite:0.3 alpha:1.0].CGColor;
    self.submitButton.layer.shadowOffset = NSMakeSize(0, 0.5);
    self.submitButton.layer.shadowRadius = 0.5;

    
    self.submitButton.contentTintColor = [NSColor colorWithWhite:0.9 alpha:1.0];
    self.submitButton.bezelStyle = NSBezelStyleRounded;
    
    //self.layer.backgroundColor =
    self.submitButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.bottomToolbar addSubview:self.submitButton];
    
    self.submitButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.submitButton
                                                           attribute: NSLayoutAttributeRight
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: self.deleteButton attribute:NSLayoutAttributeLeft
                                                          multiplier:1.0f constant:-16.0f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.submitButton
                                                            attribute: NSLayoutAttributeCenterY
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.bottomToolbar attribute:NSLayoutAttributeCenterY
                                                           multiplier:1.0 constant:0];
    
    
    [self addConstraints:@[ Right, CenterY]];
    
}



-(void)createHelpButton
{
    self.helpButton = [[NSButton alloc] initWithFrame:CGRectMake(0,0,30,30)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    [self.helpButton setTitle:@""];
    [self.helpButton setTarget:self];
    [self.helpButton setAction:@selector(helpButtonClicked:)];
    
    self.helpButton.wantsLayer = YES;
    self.helpButton.layer.backgroundColor = NSColor.clearColor.CGColor;
    self.helpButton.layer.borderColor = NSColor.clearColor.CGColor;
    self.helpButton.layer.borderWidth = 0;
    //self.submitButton.layer.cornerRadius = 4.0f;

    self.helpButton.shadow = [[NSShadow alloc] init];
    //self.bottomToolbar.layer.backgroundColor = [NSColor redColor].CGColor;
    //self.bottomToolbar.layer.cornerRadius = 5.0;
    self.helpButton.layer.shadowOpacity = 1.0;
    self.helpButton.layer.shadowColor = [NSColor colorWithWhite:0.3 alpha:1.0].CGColor;
    self.helpButton.layer.shadowOffset = NSMakeSize(0, 0.5);
    self.helpButton.layer.shadowRadius = 0.5;

    
    self.helpButton.contentTintColor = [NSColor colorWithWhite:0.9 alpha:1.0];
    self.helpButton.bezelStyle = NSBezelStyleHelpButton;
    
    //self.layer.backgroundColor =
    self.helpButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.bottomToolbar addSubview:self.helpButton];
    
    self.helpButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.helpButton
                                                           attribute: NSLayoutAttributeLeft
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: self.bottomToolbar attribute:NSLayoutAttributeLeft
                                                          multiplier:1.0f constant:16.0f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.helpButton
                                                            attribute: NSLayoutAttributeCenterY
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.bottomToolbar attribute:NSLayoutAttributeCenterY
                                                           multiplier:1.0 constant:0];
    
    [self addConstraints:@[ Left, CenterY]];
    
}

-(id)initWithFrame:(NSRect)frame andActiveDevice:(CMHardwareDevice* _Nullable)device
{
    self = [super initWithFrame:frame];
    if(self)
    {
        self.wantsLayer = YES;
    
        //[self setWantsLayer:NO];
        //self.layer.borderWidth = 0.0f;
        //self.layer.masksToBounds = YES;
        //[self.layer setBorderWidth:0];
        //self.appearance = [NSAppearance appearanceNamed: NSAppearanceNameDarkAqua];

        //create a temporary data model object that the modal table view can use to populate itself and visa versa
        //so the user can create a thru coonnection
        self.modalViewMode = ModalView_Control;
        self.dataModel = CMSoftThru;
        /*
        if( thruID && thruID.length > 0 )
        {
            self.dataModel = [CMThru.dictionary objectForKey:thruID];
            self.modalViewMode = ModalView_Modify;
            assert(self.dataModel);
        }
        else
        {
            self.dataModel = [[CMThruConnection alloc] init];
        }
        */
        
        //[self createScrollListView];
        //[self createScrollView];
        
        //create UI
        
        //[self createRadioControl];
        
        [self createDeviceCollectionView];
        [self createDescriptionLabel];
        
        [self createInputDeviceLabel];
        [self createOutputDeviceLabel];
        [self createSurfaceTypeLabel];
        //[self createIncrementLabel];
        
        //[self  createThruIDTextField];
        [self createInputPopupButton];
        [self createOutputPopupButton];
        [self createSurfaceTypePopupButton];
        //[self createIncrementField];

        [self createBottomToolbar];
        [self createDeleteButton];
        [self createSubmitButton];
        //[self createHelpButton];
        
        //Updates to determine whether to activate submit button
        [self devicePopupButtonClicked:self.inputPopupButton];
        [self devicePopupButtonClicked:self.outputPopupButton];
        [self devicePopupButtonClicked:self.surfaceTypePopupButton];

        self.userInteractionEnabled = true;
        _submitButton.enabled = YES;

        if( device ) 
        {
            for(int deviceIndex=0; deviceIndex<CMDevice.documents.count;  deviceIndex++)
            {
                CMHardwareDevice* deviceAtIndex = [CMDevice.documents objectAtIndex:deviceIndex];
                if( device == deviceAtIndex )
                {
                    //deviceAtIndex = [CMDevice.documents objectAtIndex:(deviceIndex+1)%CMDevice.documents.count];
                    NSSet* indexSet = [[NSSet alloc] initWithArray:@[[NSIndexPath indexPathForItem:deviceIndex inSection:0]]];
                    [self.deviceGroupView selectItemsAtIndexPaths:indexSet scrollPosition:NSCollectionViewScrollPositionNearestHorizontalEdge];
                    break;
                    //[self setActiveDevice:deviceAtIndex];
                    //return;
                }
            }
            [self setInputFieldsForDevice:device];
        }
        else
        {
            NSSet* indexSet = [[NSSet alloc] initWithArray:@[[NSIndexPath indexPathForItem:0 inSection:0]]];
            [self.deviceGroupView selectItemsAtIndexPaths:indexSet scrollPosition:NSCollectionViewScrollPositionNearestHorizontalEdge];
        }
       /*
        NSLayoutConstraint * documentTop = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                        attribute: NSLayoutAttributeTop
                                                                        relatedBy:NSLayoutRelationEqual
                                                                           toItem:self.scrollView
                                                                        attribute:NSLayoutAttributeTop
                                                                       multiplier:1.0f constant:0];
        
        NSLayoutConstraint * documentBottom = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                           attribute: NSLayoutAttributeBottom
                                                                           relatedBy:NSLayoutRelationEqual
                                                                              toItem:self.scrollView
                                                                           attribute:NSLayoutAttributeBottom
                                                                          multiplier:1.0f constant:0];
        
        NSLayoutConstraint * documentLeft = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                         attribute: NSLayoutAttributeLeft
                                                                         relatedBy:NSLayoutRelationEqual
                                                                            toItem:self.scrollView
                                                                         attribute:NSLayoutAttributeLeft
                                                                        multiplier:1.0f constant:0.f];
        
        NSLayoutConstraint * documentRight = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                          attribute: NSLayoutAttributeRight
                                                                          relatedBy:NSLayoutRelationEqual
                                                                             toItem:self.scrollView
                                                                          attribute:NSLayoutAttributeRight
                                                                         multiplier:1.0f constant:0.0f];
        
        
        [self.scrollView addConstraints:@[documentTop, documentLeft, documentBottom, documentRight]];
        
    
        [self updateDocumentView];
        [_descriptionLabel setNeedsLayout:YES];
        [_descriptionLabel setNeedsDisplay:YES];
        */
    }
    
    return self;
}


-(void)scrollToBottom
{
    
    
}


-(void)createScrollView
{
    
    self.scrollView = [[NSScrollView alloc] initWithFrame:CGRectMake(0,0,100,100)];
    self.scrollView.backgroundColor = [NSColor colorWithWhite:233./255. alpha:1.];
    [self addSubview:self.scrollView];
    self.scrollView.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * CenterX = [NSLayoutConstraint constraintWithItem:self.scrollView
                                                                attribute: NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeCenterX
                                                                multiplier:1 constant:0];

    NSLayoutConstraint * right = [NSLayoutConstraint constraintWithItem:self.scrollView
                                                                attribute: NSLayoutAttributeRight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeRight
                                                                 multiplier:1.0f constant:0];

    NSLayoutConstraint * top = [NSLayoutConstraint constraintWithItem:self.scrollView
                                                                attribute: NSLayoutAttributeTop
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeTop
                                                                multiplier:1.0f constant:0.0f];

    NSLayoutConstraint * bottom = [NSLayoutConstraint constraintWithItem:self.scrollView
                                                                attribute: NSLayoutAttributeBottom
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self
                                                                attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0f constant:0.0f];
    
    
    [self addConstraints:@[CenterX, right, top, bottom]];
    
    
    
    self.scrollDocumentView = [[NSView alloc] initWithFrame:CGRectMake(0,0,200,200)];
    self.scrollDocumentView.wantsLayer = true;
    self.scrollDocumentView.layer.backgroundColor = [NSColor redColor].CGColor;
    [self.scrollView setDocumentView:self.scrollDocumentView];
    self.scrollDocumentView.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * sCenterX = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                attribute: NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.scrollView
                                                                attribute:NSLayoutAttributeCenterX
                                                                multiplier:1.f constant:0];

    NSLayoutConstraint * sright = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                attribute: NSLayoutAttributeRight
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.scrollView
                                                                attribute:NSLayoutAttributeRight
                                                                 multiplier:1.0f constant:0];

    NSLayoutConstraint * stop = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                attribute: NSLayoutAttributeTop
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.scrollView
                                                                attribute:NSLayoutAttributeTop
                                                                multiplier:1.0f constant:0.0f];

    NSLayoutConstraint * sbottom = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                attribute: NSLayoutAttributeBottom
                                                                    relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.scrollView
                                                                attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0f constant:0.0f];
    
    
    [self.scrollView addConstraints:@[stop, sCenterX, sbottom, sright]];
}



-(void)updateDocumentView
{
    [self layoutSubtreeIfNeeded];
    
    /*
    CGRect frame = self.scrollDocumentView.frame;
    self.scrollDocumentView.frame = CGRectMake(0,0,frame.size.width, 1000);

    CGFloat documentViewOffset = (self.cancelButton.frame.origin.y - self.frame.origin.y + self.cancelButton.frame.size.height - (self.frame.size.height) );
    if( documentViewOffset < 0 )
    documentViewOffset = 0;
    
    NSLayoutConstraint * documentTop = [NSLayoutConstraint constraintWithItem: self.scrollDocumentView
                                                            attribute: NSLayoutAttributeTop
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.scrollView attribute:NSLayoutAttributeTop
                                                           multiplier:0.5 constant:0.0f];
     
     NSLayoutConstraint * documentLeft = [NSLayoutConstraint constraintWithItem: self.scrollDocumentView
                                                              attribute: NSLayoutAttributeLeft
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.scrollView attribute:NSLayoutAttributeLeft
                                                             multiplier:1 constant:0.0f];



    NSLayoutConstraint * documentBottom = [NSLayoutConstraint constraintWithItem: self.scrollDocumentView
                                                            attribute: NSLayoutAttributeBottom
                                                            relatedBy:NSLayoutRelationEqual
                                                              toItem: self.cancelButton attribute:NSLayoutAttributeBottom
                                                                multiplier:1 constant:20];

    NSLayoutConstraint * documentRight = [NSLayoutConstraint constraintWithItem: self.scrollDocumentView
                                                                                        attribute: NSLayoutAttributeRight
                                                                                        relatedBy:NSLayoutRelationEqual
                                                                                           toItem: self.scrollView attribute:NSLayoutAttributeRight
                                                                                       multiplier:1 constant:0.0f];
                               
    [self.scrollView addConstraints:@[ documentTop, documentLeft, documentBottom, documentRight ]];
     */
}


-(NSTextFieldButton*)createUserInputButtonForField:(NSTextField*)textField withTitle:(NSString*)title
{
    //CGFloat buttonWidth = 200;
    //CGFloat buttonHeight = 100;
    //CGFloat buttonX = self.view.frame.size.width/2 - buttonWidth/2;
    //CGFloat buttonY = self.view.frame.size.height/2 - buttonHeight/2;
    NSTextFieldButton * button = [[NSTextFieldButton alloc] initWithFrame:CGRectMake(0,0,200,100)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    
    textField.enabled = NO;
    
    button.textfield = textField;
    button.chooseDir = NO;
    button.chooseFiles = YES;
    [button setAction:@selector(textFieldButtonClicked:)];

    [button setTitle:title];
    [button setTarget:self];
    //[button setAction:@selector(csvButtonClicked:)];
    
    button.translatesAutoresizingMaskIntoConstraints = NO;
    [self.scrollDocumentView addSubview:button];
    
    //[self addLoginButtonConstraintsToItem:self fromItem:_loginButton];
    
    NSLayoutConstraint * cy = [NSLayoutConstraint constraintWithItem: button
                                                           attribute: NSLayoutAttributeCenterY
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: textField attribute:NSLayoutAttributeCenterY
                                                          multiplier:1.0f constant:0.0f];
    
    NSLayoutConstraint * left = [NSLayoutConstraint constraintWithItem: button
                                                             attribute: NSLayoutAttributeLeft
                                                             relatedBy:NSLayoutRelationEqual
                                                                toItem: textField attribute:NSLayoutAttributeRight
                                                            multiplier:1.0f constant:10.0f];
    
    /*
     NSLayoutConstraint * top = [NSLayoutConstraint constraintWithItem: _csvButton
     attribute: NSLayoutAttributeTop
     relatedBy:NSLayoutRelationEqual
     toItem: _pwField attribute:NSLayoutAttributeBottom
     multiplier:1.0 constant:20];
     */
    /*
     NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: _csvButton
     attribute: NSLayoutAttributeWidth
     relatedBy:NSLayoutRelationEqual
     toItem: self attribute:NSLayoutAttributeWidth
     multiplier:.015 constant:0];
     
     NSLayoutConstraint * height = [NSLayoutConstraint constraintWithItem: _csvButton
     attribute: NSLayoutAttributeHeight
     relatedBy:NSLayoutRelationEqual
     toItem: _nameField attribute:NSLayoutAttributeHeight
     multiplier:1 constant:0 ];
     */
    [self.scrollDocumentView addConstraints:@[ cy, left ]];
    
    return button;
    
}

/*
- (id)initWithFrame:(NSRect)frame andDataModel:(VRTDOM*)dataModel{
    self = [super initWithFrame:frame];
    if (self) {

        self.modalViewMode = ModalView_Modify;
        self.dataModel = dataModel;
        
        [self createTitleLabel];
        
        [self createUserInputFields];
        [self createModalButtons];
        //[self createLoginButton];
        
        //self.layer.backgroundColor = [[NSColor blueColor] CGColor];
        [self createActivityIndicator];
        
    }
    return self;
}

*/

/*
-(void)createActivityIndicator
{
    
    if( !self.indicatorView )
    {
        self.indicatorView = [[CustomIndicatorView alloc] init];
        _indicatorView.layer.cornerRadius = 6.0;
        
        [self addSubview:_indicatorView];
        
        _indicatorView.translatesAutoresizingMaskIntoConstraints = NO;
        //[self addIndicatorViewConstraintsToItem:_loginButtonContainer fromItem:_indicatorView ];
        
        NSLayoutConstraint * centerX = [NSLayoutConstraint constraintWithItem: _indicatorView
                                                                    attribute: NSLayoutAttributeCenterX
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem: self attribute:NSLayoutAttributeCenterX
                                                                   multiplier:1.0f constant:0.0f];
        
        
        NSLayoutConstraint * centerY = [NSLayoutConstraint constraintWithItem: _indicatorView
                                                                    attribute: NSLayoutAttributeCenterY
                                                                    relatedBy:NSLayoutRelationEqual
                                                                       toItem: self attribute:NSLayoutAttributeCenterY
                                                                   multiplier:1.0f constant:0.0f];
        
        
        NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: _indicatorView
                                                                  attribute: NSLayoutAttributeWidth
                                                                  relatedBy:NSLayoutRelationEqual
                                                                     toItem: self attribute:NSLayoutAttributeWidth
                                                                 multiplier:0 constant:150];
        
        NSLayoutConstraint * height = [NSLayoutConstraint constraintWithItem: _indicatorView
                                                                   attribute: NSLayoutAttributeHeight
                                                                   relatedBy:NSLayoutRelationEqual
                                                                      toItem: self attribute:NSLayoutAttributeHeight
                                                                  multiplier:0 constant:100];//22.0f * [[UIScreen mainScreen] scale]];
        
        [self addConstraints:@[ centerX, centerY, width, height ]];
        
        
        _indicatorView.label.stringValue = @"Logging In";
        //_indicatorView.label.font = [[[MastryFontManager sharedInstance] buttonThemeBoldFont] fontWithSize:36.0];
        //_indicatorView.label.minimumScaleFactor = 0.1;
        
        //_indicatorView.label.numberOfLines=0;
        //_indicatorView.label.lineBreakMode=NSLineBreakByWordWrapping;
        //_indicatorView.label.adjustsFontSizeToFitWidth=YES;
        _indicatorView.hidden = YES;
        
        //[_indicatorView showAndStartAnimating];
        
    }
    
    
}
 */

-(void)dealloc
{
    NSLog(@"MastryDOMView dealloc");

}



-(void) addTitleLabelConstraintsToItem:(NSView*)toItem fromItem:(NSView*)fromItem
{
    
    NSLayoutConstraint * CenterX = [NSLayoutConstraint constraintWithItem: fromItem
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: toItem attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:0.0f];
    
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: fromItem
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: toItem attribute:NSLayoutAttributeBottom
                                                               multiplier:0.10 constant:0];
    
    NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: fromItem
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: toItem attribute:NSLayoutAttributeWidth
                                                             multiplier:1 constant:0];

    
    [toItem addConstraints:@[ CenterY, width, CenterX ]];
    
}

-(NSTextField*)createTextFieldWithText:(NSString *)text placeholderText:(NSString*)placeholderText prevTextField:(NSView*)prevTextField
{
    
    NSTextField * textField = [[NSTextField alloc] initWithFrame:CGRectMake(0,0,60,100)];
    [textField setWantsLayer:YES];
    textField.layer.cornerRadius = 3.0;
    textField.layer.backgroundColor = [NSColor blueColor].CGColor;

    [textField setBezeled:YES];
    [textField setDrawsBackground:YES];
    [textField setEditable:YES];
    [textField setSelectable:YES];
    
    //textField = UITextBorderStyleNone;
    //textField = [UIFont systemFont];
    //textField = @"user@spur.com";
    
    //textField.stringValue = @"Hello";
    if( placeholderText )
        textField.placeholderString = placeholderText;
    
    if( text )
        textField.stringValue = text;
    textField.textColor = [NSColor darkGrayColor];
    textField.delegate = self;
    
    [self.scrollDocumentView addSubview:textField];
    
    
    textField.translatesAutoresizingMaskIntoConstraints = NO;

    NSLayoutConstraint * CenterX = [NSLayoutConstraint constraintWithItem: textField
                                                                attribute: NSLayoutAttributeCenterX
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.scrollDocumentView attribute:NSLayoutAttributeCenterX
                                                               multiplier:1.f constant:0.0f];
    
    
    NSLayoutConstraint * top = [NSLayoutConstraint constraintWithItem: textField
                                                            attribute: NSLayoutAttributeTop
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: prevTextField attribute:NSLayoutAttributeBottom
                                                           multiplier:1.0 constant:0];
    
    NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: textField
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.scrollDocumentView attribute:NSLayoutAttributeWidth
                                                             multiplier:0.4 constant:0];
    

    [self.scrollDocumentView addConstraints:@[ CenterX, width, top ]];
    
    
    textField.hidden = NO;
    [textField setNeedsDisplay:YES];
    [textField setNeedsLayout:YES];
    
    return textField;
    
}

- (NSView *)hitTest:(NSPoint)aPoint
{
    if( self.userInteractionEnabled )
        return [super hitTest:aPoint];
    else
        return nil;
}

-(void)textFieldButtonClicked:(id)sender
{
    NSTextFieldButton * button = (NSTextFieldButton *)sender;
    
    NSOpenPanel* openDlg = [NSOpenPanel openPanel];
    [openDlg setLevel:NSPopUpMenuWindowLevel];
    [openDlg setAllowsMultipleSelection:NO];
    
    [openDlg setCanChooseDirectories:button.chooseDir];
    [openDlg setCanChooseFiles:button.chooseFiles];
    //[openDlg setCanCreateDirectories:NO];
    
    [openDlg setPrompt:@"Select"];
    
    //NSArray* imageTypes = [NSImage imageTypes];
    
    if( button.filetypes && button.filetypes.count > 0 )
        [openDlg setAllowedContentTypes:button.filetypes];
    
    
    self.userInteractionEnabled = false;
    
    __weak typeof(self) weakSelf = self;
    
    [openDlg beginWithCompletionHandler:^(NSInteger result){
        
        //if( result == NSModalResponseOK )
        //{
        //get the URL of the selected file
        NSArray* pathURLs = [openDlg URLs];
        if( pathURLs && pathURLs.count > 0 )
        {
            NSURL * pathURL = [pathURLs objectAtIndex:0];
            
            //NSArray* filepaths = [openDlg filenames];
            //get the filepath of the selected file and convert back to url
            NSString * filepath = pathURL.path;//[filepaths objectAtIndex:0];
            //NSURL * url = [[NSURL alloc] initFileURLWithPath:filepath];
        
            if( filepath )
            {
                //NSTextField * textField = (NSTextField*) [self.inputFields objectForKey:@"URL"];
                button.textfield.stringValue = filepath;
            }
            else
            {
                [self showAlertView:@"Error" message:@"Failed to get local URL to selected file." completionHandler:nil];
            }
        }
        
        //return user interaction to this view
        //dispatch_async(dispatch_get_main_queue(), ^{
            weakSelf.userInteractionEnabled = true;
        //});
        
    }];

}

-(void)createUserInputFields
{
    //loop over each property and create and input field
    unsigned int count = 0;
    //NSMutableDictionary *dictionary = [NSMutableDictionary new];
    self.inputFields = [NSMutableDictionary new];
    self.inputFieldKeys = [NSMutableArray new];
    self.fieldButtons = [NSMutableDictionary new];
    
    if( self.modalViewMode == ModalView_Control  )//|| self.modalViewMode == ModalView_Modify)
    {
        objc_property_t *properties = nil;//class_copyPropertyList([CMThruConnection class], &count);
        
        NSView * prevTextField = self.scrollDocumentView;
        for (int i = 0; i < count; i++) {
            
            NSString *key = [NSString stringWithUTF8String:property_getName(properties[i])];
            //NSObject * value = [self valueForKey:key];
            
            NSLog(@"Input Field Key = %@", key);
            //if( value && [value isKindOfClass:[NSString class]] )
            // {
                //create a text field
                id text = nil;
                //if( self.modalViewMode == ModalView_Modify ) text = [self.dataModel valueForKey:key];
            
                NSTextField * textField = [self createTextFieldWithText:text placeholderText:key prevTextField:prevTextField];
                //textField.delegate = self;
                prevTextField = textField;
                [self.inputFields setObject:textField forKey:key];
                [self.inputFieldKeys addObject:key];
            
                if( [key localizedCompare:@"URL"] == NSOrderedSame)
                {
                    NSTextFieldButton * button = [self createUserInputButtonForField:textField withTitle:@"Choose..."];
                    button.filetypes = @[@"png", @"PNG"];
                }
            //}
            //NSString *value = [NSString stringWithFormat:@"%@",[self valueForKey:key]];
            //[dictionary setObject:value forKey:key];
            //id value = [self valueForKey:key];
            /*
             if (value == nil) {
             // nothing todo
             }
             else if ([value isKindOfClass:[NSNumber class]]
             || [value isKindOfClass:[NSString class]]
             || [value isKindOfClass:[NSDictionary class]] || [value isKindOfClass:[NSMutableArray class]]) {
             // TODO: extend to other types
             [dictionary setObject:value forKey:key];
             }
             else if ([value isKindOfClass:[NSObject class]]) {
             [dictionary setObject:[value dictionary] forKey:key];
             }
             else {
             NSLog(@"Invalid type for %@ (%@)", NSStringFromClass([self class]), key);
             }
             */
        }
        free(properties);
        
    }
    /*
    else if( self.modalViewMode == ModalView_Create_MANY)
    {
        
        //NSArray * fieldTitleStrings = [[NSArray alloc] initWithObjects:@"Path ]
        //NSArray * fieldKeys = [[NSArray alloc] initWithObjects:@"CSV", "DIR", nil];
        NSArray * fieldKeys = [[NSArray alloc] initWithObjects:@"Path to CSV", @"Path to Image Directory", nil];

        //NSArray *
        
        NSView * prevTextField = _descriptionLabel;
        for (int i = 0; i < fieldKeys.count; i++) {
            
            NSString * key = [fieldKeys objectAtIndex:i];
            NSTextField * textField = [self createTextFieldWithText:nil placeholderText:key prevTextField:prevTextField];
            NSTextFieldButton * button = [self createUserInputButtonForField:textField withTitle:@"Choose..."];
            
            if( i == 0 )
                
            {
                _csvField = textField;
                button.chooseDir = NO;
                button.chooseFiles = YES;
                button.filetypes = @[@"csv", @"CSV"];
            }
            else
            {
                _dirField = textField;
                button.chooseDir = YES;
                button.chooseFiles = NO;
            }
            
            //textField.delegate = self;
            prevTextField = textField;
            [self.inputFields setObject:textField forKey:key];
            [self.inputFieldKeys addObject:key];
            
        }


    }
     */
    //return dictionary;
    
}

-(NSButton *)createModalButton:(NSString *)title topView:(NSView*)topView selector:(SEL)selector
{
    //CGFloat buttonWidth = 200;
    //CGFloat buttonHeight = 100;
    //CGFloat buttonX = self.view.frame.size.width/2 - buttonWidth/2;
    //CGFloat buttonY = self.view.frame.size.height/2 - buttonHeight/2;
    NSButton * button = [[NSButton alloc] initWithFrame:CGRectMake(0,0,0,100)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    [button setTitle:title];
    [button setTarget:self];
    [button setAction:selector];
    
    button.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:button];
    
    //[self addLoginButtonConstraintsToItem:self fromItem:_loginButton];
    
    NSLayoutConstraint * cx = [NSLayoutConstraint constraintWithItem: button
                                                           attribute: NSLayoutAttributeCenterX
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: self attribute:NSLayoutAttributeCenterX
                                                          multiplier:1.0f constant:0.0f];
    
    NSLayoutConstraint * bottom = [NSLayoutConstraint constraintWithItem: button
                                                            attribute: NSLayoutAttributeBottom
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self attribute:NSLayoutAttributeBottom
                                                           multiplier:1.0 constant:20];
    
    NSLayoutConstraint * width = [NSLayoutConstraint constraintWithItem: button
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self attribute:NSLayoutAttributeWidth
                                                             multiplier:0.25 constant:0];
    
    
    [self addConstraints:@[ cx, width, bottom ]];
    
    return button;
    
}

/*
-(const char *)getCollectionTitle
{
    const char * collectionTitle = nil;
    if( self.dataModel.type > NUM_DOM_COLLECTIONS)
        collectionTitle = VRT_EXT_DOM_COLLECTION_TITLES[self.dataModel.type - NUM_DOM_COLLECTIONS];
    else
        collectionTitle = VRT_DOM_COLLECTION_TITLES[self.dataModel.type];
    return collectionTitle;
}

-(const char *)getDOMTitle
{
    const char * collectionTitle = nil;
    if( self.dataModel.type > NUM_DOM_COLLECTIONS)
        collectionTitle = VRT_EXT_DOM_TITLES[self.dataModel.type - NUM_DOM_COLLECTIONS];
    else
        collectionTitle = VRT_DOM_TITLES[self.dataModel.type];
    return collectionTitle;
}
*/

- (void)controlTextDidChange:(NSNotification *)notification {
    NSTextField *textField = [notification object];
    if( textField.stringValue.length > 0 )
        _submitButton.enabled = YES;
    else
        _submitButton.enabled = NO;
}

- (void)controlTextDidEndEditing:(NSNotification *)notification {
    //NSTextField *textField = [notification object];
    //if ([textField resignFirstResponder]) {
    //    textField.textColor = [NSColor blackColor];
    //}
}

-(void)helpButtonClicked:(id)sender
{
    
}

-(void)submitButtonClicked:(id)sender
{
    //[self.indicatorView showAndStartAnimating];

        //The submit button should not have been activated unless fields have been populated
        //TO DO: check ThruID for name collisions
        
        //TO DO:  make these asserts user alerts
        //assert(self.thruIDField.stringValue.length > 0);
        //assert(self.inputPopupButton.indexOfSelectedItem != self.outputPopupButton.indexOfSelectedItem);
        
        //TO DO:  check the field value for sanity
        int inputIDs[1] =  {(int)self.inputPopupButton.indexOfSelectedItem};
        int outputIDs[1] = {(int)self.outputPopupButton.indexOfSelectedItem};
        int surfaceType[1] = {(int)self.surfaceTypePopupButton.indexOfSelectedItem};

        //First get the CollectionView selectdIndexPath to check if this click is for a new device or to modify an existing device
        
        NSSet * selectedIndexPaths = [self.deviceGroupView selectionIndexPaths];
        NSIndexPath* selectedIndexPath = nil;
        
        if( selectedIndexPaths.count > 0 )
        {
            assert(selectedIndexPaths.count == 1); //we only allow one selected item at a time in this app for now
            selectedIndexPath = selectedIndexPaths.allObjects.firstObject;
        }
        else assert(1==0); //we should always have a collection view item selected

        NSInteger index = selectedIndexPath.item;//[[self.deviceGroupView content] indexOfObject:[item representedObject]];

        if( index >= CMDevice.documents.count ) // new device
        {
            //Populate thruParam Inputs/Outputs
            MIDIThruConnectionParams* thruParams = &(CMClient.thruConnections[CMClient.numThruConnections].params);
            CMInitThruParams(thruParams, inputIDs, 1, outputIDs, 1);
            
            //struct CMDeviceDescription * deviceDescription = &(CMClient.hardwareDevices[CMClient.numHardwareDevices]);
            CMDeviceType deviceType = surfaceType[0];
            
            //Use the CoreMIDI unique id of the input source endpoint for our DOM primary key
            NSString * sourceKey = [NSString stringWithFormat:@"%d", CMClient.sources[inputIDs[0]].uniqueID];//[NSString stringWithUTF8String:CMClient.sources[inputIDs[0]].uniqueID]'
            //NSString * destKey = [NSString stringWithFormat:@"%d", destUniqueID];//[NSString stringWithUTF8String:CMClient.sources[inputIDs[0]].uniqueID]'
            
            //TO DO:  introduce some error checking to this function pipeline
            CMSoftThruConnection * thruConnection = [CMSoftThruConnection createSoftThruConnection:sourceKey Params:thruParams];
            CMHardwareDevice* device = [CMHardwareDevice createHardwareDevice:sourceKey Type:deviceType];
            assert(device);
            assert(thruConnection);
            assert(thruConnection.connection);
            [device setConnection:thruConnection.connection];

            //[CMHardwareDevice createHardwareDevice:[NSString stringWithUTF8String:CMClient.sources[inputIDs[0]].name] Description:deviceDescription];
            
            //[[PbAudioAppDelegate sharedInstance] setActiveDevice:device];
            //[[PbAudioAppDelegate sharedInstance] openActiveDeviceWindow];
            
            //update the collection view UI
            [self.deviceGroupView reloadData];
            
            //select the new item at the beginning of the collection view
            NSSet* indexSet = [[NSSet alloc] initWithArray:@[[NSIndexPath indexPathForItem:CMDevice.documents.count-1 inSection:0]]];
            [self.deviceGroupView  selectItemsAtIndexPaths:indexSet scrollPosition:NSCollectionViewScrollPositionNearestHorizontalEdge];
            
            
            //[[PbAudioAppDelegate sharedInstance] dismissModalWindow];
        }
        else // modify existing device
        {
            //find selected device
            CMHardwareDevice * selectedDevice = [CMDevice.documents objectAtIndex:index];
            [self modifyDeviceWithInputFields:selectedDevice];
        }


  
}

-(void)cancelButtonClicked:(id)sender
{
    NSLog(@"cancelButtonClicked");
    //PbAudioAppDelegate *appDelegate = (PbAudioAppDelegate *)[[NSApplication sharedApplication] delegate];
    //[appDelegate dismissModalWindow];
}

-(void)createModalButtons
{
    //everybody gets a submit button
    
    NSView * topView = nil;//'_scrollListView;//[_inputFields objectForKey:[_inputFieldKeys objectAtIndex:_inputFieldKeys.count -1]];
    self.submitButton = [self createModalButton:@"Submit" topView:topView selector:@selector(submitButtonClicked:)];
    self.cancelButton = [self createModalButton:@"Cancel" topView:self.submitButton selector:@selector(cancelButtonClicked:)];

    self.modalButtons = [NSMutableArray new];
    [self.modalButtons addObject:self.submitButton];
    [self.modalButtons addObject:self.cancelButton];

    /*)
    NSLayoutConstraint * bottom = [NSLayoutConstraint constraintWithItem: self.scrollView
                                                              attribute: NSLayoutAttributeBottom
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.cancelButton attribute:NSLayoutAttributeBottom
                                                             multiplier:1 constant:0];
    
    */
    //[self addConstraints:@[ bottom ]];

}

- (void)processAlertReturnCode:(NSInteger)returnCode
{
    if (returnCode == NSModalResponseOK )//|| returnCode == NSOKButton)
    {
        NSLog(@"(returnCode == NSOKButton)");
        //[self.window makeKeyWindow];
    }
    else if (returnCode == NSModalResponseCancel)// || returnCode == NSCancelButton)
    {
        NSLog(@"(returnCode == NSCancelButton)");
        
        //MastryAdminAppDelegate *appDelegate = (MastryAdminAppDelegate *)[[NSApplication sharedApplication] delegate];
    [[NSApp modalWindow] makeKeyWindow];
        //[appDelegate.window makeKeyWindow];

    }
    else if(returnCode == NSAlertFirstButtonReturn)
    {
        NSLog(@"if (returnCode == NSAlertFirstButtonReturn)");
    }
    else if (returnCode == NSAlertSecondButtonReturn)
    {
        NSLog(@"else if (returnCode == NSAlertSecondButtonReturn)");
    }
    else if (returnCode == NSAlertThirdButtonReturn)
    {
        NSLog(@"else if (returnCode == NSAlertThirdButtonReturn)");
    }
    else
    {
        NSLog(@"All Other return code %ld",(long)returnCode);
    }
    
}

- (void)alertDidEnd:(NSAlert *)alert returnCode:(NSInteger)returnCode contextInfo:(void *)contextInfo
{
    /*
     The following options are deprecated in 10.9. Use NSAlertFirstButtonReturn instead
     NSAlertDefaultReturn = 1,
     NSAlertAlternateReturn = 0,
     NSAlertOtherReturn = -1,
     NSAlertErrorReturn = -2
     NSOKButton = 1, // NSModalResponseOK should be used
     NSCancelButton = 0 // NSModalResponseCancel should be used
     */
    
    [self processAlertReturnCode:returnCode];
    
}

-(void)showAlertView:(NSString*)title message:(NSString*)message async:(BOOL)async
{
    if( async )
    {
        dispatch_async(dispatch_get_main_queue(), ^{
            [self showAlertView:title message:message completionHandler:nil];
        });
    }
    else
        [self showAlertView:title message:message completionHandler:nil];
}
-(void)showAlertView:(NSString*)title message:(NSString*)message completionHandler:(void (^ __nullable)(NSModalResponse returnCode))handler
{
    
    //MastryAdminAppDelegate *appDelegate = (MastryAdminAppDelegate *)[[NSApplication sharedApplication] delegate];
    //[self.window setStyleMask:appDelegate.modalWindowStyleMask];

    //dispatch_sync(dispatch_get_main_queue(), ^{
        
        [self.indicatorView hideAndStopAnimating];
        
        NSAlert *alert = [[NSAlert alloc] init];
        //alert.delegate = self;
        //[alert addButtonWithTitle:@"Continue"];
        //[alert addButtonWithTitle:@"OK"];
        [alert setMessageText:title];
        [alert setInformativeText:message];
        [alert setAlertStyle:NSAlertStyleWarning];

        //[alert beginSheetModalForWindow:[self window] modalDelegate:self didEndSelector:@selector(alertDidEnd:returnCode:contextInfo:) contextInfo:nil];
        NSModalResponse response = [alert runModal];

        [self processAlertReturnCode:response];
        
        //CMThruApplicationDelegate *appDelegate = (CMThruApplicationDelegate *)[[NSApplication sharedApplication] delegate];
        [self.window setStyleMask:NSWindowStyleMaskTitled];
        //[self.window display];
        [self.window layoutIfNeeded];
        [self.window setViewsNeedDisplay:YES];
        
    //});
    
}


- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    //[self.descriptionLabel drawRect:dirtyRect];
    //[self.descriptionLabel drawCellInside:self.descriptionLabel.cell];
    // Drawing code here.
    
    //self.imageView.layer.cornerRadius = self.imageView.frame.size.width/2.0;
    
}
@end
