//
//  AudioMidiSettingsView.m
//  [Pb] Audio
//
//  Created by Joe Moulton on 9/10/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#import "AudioMidiSettingsView.h"

#import "PbAudioAppInterface.h"

#import <Foundation/NSObjcRuntime.h>
#import <objc/runtime.h>

#import "PbAudioAppDelegate.h"

//#import "CustomIndicatorView.h"
//#import "CocoaTextFieldButton.h"

#import "NSCheckboxView.h"

#import "AVFoundation/AVAudioSession.h"

//#import "CMHardwareDevice.h"

Float64 HardcodedSampleRates[3] = {44100., 48000., 96000.};

@interface AudioMidiSettingsView()
{
    UInt32        _minBufferSize;
    UInt32        _maxBufferSize;
    UInt32        _minBufferSizeStep;
    
    int               _nSampleRates;
    AudioValueRange*  _SampleRates;
    
    int               _nBufferSizes;
    AudioValueRange*  _BufferSize;

    
    PBAudioDeviceList _deviceList;
    
    id _defaultDeviceObserverToken;
    id _deviceAvailabilityObserverToken;

    id _midiSourceObserverToken;
    id _midiDestinationObserverToken;

    //Labels
    //CocoaTextField *_inputDeviceLabel;
    CocoaTextField *_outputDeviceLabel;

    //Controls
    CocoaScrollView * _channelScrollView;

    
    //NSMutableDictionary * _inputFields;
    //NSMutableDictionary * _fieldButtons;
        
    //NSButton * _submitButton;
    //NSButton * _deleteButton;
    //NSButton * _cancelButton;
    
    volatile bool _userInteractionEnabled;
    
    
}


@property (nonatomic, strong) id defaultDeviceObserverToken;
@property (nonatomic, strong) id deviceAvailabilityObserverToken;

@property (nonatomic, strong) id midiSourceObserverToken;
@property (nonatomic, strong) id midiDestinationObserverToken;

//@property (nonatomic, retain) NSImageView * imageView;
//@property (nonatomic, retain) CMLogoView * imageView;

//Labels

//Audio Output Device Control
@property (nonatomic, retain) CocoaTextField   * outputDeviceLabel;
@property (nonatomic, retain) CocoaPopUpButton * outputPopupButton;
@property (nonatomic, retain) CocoaButton      * testButton;

//Audio Output Channel Control 
@property (nonatomic, retain)    CocoaTextField * outputChannelLabel;
@property (nonatomic, retain)   CocoaScrollView * channelScrollView;
@property (nonatomic, retain) NSCheckboxView * channelFilterView;

//Device Sample Rate Control
@property (nonatomic, retain) CocoaTextField   * sampleRateLabel;
@property (nonatomic, retain) CocoaPopUpButton * sampleRatePopupButton;

//Stream Buffer Size Control
@property (nonatomic, retain) CocoaTextField   * bufferSizeLabel;
@property (nonatomic, retain) CocoaPopUpButton * bufferSizePopupButton;

//Midi Input Device Control
@property (nonatomic, retain)    CocoaTextField * midiInputLabel;
@property (nonatomic, retain)   CocoaScrollView * midiInputScrollView;
@property (nonatomic, retain) NSCheckboxView * midiInputFilterView;

//Toolbar
@property (nonatomic, retain) CocoaView * bottomToolbar;


@property (nonatomic) int modalViewMode;
@property (nonatomic) int type;

//@property (nonatomic, retain) CMProxyConnection * dataModel;

@property (atomic) bool userInteractionEnabled;


@property (nonatomic, retain) NSMutableArray * inputFieldKeys;
@property (nonatomic, retain) NSMutableDictionary *inputFields;
@property (nonatomic, retain) NSMutableDictionary *fieldButtons;

@property (nonatomic, retain) NSMutableArray * modalButtons;

@property (nonatomic, retain) CocoaButton * submitButton;
@property (nonatomic, retain) CocoaButton * helpButton;

@property (nonatomic, retain) CocoaButton * cancelButton;

//@property (nonatomic, retain) CustomIndicatorView * indicatorView;

#if !TARGET_OS_OSX
//Must retain UIMenus to clear lists of UIAction/UICommand
@property (nonatomic, retain) UIMenu * outputDeviceMenu;
@property (nonatomic, retain) UIMenu * sampleRateMenu;
@property (nonatomic, retain) UIMenu * bufferSizeMenu;
#endif

@end

@implementation AudioMidiSettingsView


@synthesize defaultDeviceObserverToken      = _defaultDeviceObserverToken;
@synthesize deviceAvailabilityObserverToken = _deviceAvailabilityObserverToken;

@synthesize midiSourceObserverToken      = _midiSourceObserverToken;
@synthesize midiDestinationObserverToken = _midiDestinationObserverToken;


-(void)RegisterNotificationObservers
{
    /*
    CFNotificationCenterRef center = CFNotificationCenterGetLocalCenter();
    assert(center);
    
    //__weak typeof(self) weakSelf = self;
    //self.defaultDeviceObserverToken =
    CFNotificationCenterAddObserver(center, NULL, PBAudioDeviceDefaultOutputChangedNotificationCallback,
                                    CFSTR("PBADeviceDefaultOutputDeviceChangedNotification"), //_PBAMasterStream->outputEnabled ? kPBADeviceDefaultOutputChangedNotification : kPBADeviceDefaultInputChangedNotification,
                                    NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    
    
    //self.deviceAvailabilityObserverToken =
    CFNotificationCenterAddObserver(center, NULL, PBAudioDevicesAvailableChangedNotificationCallback,
                                    CFSTR("PBADeviceAvailableDevicesChangedNotification"),
                                    NULL, CFNotificationSuspensionBehaviorDeliverImmediately);
    */
    
    __weak typeof(self) weakSelf = self;
    self.defaultDeviceObserverToken = [NSNotificationCenter.defaultCenter addObserverForName:@"PBADeviceDefaultOutputDeviceChangedNotification" object:nil queue:nil usingBlock:^(NSNotification * note) 
    {
        //update ui accordingly
    }];
    
    self.deviceAvailabilityObserverToken = [NSNotificationCenter.defaultCenter addObserverForName:@"PBADeviceAvailableDevicesChangedNotification" object:nil queue:nil usingBlock:^(NSNotification * note) 
    {
        NSLog(@"AudioMidiSettingsView::AudioDevicesAvailableChangedNotification\n");

        //Repopulate the internal list of Audio Devices
        [self updateOutputDeviceList];
    }];

    self.midiSourceObserverToken = [NSNotificationCenter.defaultCenter addObserverForName:@"CMidiSourcesAvailableChangedNotification" object:nil queue:nil usingBlock:^(NSNotification * note)
    {
        NSLog(@"AudioMidiSettingsView::MidiSourcesAvailableChangedNotification\n");

        //Repopulate the internal list of Audio Devices
        [self updateMidiInputDeviceList];
    }];
    
    self.midiDestinationObserverToken = [NSNotificationCenter.defaultCenter addObserverForName:@"CMidiDestinationsAvailableChangedNotification" object:nil queue:nil usingBlock:^(NSNotification * note)
    {
        NSLog(@"AudioMidiSettingsView::MidiDestinationsAvailableChangedNotification\n");

        //Repopulate the internal list of Audio Devices
        //[self updateMidiOutputDeviceList];
    }];

    
    
}

-(void)RemoveNotificationObservers
{
    [[NSNotificationCenter defaultCenter] removeObserver:self.midiSourceObserverToken];
    [[NSNotificationCenter defaultCenter] removeObserver:self.midiDestinationObserverToken];

    [[NSNotificationCenter defaultCenter] removeObserver:self.defaultDeviceObserverToken];
    [[NSNotificationCenter defaultCenter] removeObserver:self.deviceAvailabilityObserverToken];
}

-(void)dealloc
{
    [self RemoveNotificationObservers];
    if(_SampleRates) free(_SampleRates);
}


-(id)initWithFrame:(CGRect)frame //andActiveDevice:(CMHardwareDevice* _Nullable)device
{
    self = [super initWithFrame:frame];
    if(self)
    {
#if TARGET_OS_OSX
        self.wantsLayer = YES;
#endif
      
        //self.backgroundColor       = [CocoaColor darkGrayColor];
        //self.layer.backgroundColor = CocoaColor.darkGrayColor.CGColor;
        //[self setWantsLayer:NO];
        //self.layer.borderWidth = 0.0f;
        //self.layer.masksToBounds = YES;
        //[self.layer setBorderWidth:0];
        //self.appearance = [NSAppearance appearanceNamed: NSAppearanceNameDarkAqua];

        //create a temporary data model object that the modal table view can use to populate itself and visa versa
        //so the user can create a thru coonnection
        //self.modalViewMode = ModalView_Control;
        //self.dataModel     = CMSoftThru;
        
        //Audio Output Device Selection
        [self createOutputDeviceLabel];
        [self createTestButton];
        [self createOutputPopupButton];
        
        
        //Audio Output Channel Selection
        [self createOutputChannelLabel];
        [self createChannelScrollView];
        
        
        //SampleRate Selection
        [self createSampleRateLabel];
        [self createSampleRatePopupButton];

        
        //BufferSize Selection
        [self createBufferSizeLabel];
        [self createBufferSizePopupButton];

        
        //Midi Input Selection
        [self createMidiInputLabel];
        [self createMidiInputScrollView];
        
        
        /*
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
         */
        
       /*
        NSLayoutConstraint * documentTop = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                        attribute: NSLayoutAttributeTop
                                                                        relatedBy:NSLayoutRelationEqual
                                                                           toItem:self.channelScrollView
                                                                        attribute:NSLayoutAttributeTop
                                                                       multiplier:1.0f constant:0];
        
        NSLayoutConstraint * documentBottom = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                           attribute: NSLayoutAttributeBottom
                                                                           relatedBy:NSLayoutRelationEqual
                                                                              toItem:self.channelScrollView
                                                                           attribute:NSLayoutAttributeBottom
                                                                          multiplier:1.0f constant:0];
        
        NSLayoutConstraint * documentLeft = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                         attribute: NSLayoutAttributeLeft
                                                                         relatedBy:NSLayoutRelationEqual
                                                                            toItem:self.channelScrollView
                                                                         attribute:NSLayoutAttributeLeft
                                                                        multiplier:1.0f constant:0.f];
        
        NSLayoutConstraint * documentRight = [NSLayoutConstraint constraintWithItem:self.scrollDocumentView
                                                                          attribute: NSLayoutAttributeRight
                                                                          relatedBy:NSLayoutRelationEqual
                                                                             toItem:self.channelScrollView
                                                                          attribute:NSLayoutAttributeRight
                                                                         multiplier:1.0f constant:0.0f];
        
        
        [self.channelScrollView addConstraints:@[documentTop, documentLeft, documentBottom, documentRight]];
        
    
        [self updateDocumentView];
        [_descriptionLabel setNeedsLayout:YES];
        [_descriptionLabel setNeedsDisplay:YES];
        */
        
        //Subscribe to Audio Device/Configuration Changes needed for UI updates
        [self RegisterNotificationObservers];
    }
    
    return self;
}

/*
//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createInputDeviceLabel
{
    self.inputDeviceLabel = [[CocoaTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
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
*/

#pragma mark -- Output Device Control

//FYI, the width of the title label is currently controlling the width of the popover view!!!
//And the width of the title label is determined by font size + sizetofit
-(void) createOutputDeviceLabel
{
    self.outputDeviceLabel = [[CocoaTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
#if TARGET_OS_OSX
    self.outputDeviceLabel.focusRingType = NSFocusRingTypeDefault;
    self.outputDeviceLabel.stringValue  = @"Active Output Channels:";
    self.outputDeviceLabel.wantsLayer = YES;
    
    //self.outputDeviceLabel.lineBreakMode = NSLineBreakByClipping;
    //self.outputDeviceLabel.usesSingleLineMode = YES;
    //self.outputDeviceLabel.maximumNumberOfLines = YES;
    
    self.outputDeviceLabel.editable = YES;
    self.outputDeviceLabel.drawsBackground = YES;
#else
    
    self.outputDeviceLabel.text  = @"Active Output Channels:";
#endif
    self.outputDeviceLabel.layer.masksToBounds = NO;
    //self.outputDeviceLabel.editable = NO;

    self.outputDeviceLabel.textColor = CocoaColor.whiteColor;
    self.outputDeviceLabel.backgroundColor = [CocoaColor clearColor];
    self.outputDeviceLabel.layer.backgroundColor = [CocoaColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    
#if TARGET_OS_OSX
    self.outputDeviceLabel.alignment = NSTextAlignmentRight;
    
    _outputDeviceLabel.bezeled         = NO;
    _outputDeviceLabel.drawsBackground = NO;
    _outputDeviceLabel.editable        = NO;

    [self addSubview:self.outputDeviceLabel];

#else
    self.outputDeviceLabel.textAlignment = NSTextAlignmentRight;
    [self.contentView addSubview:self.outputDeviceLabel];
#endif
    [self.outputDeviceLabel sizeToFit];
    
    CGRect frame = self.outputDeviceLabel.frame;
    
    self.outputDeviceLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.outputDeviceLabel
                                                             attribute: NSLayoutAttributeLeft
                                                             relatedBy:NSLayoutRelationEqual
                                                                toItem: self attribute:NSLayoutAttributeLeft
                                                            multiplier:1 constant:20.];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.outputDeviceLabel
                                                            attribute: NSLayoutAttributeTop
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self attribute:NSLayoutAttributeTop
                                                           multiplier:1.0 constant:20.];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.outputDeviceLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self attribute:NSLayoutAttributeWidth
                                                             multiplier:0 constant:frame.size.width];
    
    
    [self addConstraints:@[ Left, Top, Width ]];
    
#if TARGET_OS_OSX
    self.outputDeviceLabel.stringValue  = @"Output:";
#else
    self.outputDeviceLabel.text = @"Output:";
#endif
    
}

-(CustomButtonView*)createButtonViewTitle:(NSString*)title
{
    CustomButtonView * button = [[CustomButtonView alloc] initWithFrame:CGRectMake(0,0,80,30)];
    
    //NSString * title = @"";
    button.title = title;
    button.selectedTitle = title;
    button.highlightedTitle = title;
    //[button sizeToFit];
    
#if TARGET_OS_OSX
    button.wantsLayer = YES;
    //button.layer.borderWidth = 1;
#endif
    
    button.layer.borderColor = [CocoaColor clearColor].CGColor;
    button.layer.borderWidth = 0;
    button.layer.cornerRadius = 4;
    //button.layer.masksToBounds = YES;
    //button.layer.maskedCorners = YES;
        
    button.titleLabel.layer.borderWidth = 0;
    button.titleLabel.layer.masksToBounds = YES;
    
    
    
    button.translatesAutoresizingMaskIntoConstraints = NO;
    
    //button.backgroundColor = CocoaColor.grayColor;
    button.layer.backgroundColor = [CocoaColor clearColor].CGColor;
    button.normalBackgroundColor = [CocoaColor grayColor];
    button.highlightedBackgroundColor = [CocoaColor grayColor];
    button.selectedBackgroundColor = [CocoaColor grayColor];
    
    
    //button.backgroundImageView.contentMode
    
    button.delegate = self;
    
    button.titleColor = [CocoaColor whiteColor];
    button.highlightedTitleColor = [CocoaColor whiteColor];
    button.selectedTitleColor = button.titleColor;
    
    button.titleLabel.backgroundColor = [CocoaColor clearColor];
    
    button.layer.backgroundColor = [CocoaColor clearColor].CGColor;

    return button;
}

-(void)createTestButton
{

#if TARGET_OS_OSX
    
    self.testButton = [[CocoaButton alloc] initWithFrame:CGRectMake(0,0,80,30)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    // button.center = CGPointMake(self.view.frame.size.width/2, self.view.frame.size.height/2);
    
    self.testButton.enabled = YES;
    
    [self.testButton setTitle:@"Test"];
    [self.testButton setTarget:self];
    [self.testButton setAction:@selector(testButtonClicked:)];
    self.testButton.wantsLayer = YES;

    self.testButton.layer.backgroundColor = CocoaColor.clearColor.CGColor;
    self.testButton.layer.borderColor = CocoaColor.clearColor.CGColor;
    self.testButton.layer.borderWidth = 0;
    self.testButton.layer.cornerRadius = 4.0f;

    self.testButton.shadow = [[NSShadow alloc] init];
    //self.bottomToolbar.layer.backgroundColor = [NSColor redColor].CGColor;
    //self.bottomToolbar.layer.cornerRadius = 5.0;
    self.testButton.layer.shadowOpacity = 1.0;
    self.testButton.layer.shadowColor = [CocoaColor colorWithWhite:0.3 alpha:1.0].CGColor;
    self.testButton.layer.shadowOffset = CGSizeMake(0, 0.5);
    self.testButton.layer.shadowRadius = 0.5;
    
    self.testButton.contentTintColor = [CocoaColor colorWithWhite:0.9 alpha:1.0];
    self.testButton.bezelStyle = NSBezelStyleRounded;
     
    //self.layer.backgroundColor =
    self.testButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self addSubview:self.testButton];

#else
    //self.testButton = [self createButtonViewTitle:@"Test"];

    
    self.testButton = [[CocoaButton alloc] initWithFrame:CGRectMake(0,0,100,30)];//CGRectMake(buttonX,buttonY, buttonWidth, buttonHeight)];
    
    self.testButton.enabled = YES;
    
    self.testButton.titleLabel.text = @"Test";
    self.testButton.titleLabel.textColor = CocoaColor.whiteColor;
    self.testButton.titleLabel.font = [CocoaFont systemFontOfSize:22.0];
    //[self.testButton setTarget:self];
    //[self.testButton setAction:@selector(testButtonClicked:)];
    //self.testButton.wantsLayer = YES;

    [self.testButton addTarget:self action:@selector(testButtonClicked:) forControlEvents:UIControlEventTouchUpInside];
    
    //self.testButton.backgroundColor = CocoaColor.darkGrayColor;
    self.testButton.layer.backgroundColor = CocoaColor.darkGrayColor.CGColor;
    self.testButton.layer.borderColor = CocoaColor.darkGrayColor.CGColor;
    self.testButton.layer.borderWidth = 0;
    self.testButton.layer.cornerRadius = 4.0f;

    //self.testButton.shadow = [[NSShadow alloc] init];
    //self.bottomToolbar.layer.backgroundColor = [NSColor redColor].CGColor;
    //self.bottomToolbar.layer.cornerRadius = 5.0;
    self.testButton.layer.shadowOpacity = 1.0;
    self.testButton.layer.shadowColor = [CocoaColor colorWithWhite:0.3 alpha:1.0].CGColor;
    self.testButton.layer.shadowOffset = CGSizeMake(0, 0.5);
    self.testButton.layer.shadowRadius = 0.5;
    
    //self.testButton.contentTintColor = [CocoaColor colorWithWhite:0.9 alpha:1.0];
    //self.testButton.bezelStyle = NSBezelStyleRounded;
    
    
    self.testButton.translatesAutoresizingMaskIntoConstraints = NO;
    [self.contentView addSubview:self.testButton];
#endif

    
    self.testButton.translatesAutoresizingMaskIntoConstraints = NO;
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.testButton
                                                           attribute: NSLayoutAttributeRight
                                                           relatedBy:NSLayoutRelationEqual
                                                              toItem: self attribute:NSLayoutAttributeRight
                                                          multiplier:1.0f constant:-26.0f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.testButton
                                                            attribute: NSLayoutAttributeCenterY
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.outputDeviceLabel attribute:NSLayoutAttributeCenterY
                                                           multiplier:1.0 constant:0];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.testButton
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self attribute:NSLayoutAttributeWidth
                                                             multiplier:0 constant:self.frame.size.width/12.];
    
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem: self.testButton
                                                              attribute: NSLayoutAttributeHeight
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.outputDeviceLabel attribute:NSLayoutAttributeHeight
                                                             multiplier:0.85 constant:0];
    
    [self addConstraints:@[ Right, CenterY, Width, Height]];
    
}

-(void)devicePopupButtonClicked:(id)sender
{
    if( sender == self.outputPopupButton )
    {
#if TARGET_OS_OSX
        int selectedIndex = (int)self.outputPopupButton.indexOfSelectedItem;
        PBAudio.SetOutputDevice(&PBAudio.OutputStreams[0], _deviceList.devices[selectedIndex]);
        [self createChannelFilterView]; //repopulate the output channel list for the newly active device
        [self updateSampleRateList];    //repopulate the sample rate list for the newly active device
        [self updateBufferSizeList];    //repopulate the buffer size list for the newly active device
#else
        
#endif
        
    }
}



-(int)updateOutputDeviceList
{
    int outputIndex = 0;
    
    //Ask [Pb]Audio for the list of connected Audio Interface Devices
    _deviceList = PBAudioAvailableDevices(kAudioObjectPropertyScopeOutput);
    
#if TARGET_OS_OSX
    for( int i = 0; i < _deviceList.count; i++)
    {
        UInt32 nameLen = 128; char deviceName[128] = "\0"; PBAudioDeviceName(_deviceList.devices[i], deviceName, &nameLen);
        [self.outputPopupButton addItemWithTitle:[NSString stringWithUTF8String:deviceName]];
        if(PBAudio.OutputStreams[0].audioDevice == _deviceList.devices[i]) outputIndex = i;
    }

    //select the index of active audio device
    [self.outputPopupButton selectItemAtIndex:outputIndex];
#else
    NSMutableArray * menuActions = [[NSMutableArray alloc] initWithCapacity:_deviceList.count];
    NSArray *currentOutputs = AVAudioSession.sharedInstance.currentRoute.outputs; //when is there ever more than 1 device output?
        
    for( int i = 0; i < _deviceList.count; i++)
    {
        AVAudioSessionPortDescription* port = [currentOutputs objectAtIndex:i];
        //UInt32 nameLen = 128; char deviceName[128] = "\0"; PBAudioDeviceName(_deviceList.devices[i], deviceName, &nameLen);
        
        //Create Popup Menu Action Items
        NSString * title = port.portName;
        UICommand* command = [UICommand commandWithTitle:title image:nil action:@selector(devicePopupButtonClicked:) propertyList:[NSNumber numberWithInt:i]];
        //UIAction* action = [UIAction actionWithHandler:outputDeviceMenuActionHandler];
        //action.title = port.portName;//[NSString stringWithString:deviceName];
        
        command.state = UIMenuElementStateOff;
        
        if(PBAudio.OutputStreams[0].audioDevice == i)
        {
            command.state = UIMenuElementStateOn;
            outputIndex = i;
        }
        
        [menuActions addObject:command];

    }


    self.outputDeviceMenu = [UIMenu menuWithChildren:menuActions];
    self.outputPopupButton.menu = self.outputDeviceMenu;
    
#endif
    return outputIndex;
}



-(void)createOutputPopupButton
{
#if TARGET_OS_OSX
    self.outputPopupButton = [[CocoaPopUpButton alloc] initWithFrame:CGRectZero pullsDown:NO];
    self.outputPopupButton.enabled = YES;
    [self.outputPopupButton setAction:@selector(devicePopupButtonClicked:)];
    self.outputPopupButton.target = self; //without this popup items will appear grayed out if no other ui to make first responder

    int outputIndex = [self updateOutputDeviceList];
    [self addSubview:self.outputPopupButton];
 
#else
    self.outputPopupButton = [[CocoaButton alloc] initWithFrame:CGRectZero];
    
    self.outputPopupButton.enabled = YES;
    
    self.outputPopupButton.titleLabel.textColor = CocoaColor.whiteColor;
    self.outputPopupButton.titleLabel.font = self.outputDeviceLabel.font;
    //[self.testButton setTarget:self];
    //[self.testButton setAction:@selector(testButtonClicked:)];
    //self.testButton.wantsLayer = YES;

    //self.testButton.backgroundColor = CocoaColor.darkGrayColor;
    self.outputPopupButton.layer.backgroundColor = CocoaColor.clearColor.CGColor;
    self.outputPopupButton.layer.borderColor = CocoaColor.grayColor.CGColor;
    self.outputPopupButton.layer.borderWidth = 0.5;
    self.outputPopupButton.layer.cornerRadius = 3.0f;
    
    int outputIndex = [self updateOutputDeviceList];
    
    self.outputPopupButton.showsMenuAsPrimaryAction        = YES;
    self.outputPopupButton.changesSelectionAsPrimaryAction = YES;

    
    [self.contentView addSubview:self.outputPopupButton];

#endif
    

    self.outputPopupButton.translatesAutoresizingMaskIntoConstraints = NO;
        
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.outputPopupButton
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:4.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.outputPopupButton
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.testButton attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:-6.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.outputPopupButton
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}


-(void)testButtonClicked:(id)sender
{
    NSLog(@"testButtonClicked");

    //For all active output streams...
    //For all active stream channels
    //Set their Output Pass to play a test tone
    //TO DO:  Send message to PbAudio Engine Process Event Loop instead of direct call
    //PbAudio.OutputStreams[0].outputpass = TestOutputPass;
    
    CMUniversalMessage * message = &pba_message_events[pba_message_event_index++]; pba_message_event_index = pba_message_event_index % MAX_MESSAGE_EVENTS;

    message->type                    = CMMessageTypeData128;
    //message->group                 = TestOutputPassID;

    message->data128.status          = CMSysExStatusComplete;
    message->data128.sysex8.streamID = 0;                                //additional stream id property
    
    //SYSEX: [F7, manufacturer id, channel id, device id, command id, param id, param value, F7]
    //message->data128.sysex8.data[0]  = 0x42;                           //eg Korg
    message->data128.sysex8.data[1]    = 0;                              //midi 1.0 sysex stream id
    //message->data128.sysex8.data[2]  = 0;                              //midi 1.0 systex audio device id
    message->data128.sysex8.data[3]    = pba_stream_change_output_pass;   //midi 1.0 sysex command id
    message->data128.sysex8.data[4]    = TestOutputPassID;               //midi 1.0 sysex param id
    message->data128.sysex8.data[5]    = 0;                              //midi 1.0 sysex param value

    
        
    struct kevent kev;
    EV_SET(&kev, message->type, EVFILT_USER, 0, NOTE_TRIGGER, 0, message);
    kevent(PBAudio.eventQueue.kq, &kev, 1, NULL, 0, NULL);
    
    //MCUApplicationDelegate *appDelegate = (MCUApplicationDelegate *)[[NSApplication sharedApplication] delegate];
    //[appDelegate dismissModalWindow];
}

#pragma mark -- Output Channel Control

-(void)createOutputChannelLabel
{
    self.outputChannelLabel = [[CocoaTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
#if TARGET_OS_OSX
    self.outputChannelLabel.focusRingType = NSFocusRingTypeDefault;
    self.outputChannelLabel.stringValue  = @"Active Output Channels:";
    self.outputChannelLabel.wantsLayer = YES;
    
    //self.outputDeviceLabel.lineBreakMode = NSLineBreakByClipping;
    //self.outputDeviceLabel.usesSingleLineMode = YES;
    //self.outputDeviceLabel.maximumNumberOfLines = YES;
    
    self.outputChannelLabel.editable = YES;
    self.outputChannelLabel.drawsBackground = YES;
    self.outputChannelLabel.layer.masksToBounds = NO;
#else
    self.outputChannelLabel.text  = @"Active Output Channels:";
#endif
    self.outputChannelLabel.textColor = CocoaColor.whiteColor;
    self.outputChannelLabel.backgroundColor = [CocoaColor clearColor];
    self.outputChannelLabel.layer.backgroundColor = [CocoaColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;

#if TARGET_OS_OSX
    self.outputChannelLabel.alignment = NSTextAlignmentRight;

    _outputChannelLabel.bezeled         = NO;
    _outputChannelLabel.editable        = NO;
    _outputChannelLabel.drawsBackground = NO;
    [self addSubview:self.outputChannelLabel];
#else
    self.outputChannelLabel.textAlignment = NSTextAlignmentRight;
    [self.contentView addSubview:self.outputChannelLabel];
#endif
    
    [self.outputChannelLabel sizeToFit];

    CGRect frame = self.outputChannelLabel.frame;

    self.outputChannelLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.outputChannelLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0.];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.outputChannelLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputDeviceLabel attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:10.];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.outputChannelLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.outputDeviceLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1.0 constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
    
}


-(void)createChannelScrollView
{
    self.channelScrollView = [[CocoaScrollView alloc] initWithFrame:CGRectMake(0,0,100,100)];
    //self.channelScrollView.backgroundColor = [NSColor grayColor];//[NSColor colorWithWhite:233./255. alpha:1.];
    
#if TARGET_OS_OSX
    [self addSubview:self.channelScrollView];
#else
    self.channelScrollView.layer.borderColor = CocoaColor.grayColor.CGColor;
    self.channelScrollView.layer.borderWidth = 0.5;
    self.channelScrollView.layer.cornerRadius = 3.0f;
    [self.contentView addSubview:self.channelScrollView];
#endif
    
    self.channelScrollView.translatesAutoresizingMaskIntoConstraints = NO;
    

    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.channelScrollView
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputPopupButton attribute:NSLayoutAttributeLeft
                                                               multiplier:1.0 constant:0];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.channelScrollView
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.testButton attribute:NSLayoutAttributeRight
                                                               multiplier:1.0 constant:0];
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.channelScrollView
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputChannelLabel attribute:NSLayoutAttributeTop
                                                               multiplier:1.0 constant:0];
    
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem: self.channelScrollView
                                                                attribute: NSLayoutAttributeHeight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeHeight
                                                               multiplier:0 constant:100.];
    
    [self addConstraints:@[Left, Right, Top, Height]];
    
    [self createChannelFilterView];
}

-(void)buttonToggledAtIndex:(int)index sender:(id)sender
{
    NSLog(@"AudioMidiSettingsView::NSCheckBoxView::buttonToggledAtIndex(%d)", index);
    
    //if( self.modalViewMode == ModalView_Modify ) self.submitButton.enabled = YES;
    if( sender == self.channelFilterView )
    {
        if( ((ToggleButton*)[self.channelFilterView.buttons objectAtIndex:index]).state == 1 )
        {
            //Enable Audio Device Stream
            //PBAudio.Start(&PBAudio.OutputStreams[index]);
        }
        else
        {
            //Disable Audio Device Stream
            //PBAudio.Stop(&PBAudio.OutputStreams[index]);
        }
    }
    else if( sender == self.midiInputFilterView )
    {
        
        //Enable\Disable Midi Input Connection
#if TARGET_OS_OSX
        bool enabled = ((ToggleButton*)[self.midiInputFilterView.buttons objectAtIndex:index]).state;
        NSLog(@"%@ Toggled: %d", ((ToggleButton*)[self.midiInputFilterView.buttons objectAtIndex:index]).title, enabled);
#else
        bool enabled = ((ToggleButton*)[self.midiInputFilterView.buttons objectAtIndex:index]).isOn;
        NSLog(@"%@ Toggled: %d", ((ToggleButton*)[self.midiInputFilterView.buttons objectAtIndex:index]).title, enabled);
#endif
        
        //Populate thruParam Inputs/Outputs
        //MIDIThruConnectionParams* thruParams = &(CMClient.thruConnections[CMClient.numThruConnections].params);
        //CMInitThruParams(thruParams, inputIDs, 1, outputIDs, 1);
        
        //struct CMDeviceDescription * deviceDescription = &(CMClient.hardwareDevices[CMClient.numHardwareDevices]);
        //CMDeviceType deviceType = surfaceType[0];
        
        //Use the CoreMIDI unique id of the input source endpoint for our DOM primary key
        NSString * sourceKey = [NSString stringWithFormat:@"%d", CMClient.sources[index].uniqueID];//[NSString stringWithUTF8String:CMClient.sources[inputIDs[0]].uniqueID]'
        //NSString * destKey = [NSString stringWithFormat:@"%d", destUniqueID];//[NSString stringWithUTF8String:CMClient.sources[inputIDs[0]].uniqueID]'
        
        //Send message to enable/disable a midi input connection
        CMUniversalMessage * message     = &pba_message_events[pba_message_event_index++]; pba_message_event_index = pba_message_event_index % MAX_MESSAGE_EVENTS;
        message->type                    = CMMessageTypeSystem;
        message->group                   = pba_midi_input_connection;
        message->system.status           = enabled ? CMStatusStart : CMStatusStop;
        message->system.uniqueID         = CMClient.sources[index].uniqueID;
            
        NSLog(@"sourceKey = %@", sourceKey);
        /*
        //TO DO:  introduce some error checking to this function pipeline
        CMInputConnection * inputConnection = [CMInputConnection createInputConnection:sourceKey];// Params:thruParams];
        //CMHardwareDevice* device = [CMHardwareDevice createHardwareDevice:sourceKey Type:deviceType];
        //assert(device);
        assert(inputConnection);
        assert(inputConnection.connection);
        //[device setConnection:thruConnection.connection];
        */
        
        struct kevent kev;
        EV_SET(&kev, message->type, EVFILT_USER, 0, NOTE_TRIGGER, 0, message);
        kevent(PBAudio.eventQueue.kq, &kev, 1, NULL, 0, NULL);
            
    }
    
}

-(void)createChannelFilterView
{
    int nMonoOutputs = 2;
    
    if(self.channelFilterView)
    {
        [self.channelFilterView removeFromSuperview];
        self.channelFilterView = nil;
#if TARGET_OS_OSX
        [self.channelScrollView setDocumentView:nil];

        [self.channelScrollView layout];
        [self.channelScrollView.documentView layout];

        [self.channelScrollView setNeedsLayout:YES];
        [self.channelScrollView setNeedsDisplay:YES];
#else


#endif

        
    }

#if TARGET_OS_OSX
    AudioObjectPropertyScope scope = kAudioObjectPropertyScopeOutput;
    nMonoOutputs = PBAudioDeviceChannelCount(PBAudio.OutputStreams[0].audioDevice, scope);
#endif
    
    NSMutableArray* stereoPairNames = [[NSMutableArray alloc] initWithCapacity:nMonoOutputs/2];
    
    
#if TARGET_OS_OSX

    //Iterate output channels to get each channel's name +Condense to channel pairs
    for (int i=0; i<nMonoOutputs; i+=2 )
    {
        OSStatus status;
        
        //char channelName[128] = "\0"; propertySize = 128;
        NSString * channelNameL; UInt32 propertySizeL = 0;
        NSString * channelNameR; UInt32 propertySizeR = 0;
        
        //The channels of a device are also its "elements", and the element value is the channel number starting with 1.
        //Since the channels are the elements, the appropriate AudioObjectProperty for the channel names is kAudioObjectPropertyElementName.
        //Here's the AudioDevicePropertyAddress that you need to use:
        
        AudioObjectPropertyAddress chPropertyAddressL = { kAudioObjectPropertyElementName, scope, i+1};
        AudioObjectPropertyAddress chPropertyAddressR = { kAudioObjectPropertyElementName, scope, i+2};

    
        
        status = AudioObjectGetPropertyDataSize(PBAudio.OutputStreams[0].audioDevice, &chPropertyAddressL, 0, nil, &propertySizeL); assert(status == noErr);
        status = AudioObjectGetPropertyDataSize(PBAudio.OutputStreams[0].audioDevice, &chPropertyAddressR, 0, nil, &propertySizeR); assert(status == noErr);
        
        status = AudioObjectGetPropertyData(PBAudio.OutputStreams[0].audioDevice, &chPropertyAddressL, 0, nil, &propertySizeL, &channelNameL); assert(status == noErr);
        status = AudioObjectGetPropertyData(PBAudio.OutputStreams[0].audioDevice, &chPropertyAddressR, 0, nil, &propertySizeR, &channelNameR); assert(status == noErr);
        
        //fprintf(stdout, "Channel %d Name:  %s\n", i+1, channelName);
        NSLog(@"Channel %d Name: %@\n", i+1, channelNameL);
        NSLog(@"Channel %d Name: %@\n", i+2, channelNameR);
        
        if( channelNameL.length == 0 ) channelNameL = [NSString stringWithFormat:@"Output %d", i+1];
        if( channelNameR.length == 0 ) channelNameR = [NSString stringWithFormat:@"Output %d", i+2];
        
        NSString *sharedPrefix = [channelNameL commonPrefixWithString:channelNameR options:NSLiteralSearch];
        
        //search backword in the shared prefix to look for     the last occurence of a space character
        NSRange range = [sharedPrefix rangeOfString:@" " options:NSBackwardsSearch];
        if(range.length > 0) sharedPrefix = [sharedPrefix substringToIndex:range.location+1];
        
        NSString *suffixL      = [channelNameL substringFromIndex:sharedPrefix.length];
        NSString *suffixR      = [channelNameR substringFromIndex:sharedPrefix.length];
        
        NSString * stereoPairName = [NSString stringWithFormat:@"%@%@ + %@", sharedPrefix, suffixL, suffixR];
        NSLog(@"Stereo Pair Name: %@", stereoPairName);
        
        [stereoPairNames addObject:stereoPairName];
    }
    
#else
    
    for( int i = 0; i < _deviceList.count; i++)
    {
        AVAudioSessionPortDescription* port = [AVAudioSession.sharedInstance.currentRoute.outputs objectAtIndex:i];
        //UInt32 nameLen = 128; char deviceName[128] = "\0"; PBAudioDeviceName(_deviceList.devices[i], deviceName, &nameLen);
        
        nMonoOutputs = port.channels.count;
        for( int c = 0; c < nMonoOutputs ; c+=2)
        {
            AVAudioSessionChannelDescription* channelL = [port.channels objectAtIndex:c];
            AVAudioSessionChannelDescription* channelR = [port.channels objectAtIndex:c+1];

            //char channelName[128] = "\0"; propertySize = 128;
            NSString * channelNameL = channelL.channelName; // UInt32 propertySizeL = 0;
            NSString * channelNameR = channelR.channelName; //UInt32 propertySizeR = 0;
            
            //fprintf(stdout, "Channel %d Name:  %s\n", i+1, channelName);
            NSLog(@"Channel %d Name: %@\n", i+1, channelNameL);
            NSLog(@"Channel %d Name: %@\n", i+2, channelNameR);
            
            if( channelNameL.length == 0 ) channelNameL = [NSString stringWithFormat:@"Output %d", i+1];
            if( channelNameR.length == 0 ) channelNameR = [NSString stringWithFormat:@"Output %d", i+2];
            
            NSString *sharedPrefix = [channelNameL commonPrefixWithString:channelNameR options:NSLiteralSearch];
            
            //search backword in the shared prefix to look for     the last occurence of a space character
            NSRange range = [sharedPrefix rangeOfString:@" " options:NSBackwardsSearch];
            if(range.length > 0) sharedPrefix = [sharedPrefix substringToIndex:range.location+1];
            
            NSString *suffixL      = [channelNameL substringFromIndex:sharedPrefix.length];
            NSString *suffixR      = [channelNameR substringFromIndex:sharedPrefix.length];
            
            NSString * stereoPairName = [NSString stringWithFormat:@"%@%@ + %@", sharedPrefix, suffixL, suffixR];
            NSLog(@"Stereo Pair Name: %@", stereoPairName);
            
            [stereoPairNames addObject:stereoPairName];
            
        }

        break;
        
    }
    
#endif

    self.channelFilterView = [[NSCheckboxView alloc] initWithIdentifiers:stereoPairNames Title:nil Justification:NSTextAlignmentVertical];
    self.channelFilterView.delegate = self;
    
    //self.channelFilterView.wantsLayer = true;
    //self.channelFilterView.layer.backgroundColor = [NSColor redColor].CGColor;
    
#if TARGET_OS_OSX
    for( CocoaButton * button in self.channelFilterView.buttons ) button.enabled = YES;
    for (int i=0; i<nMonoOutputs; i+=2 )
    {
        //check corresponding streams for active channel pairs
        if( PBAudio.OutputStreams[i].audioDevice == _deviceList.devices[self.outputPopupButton.indexOfSelectedItem]
           && PBAudio.OutputStreams[i].outputEnabled && PBAudio.OutputStreams[i].running)
        {
            ((CocoaButton*)[self.channelFilterView.buttons objectAtIndex:i/2]).state = NSControlStateValueOn;
        }
    }
    
    [self.channelScrollView setDocumentView:self.channelFilterView];
#else
    self.channelFilterView.backgroundColor = CocoaColor.clearColor;
    self.channelFilterView.layer.backgroundColor = CocoaColor.clearColor.CGColor;
    [self.channelScrollView addSubview:self.channelFilterView];
#endif
    
    self.channelFilterView.translatesAutoresizingMaskIntoConstraints = NO;
    //[self.channelFilterView setAutoresizesSubviews:NO];
    
    NSLayoutConstraint * sCenterX = [NSLayoutConstraint constraintWithItem:self.channelFilterView
                                                                 attribute: NSLayoutAttributeCenterX
                                                                 relatedBy:NSLayoutRelationEqual
                                                                    toItem:self.channelScrollView
                                                                 attribute:NSLayoutAttributeCenterX
                                                                multiplier:1.f constant:0];
    
    NSLayoutConstraint * swidth = [NSLayoutConstraint constraintWithItem:self.channelFilterView
                                                               attribute: NSLayoutAttributeWidth
                                                               relatedBy:NSLayoutRelationEqual
                                                                  toItem:self.channelScrollView
                                                               attribute:NSLayoutAttributeWidth
                                                              multiplier:1.0f constant:0];
    
    NSLayoutConstraint * stop = [NSLayoutConstraint constraintWithItem:self.channelFilterView
                                                             attribute: NSLayoutAttributeTop
                                                             relatedBy:NSLayoutRelationEqual
                                                                toItem:self.channelScrollView
                                                             attribute:NSLayoutAttributeTop
                                                            multiplier:1.0f constant:0.0f];
    
    NSLayoutConstraint * sheight = [NSLayoutConstraint constraintWithItem:self.channelFilterView
                                                                attribute: NSLayoutAttributeHeight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.channelScrollView
                                                                attribute:NSLayoutAttributeHeight
                                                               multiplier:0.0 constant:self.channelFilterView.frame.size.height];
    
    
    [self.channelScrollView addConstraints:@[stop, sCenterX, swidth]];
    
    //[self.channelScrollView.documentView setAutoResizesSubviews:NO];
    
    //necessary to trigger intrinsicContentSize on updates
    
#if TARGET_OS_OSX
    [self.channelFilterView layout];
    [self.channelScrollView layout];
    [self.channelScrollView.documentView layout];
    
    [self.channelFilterView setNeedsLayout:YES];
    [self.channelFilterView setNeedsDisplay:YES];
    
    [self.channelScrollView.documentView setNeedsLayout:YES];
    [self.channelScrollView.documentView setNeedsDisplay:YES];
    
    [self.channelScrollView setNeedsLayout:YES];
    [self.channelScrollView setNeedsDisplay:YES];
#endif
    
}

#pragma mark -- Sample Rate Control

-(void)createSampleRateLabel
{
    self.sampleRateLabel = [[CocoaTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
#if TARGET_OS_OSX
    self.sampleRateLabel.focusRingType = NSFocusRingTypeDefault;
    self.sampleRateLabel.stringValue  = @"Active Output Channels:";
    self.sampleRateLabel.wantsLayer = YES;
    
    //self.outputDeviceLabel.lineBreakMode = NSLineBreakByClipping;
    //self.outputDeviceLabel.usesSingleLineMode = YES;
    //self.outputDeviceLabel.maximumNumberOfLines = YES;
    
    self.sampleRateLabel.editable = YES;
    self.sampleRateLabel.drawsBackground = YES;
    self.sampleRateLabel.layer.masksToBounds = NO;
    
#else
    self.sampleRateLabel.text  = @"Active Output Channels:";
#endif
    
    self.sampleRateLabel.textColor = CocoaColor.whiteColor;
    self.sampleRateLabel.backgroundColor = [CocoaColor clearColor];
    self.sampleRateLabel.layer.backgroundColor = [CocoaColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    
#if TARGET_OS_OSX

    self.sampleRateLabel.alignment = NSTextAlignmentRight;
    
    _sampleRateLabel.bezeled         = NO;
    _sampleRateLabel.editable        = NO;
    _sampleRateLabel.drawsBackground = NO;
    [self addSubview:self.sampleRateLabel];
#else
    self.sampleRateLabel.textAlignment = NSTextAlignmentRight;
    [self.contentView addSubview:self.sampleRateLabel];
#endif
    [self.sampleRateLabel sizeToFit];

    CGRect frame = self.sampleRateLabel.frame;

    self.sampleRateLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.sampleRateLabel
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.outputChannelLabel attribute:NSLayoutAttributeLeft
                                                               multiplier:1 constant:0.];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.sampleRateLabel
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.channelScrollView attribute:NSLayoutAttributeBottom
                                                               multiplier:1.0 constant:10.];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.sampleRateLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.outputChannelLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1.0 constant:0];

    
    [self addConstraints:@[ Left, Top, Width ]];
    
#if TARGET_OS_OSX
    self.sampleRateLabel.stringValue  = @"Sample Rate:";
#else
    self.sampleRateLabel.text  = @"Sample Rate:";
#endif
}

-(int)updateSampleRateList
{
    int rateIndex = 0;
    Float64 sampleRate = 0.;
    int prevSampleRateCount = _nSampleRates;
    
    AudioObjectPropertyScope scope = kAudioObjectPropertyScopeOutput;

#if TARGET_OS_OSX
    [self.sampleRatePopupButton removeAllItems];
    
    //Ask [Pb]Audio for the count of available sample rates
    //Either Input or Output Scope will work here
    PBAudioDeviceNominalSampleRateCount(PBAudio.OutputStreams[0].audioDevice, scope, &_nSampleRates);
    
    UInt32 propertySize = _nSampleRates * sizeof(AudioValueRange);
    AudioObjectPropertyAddress srListAddress = {kAudioDevicePropertyAvailableNominalSampleRates, scope, kAudioObjectPropertyElementMain};
    
    //Allocate memory to store array of sample rates
    if( !_SampleRates || _nSampleRates != prevSampleRateCount)
    {
        if( _SampleRates ) free(_SampleRates); _SampleRates = NULL;
        _SampleRates = malloc(sizeof(AudioValueRange) * _nSampleRates); assert(_SampleRates);
    }
    OSStatus status = AudioObjectGetPropertyData(PBAudio.OutputStreams[0].audioDevice, &srListAddress, 0, nil, &propertySize, _SampleRates); assert(status == noErr);

    //Get the device's current sample rate
    PBAudioDeviceNominalSampleRate(PBAudio.OutputStreams[0].audioDevice, scope, &sampleRate);

    for( int i = 0; i < _nSampleRates; i++)
    {
        NSString * sampleRateString = [NSString stringWithFormat:@"%g Hz", _SampleRates[i].mMinimum];
        [self.sampleRatePopupButton addItemWithTitle:sampleRateString];
        
        //compare to the current sample rate
        if( memcmp(&sampleRate, &_SampleRates[i].mMinimum, sizeof(Float64)) == 0) rateIndex = i;
    }
    

    //select the index of active audio device
    [self.sampleRatePopupButton selectItemAtIndex:rateIndex];
#else
    
    NSMutableArray * menuActions = [[NSMutableArray alloc] initWithCapacity:_nSampleRates];
    //NSArray *rates = AVAudioSession.sharedInstance.currentRoute.outputs;
    

    _nSampleRates = sizeof(HardcodedSampleRates)/sizeof(Float64);
    
    //Allocate memory to store array of sample rates
    if( !_SampleRates || _nSampleRates != prevSampleRateCount)
    {
        if( _SampleRates ) free(_SampleRates); _SampleRates = NULL;
        _SampleRates = malloc(sizeof(AudioValueRange) * _nSampleRates); assert(_SampleRates);
    }
    
    //__block AudioValueRange* SampleRates = _SampleRates;
    //UIActionHandler sampleRateMenuActionHandler = ^(__kindof UITaggedAction *action)
    //{
    //    NSLog(@"SampleRate Menu::Action %@ selected (%d)!", action.title);
    //    PBAudioDeviceSetSampleRate(PBAudio.OutputStreams[0].audioDevice, kAudioObjectPropertyScopeOutput, _SampleRates[action.tag].mMinimum);
    //};
    
    //Get the device's current sample rate
    PBAudioDeviceNominalSampleRate(PBAudio.OutputStreams[0].audioDevice, scope, &sampleRate);
    
    NSLog(@"AVAudioSession.sampleRate = %g", sampleRate);
    
    for( int i = 0; i < _nSampleRates; i++)
    {
        //Populate sample rate array with hardcoded values
        _SampleRates[i] = (AudioValueRange){ HardcodedSampleRates[i], HardcodedSampleRates[i] };
        
        //Create Popup Menu Action Items
        NSString * title = [NSString stringWithFormat:@"%g Hz", _SampleRates[i].mMinimum];
        UICommand* command = [UICommand commandWithTitle:title image:nil action:@selector(sampleRatePopupButtonClicked:) propertyList:[NSNumber numberWithInt:i]];
        //UIAction* action = [UIAction actionWithHandler:sampleRateMenuActionHandler];
        //action.title = [NSString stringWithFormat:@"%g Hz", _SampleRates[i].mMinimum];
        command.state = UIMenuElementStateOff;
        
        if(memcmp(&_SampleRates[i].mMinimum, &sampleRate, sizeof(Float64)) == 0)
        {
            command.state = UIMenuElementStateOn;
            rateIndex = i;
        }
        
        [menuActions addObject:command];
    }

    //TO DO: Set button text rendered to reflect based on selected sample rate
    
    self.sampleRateMenu = [UIMenu menuWithChildren:menuActions];
    self.sampleRatePopupButton.menu = self.sampleRateMenu;
    
#endif
    
    return rateIndex;
}

-(void)sampleRatePopupButtonClicked:(id)sender
{

#if TARGET_OS_OSX
    if( sender == self.sampleRatePopupButton )
    {
        int selectedIndex = (int)self.sampleRatePopupButton.indexOfSelectedItem;
        PBAudioDeviceSetSampleRate(PBAudio.OutputStreams[0].audioDevice, kAudioObjectPropertyScopeOutput, _SampleRates[selectedIndex].mMinimum);

        [self updateBufferSizeList]; //must repopulate sample rate list strings with durations dependent on sample rate
    }
#else
    if( [sender isKindOfClass:[UICommand class]] )
    {
        UICommand * command = sender;
        NSNumber * selectedNum = command.propertyList;
        int selectedIndex = selectedNum.intValue;
        PBAudioDeviceSetSampleRate(PBAudio.OutputStreams[0].audioDevice, kAudioObjectPropertyScopeOutput, _SampleRates[selectedIndex].mMinimum);

        [self updateSampleRateList]; //Must recreate the menu to reselect items due to "Action\Command is immutable because it is a child of a menu"
        [self updateBufferSizeList]; //must repopulate sample rate list strings with durations dependent on sample rate

        //TO DO:  can't get sender to work for UILeaf
        //if( command.sender == self.sampleRatePopupButton )  assert(1==0);
    }
#endif
}

-(void)createSampleRatePopupButton
{
    _nSampleRates = 0;
    
#if TARGET_OS_OSX

    self.sampleRatePopupButton = [[CocoaPopUpButton alloc] initWithFrame:CGRectZero pullsDown:NO];
    self.sampleRatePopupButton.enabled = YES;
    [self.sampleRatePopupButton setAction:@selector(sampleRatePopupButtonClicked:)];
    self.sampleRatePopupButton.target = self; //without this popup items will appear grayed out if no other ui to make first responder
    
    int outputIndex = [self updateSampleRateList];
    [self addSubview:self.sampleRatePopupButton];
#else
    self.sampleRatePopupButton = [[CocoaButton alloc] initWithFrame:CGRectZero];
    
    self.sampleRatePopupButton.enabled = YES;
    
    self.sampleRatePopupButton.titleLabel.textColor = CocoaColor.whiteColor;
    self.sampleRatePopupButton.titleLabel.font = self.outputDeviceLabel.font;
    //[self.testButton setTarget:self];
    //[self.testButton setAction:@selector(testButtonClicked:)];
    //self.testButton.wantsLayer = YES;

    //self.testButton.backgroundColor = CocoaColor.darkGrayColor;
    self.sampleRatePopupButton.layer.backgroundColor = CocoaColor.clearColor.CGColor;
    self.sampleRatePopupButton.layer.borderColor = CocoaColor.grayColor.CGColor;
    self.sampleRatePopupButton.layer.borderWidth = 0.5;
    self.sampleRatePopupButton.layer.cornerRadius = 3.0f;
    
    int outputIndex = [self updateSampleRateList];

    self.sampleRatePopupButton.showsMenuAsPrimaryAction        = YES;
    self.sampleRatePopupButton.changesSelectionAsPrimaryAction = YES;

    [self.contentView addSubview:self.sampleRatePopupButton];
#endif
    

    
    self.sampleRatePopupButton.translatesAutoresizingMaskIntoConstraints = NO;
        
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.sampleRatePopupButton
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.sampleRateLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:4.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.sampleRatePopupButton
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.channelScrollView attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:0.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.sampleRatePopupButton
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.sampleRateLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}

#pragma mark -- Buffer Size Control

-(void)createBufferSizeLabel
{
    self.bufferSizeLabel = [[CocoaTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
#if TARGET_OS_OSX
    self.bufferSizeLabel.focusRingType = NSFocusRingTypeDefault;
    self.bufferSizeLabel.stringValue  = @"Active Output Channels:";
    self.bufferSizeLabel.wantsLayer = YES;
    
    //self.outputDeviceLabel.lineBreakMode = NSLineBreakByClipping;
    //self.outputDeviceLabel.usesSingleLineMode = YES;
    //self.outputDeviceLabel.maximumNumberOfLines = YES;
    
    self.bufferSizeLabel.editable = YES;
    self.bufferSizeLabel.drawsBackground = YES;
    self.bufferSizeLabel.layer.masksToBounds = NO;
#else
    self.bufferSizeLabel.text  = @"Active Output Channels:";
#endif
    
    self.bufferSizeLabel.textColor = CocoaColor.whiteColor;
    self.bufferSizeLabel.backgroundColor = [CocoaColor clearColor];
    self.bufferSizeLabel.layer.backgroundColor = [CocoaColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    
#if TARGET_OS_OSX
    self.bufferSizeLabel.alignment = NSTextAlignmentRight;
    
    _bufferSizeLabel.bezeled         = NO;
    _bufferSizeLabel.editable        = NO;
    _bufferSizeLabel.drawsBackground = NO;
    [self addSubview:self.bufferSizeLabel];
#else
    self.bufferSizeLabel.textAlignment = NSTextAlignmentRight;
    [self.contentView addSubview:self.bufferSizeLabel];
#endif
    [self.bufferSizeLabel sizeToFit];
    
    CGRect frame = self.bufferSizeLabel.frame;
    
    self.bufferSizeLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.bufferSizeLabel
                                                             attribute: NSLayoutAttributeLeft
                                                             relatedBy:NSLayoutRelationEqual
                                                                toItem: self.sampleRateLabel attribute:NSLayoutAttributeLeft
                                                            multiplier:1 constant:0.];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.bufferSizeLabel
                                                            attribute: NSLayoutAttributeTop
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.sampleRatePopupButton attribute:NSLayoutAttributeBottom
                                                           multiplier:1.0 constant:10.];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.bufferSizeLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.sampleRateLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1.0 constant:0];
    
    
    [self addConstraints:@[ Left, Top, Width ]];
    
#if TARGET_OS_OSX
    self.bufferSizeLabel.stringValue  = @"Buffer Size:";
#else
    self.bufferSizeLabel.text = @"Buffer Size:";
#endif
}

-(int)updateBufferSizeList
{
    int    sizeIndex  = 0;
    UInt32 bufferSize = 0;
    UInt32 tmpBufferSize = 0;
    Float64 sampleRate = 0.0;
    
#if TARGET_OS_OSX
    PBAudioDeviceNominalSampleRate(PBAudio.OutputStreams[0].audioDevice, kAudioObjectPropertyScopeOutput, &sampleRate);
    //int prevNumBufferSize = _nBufferSizes;

    [self.bufferSizePopupButton removeAllItems];
    
    //Ask [Pb]Audio for the count of available sample rates
    //Either Input or Output Scope will work here
    PBAudioDeviceBufferSizeRange(PBAudio.OutputStreams[0].audioDevice, &_minBufferSize, &_maxBufferSize);
    PBAudioDeviceBufferSize(PBAudio.OutputStreams[0].audioDevice, &bufferSize);
    
    //1 check if power of two
    //2 if not, find next power of two
    //provide sample rates for each step of minimum buffer size up to max
    assert(_minBufferSize > 0);
    _minBufferSizeStep = _minBufferSize;
    if( (_minBufferSize & (_minBufferSize - 1)) != 0 )
    {
        _minBufferSizeStep = 32;
        while ( _minBufferSizeStep < _minBufferSize ) _minBufferSizeStep *= 2;
        _minBufferSize = _minBufferSizeStep;
    }
    
    int i = 0;
    tmpBufferSize = _minBufferSize;
    while( tmpBufferSize < _maxBufferSize )
    {
        double duration = (double)tmpBufferSize / (double)sampleRate * 1000.;
        NSString * bufferSizeString = [NSString stringWithFormat:@"%u samples (%.1f ms)", (unsigned int)tmpBufferSize, (float)duration ];
        [self.bufferSizePopupButton addItemWithTitle:bufferSizeString];
        
        //compare to the current buffer size setting
        if( memcmp(&bufferSize, &tmpBufferSize, sizeof(UInt32)) == 0) sizeIndex = i;
     
        tmpBufferSize += _minBufferSizeStep;
        i++;
    }
    

    //select the index of active audio device
    [self.bufferSizePopupButton selectItemAtIndex:sizeIndex];
#else
    PBAudioDeviceNominalSampleRate(PBAudio.OutputStreams[0].audioDevice, kAudioObjectPropertyScopeOutput, &sampleRate);

    NSMutableArray * menuActions = [[NSMutableArray alloc] initWithCapacity:_nSampleRates];
    //NSArray *rates = AVAudioSession.sharedInstance.currentRoute.outputs;

    //Determine the count of available/preferred buffer sizes
    _minBufferSize = 32;   //documentation guarantees a minimum of 256 but could be lower
    _maxBufferSize = 4096; //this is the max according to the documentation

    //Get the active device's current buffer size
    PBAudioDeviceBufferSize(PBAudio.OutputStreams[0].audioDevice, &bufferSize);
    
    NSLog(@"AVAudioSession.bufferSize = %d", bufferSize);
    
    //1 check if power of two
    //2 if not, find next power of two
    //provide sample rates for each step of minimum buffer size up to max
    assert(_minBufferSize > 0);
    _minBufferSizeStep = _minBufferSize;
    if( (_minBufferSize & (_minBufferSize - 1)) != 0 )
    {
        _minBufferSizeStep = 32;
        while ( _minBufferSizeStep < _minBufferSize ) _minBufferSizeStep *= 2;
        _minBufferSize = _minBufferSizeStep;
    }
    
    int i = 0;
    tmpBufferSize = _minBufferSize;
    while( tmpBufferSize < _maxBufferSize )
    {
        double duration = (double)tmpBufferSize / (double)sampleRate * 1000.;
        NSString * bufferSizeString = [NSString stringWithFormat:@"%u samples (%.1f ms)", (unsigned int)tmpBufferSize, (float)duration ];
        
        UICommand* command = [UICommand commandWithTitle:bufferSizeString image:nil action:@selector(bufferSizePopupButtonClicked:) propertyList:[NSNumber numberWithInt:i]];
        //UIAction* action = [UIAction actionWithHandler:sampleRateMenuActionHandler];
        //action.title = [NSString stringWithFormat:@"%g Hz", _SampleRates[i].mMinimum];
        command.state = UIMenuElementStateOff;
        
        //compare to the current buffer size setting
        if( memcmp(&bufferSize, &tmpBufferSize, sizeof(UInt32)) == 0)
        {
            command.state = UIMenuElementStateOn;
            sizeIndex = i;
        }
        
        [menuActions addObject:command];

        tmpBufferSize += _minBufferSizeStep;
        i++;
    }
    
    self.bufferSizeMenu = [UIMenu menuWithChildren:menuActions];
    self.bufferSizePopupButton.menu = self.bufferSizeMenu;
    
#endif
    
    return sizeIndex;
}

-(void)bufferSizePopupButtonClicked:(id)sender
{
#if TARGET_OS_OSX

    if( sender == self.bufferSizePopupButton )
    {
        UInt32 tmpBufferSize = 0;
        int selectedIndex = (int)self.bufferSizePopupButton.indexOfSelectedItem;
        
        int i = 0;
        tmpBufferSize = _minBufferSize;
        while( tmpBufferSize < _maxBufferSize )
        {
            if( selectedIndex == i ) break;
            tmpBufferSize += _minBufferSizeStep;
            i++;
        }
        
        PBAudioDeviceSetBufferSize(PBAudio.OutputStreams[0].audioDevice, tmpBufferSize);
    }
#else
    if( [sender isKindOfClass:[UICommand class]] )
    {
        UICommand * command    = sender;
        NSNumber * selectedNum = command.propertyList;
        int selectedIndex      = selectedNum.intValue;

        UInt32 tmpBufferSize = 0;
        
        int i = 0;
        tmpBufferSize = _minBufferSize;
        while( tmpBufferSize < _maxBufferSize )
        {
            if( selectedIndex == i ) break;
            tmpBufferSize += _minBufferSizeStep;
            i++;
        }
        
        PBAudioDeviceSetBufferSize(PBAudio.OutputStreams[0].audioDevice, tmpBufferSize);
        
        //Must recreate the menu to reselect items due to "Action\Command is immutable because it is a child of a menu"
        [self updateBufferSizeList];
    }
#endif
}

-(void)createBufferSizePopupButton
{
    //_nSampleRates = 0;
#if TARGET_OS_OSX
    self.bufferSizePopupButton = [[CocoaPopUpButton alloc] initWithFrame:CGRectZero pullsDown:NO];
    self.bufferSizePopupButton.enabled = YES;
    [self.bufferSizePopupButton setAction:@selector(bufferSizePopupButtonClicked:)];
    self.bufferSizePopupButton.target = self; //without this popup items will appear grayed out if no other ui to make first responder
    
    int outputIndex = [self updateBufferSizeList];
    [self addSubview:self.bufferSizePopupButton];
#else
    self.bufferSizePopupButton = [[CocoaButton alloc] initWithFrame:CGRectZero];
    
    self.bufferSizePopupButton.enabled = YES;
    
    self.bufferSizePopupButton.titleLabel.textColor = CocoaColor.whiteColor;
    self.bufferSizePopupButton.titleLabel.font = self.outputDeviceLabel.font;

    self.bufferSizePopupButton.layer.backgroundColor = CocoaColor.clearColor.CGColor;
    self.bufferSizePopupButton.layer.borderColor = CocoaColor.grayColor.CGColor;
    self.bufferSizePopupButton.layer.borderWidth = 0.5;
    self.bufferSizePopupButton.layer.cornerRadius = 3.0f;
    
    int outputIndex = [self updateBufferSizeList];

    self.bufferSizePopupButton.showsMenuAsPrimaryAction        = YES;
    self.bufferSizePopupButton.changesSelectionAsPrimaryAction = YES;

    [self.contentView addSubview:self.bufferSizePopupButton];
#endif
    
    self.bufferSizePopupButton.translatesAutoresizingMaskIntoConstraints = NO;
        
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.bufferSizePopupButton
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.bufferSizeLabel attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:4.f];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.bufferSizePopupButton
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.sampleRatePopupButton attribute:NSLayoutAttributeRight
                                                               multiplier:1 constant:0.f];
    
    NSLayoutConstraint * CenterY = [NSLayoutConstraint constraintWithItem: self.bufferSizePopupButton
                                                                attribute: NSLayoutAttributeCenterY
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.bufferSizeLabel attribute:NSLayoutAttributeCenterY
                                                               multiplier:1.0 constant:0.f];
    

    
    [self addConstraints:@[ CenterY, Left, Right ]];
}


#pragma mark -- Midi Input Control

-(void)createMidiInputLabel
{
    self.midiInputLabel = [[CocoaTextField alloc] initWithFrame:CGRectMake(0,200,200,100)];
    
#if TARGET_OS_OSX
    
    self.midiInputLabel.focusRingType = NSFocusRingTypeDefault;
    self.midiInputLabel.stringValue  = @"Active Output Channels:";
    self.midiInputLabel.wantsLayer = YES;
    
    
    //self.outputDeviceLabel.lineBreakMode = NSLineBreakByClipping;
    //self.outputDeviceLabel.usesSingleLineMode = YES;
    //self.outputDeviceLabel.maximumNumberOfLines = YES;
    
    self.midiInputLabel.editable = YES;
    self.midiInputLabel.drawsBackground = YES;
    self.midiInputLabel.layer.masksToBounds = NO;
#else
    self.midiInputLabel.text  = @"Active Output Channels:";
#endif
    
    self.midiInputLabel.textColor = CocoaColor.whiteColor;
    self.midiInputLabel.backgroundColor = [CocoaColor clearColor];
    self.midiInputLabel.layer.backgroundColor = [CocoaColor clearColor].CGColor;
    //self.descriptionLabel.delegate = self;
    
#if TARGET_OS_OSX
    self.midiInputLabel.alignment = NSTextAlignmentRight;
    
    _midiInputLabel.bezeled         = NO;
    _midiInputLabel.editable        = NO;
    _midiInputLabel.drawsBackground = NO;
    [self addSubview:self.midiInputLabel];
#else
    self.midiInputLabel.textAlignment = NSTextAlignmentRight;
    [self.contentView addSubview:self.midiInputLabel];
#endif
    
    [self.midiInputLabel sizeToFit];
    
    CGRect frame = self.midiInputLabel.frame;
    
    self.midiInputLabel.translatesAutoresizingMaskIntoConstraints = NO;
    
    
    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.midiInputLabel
                                                             attribute: NSLayoutAttributeLeft
                                                             relatedBy:NSLayoutRelationEqual
                                                                toItem: self.bufferSizeLabel attribute:NSLayoutAttributeLeft
                                                            multiplier:1 constant:0.];
    
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.midiInputLabel
                                                            attribute: NSLayoutAttributeTop
                                                            relatedBy:NSLayoutRelationEqual
                                                               toItem: self.bufferSizePopupButton attribute:NSLayoutAttributeBottom
                                                           multiplier:1.0 constant:10.];
    
    NSLayoutConstraint * Width = [NSLayoutConstraint constraintWithItem: self.midiInputLabel
                                                              attribute: NSLayoutAttributeWidth
                                                              relatedBy:NSLayoutRelationEqual
                                                                 toItem: self.bufferSizeLabel attribute:NSLayoutAttributeWidth
                                                             multiplier:1.0 constant:0];
    
    
    [self addConstraints:@[ Left, Top, Width ]];
    
#if TARGET_OS_OSX
    self.midiInputLabel.stringValue  = @"Active MIDI Inputs:";
#else
    self.midiInputLabel.text         = @"Active MIDI Inputs:";
#endif
}


-(void)createMidiInputScrollView
{
    self.midiInputScrollView = [[CocoaScrollView alloc] initWithFrame:CGRectMake(0,0,100,100)];
    //self.midiInputScrollView.backgroundColor = [NSColor grayColor];//[NSColor colorWithWhite:233./255. alpha:1.];
    
#if TARGET_OS_OSX
    [self addSubview:self.midiInputScrollView];
#else
    self.midiInputScrollView.scrollEnabled = YES;
    self.midiInputScrollView.userInteractionEnabled = YES;
    
    self.midiInputScrollView.layer.borderColor = CocoaColor.grayColor.CGColor;
    self.midiInputScrollView.layer.borderWidth = 0.5;
    self.midiInputScrollView.layer.cornerRadius = 3.0f;
    [self.contentView addSubview:self.midiInputScrollView];
#endif
    
    self.midiInputScrollView.translatesAutoresizingMaskIntoConstraints = NO;

    NSLayoutConstraint * Left = [NSLayoutConstraint constraintWithItem: self.midiInputScrollView
                                                                attribute: NSLayoutAttributeLeft
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.bufferSizePopupButton attribute:NSLayoutAttributeLeft
                                                               multiplier:1.0 constant:0];
    
    NSLayoutConstraint * Right = [NSLayoutConstraint constraintWithItem: self.midiInputScrollView
                                                                attribute: NSLayoutAttributeRight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.testButton attribute:NSLayoutAttributeRight
                                                               multiplier:1.0 constant:0];
    
    NSLayoutConstraint * Top = [NSLayoutConstraint constraintWithItem: self.midiInputScrollView
                                                                attribute: NSLayoutAttributeTop
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self.midiInputLabel attribute:NSLayoutAttributeTop
                                                               multiplier:1.0 constant:0];
    
    NSLayoutConstraint * Height = [NSLayoutConstraint constraintWithItem: self.midiInputScrollView
                                                                attribute: NSLayoutAttributeHeight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem: self attribute:NSLayoutAttributeHeight
                                                               multiplier:0 constant:100.];
    
    [self addConstraints:@[Left, Right, Top, Height]];
    
    [self createMidiInputFilterView];
}


-(void)createMidiInputFilterView
{

    if(self.midiInputFilterView)
    {
        [self.midiInputFilterView removeFromSuperview];
        self.midiInputFilterView = nil;
#if TARGET_OS_OSX

        [self.midiInputScrollView setDocumentView:nil];
        
        [self.midiInputFilterView layout];
        [self.midiInputScrollView.documentView layout];
        
        [self.midiInputFilterView setNeedsLayout:YES];
        [self.midiInputFilterView setNeedsDisplay:YES];
#else


#endif
    }
    
    CMUpdateInputDevices(); if( CMClient.numSources < 1) return;
    
    NSMutableArray* inputDeviceNames = [[NSMutableArray alloc]      initWithCapacity:CMClient.numSources];
    NSDictionary*   sourceIDMap      = [[NSMutableDictionary alloc] initWithCapacity:CMClient.numSources];
    
    int srcIndex = 0;
    for( int i = 0; i < CMClient.numSources; i++)
    {
        NSString * deviceName = [NSString stringWithUTF8String:   CMClient.sources[i].name];
        NSString * sourceID   = [NSString stringWithFormat:@"%d", CMClient.sources[i].uniqueID];
        [inputDeviceNames addObject:deviceName];
        [sourceIDMap setValue:[NSNumber numberWithInt:i] forKey:sourceID];
    }
    
    
    self.midiInputFilterView = [[NSCheckboxView alloc] initWithIdentifiers:inputDeviceNames Title:nil Justification:NSTextAlignmentVertical];
    self.midiInputFilterView.delegate = self;
    
    //self.channelFilterView.wantsLayer = true;
    //self.channelFilterView.layer.backgroundColor = [NSColor redColor].CGColor;
    //for( NSButton * button in self.midiInputFilterView.buttons ) button.enabled = YES;

    //tick checkbox buttons that correspond to active CMidi input connections
    for (int i=0; i<CMClient.numInputConnections; i++ )
    {
        //Use the temporary deviceIDMap that was just created to look for the index of the corresponding checkbox for a Midi Source's UniqueID
        NSString * sourceID    = [NSString stringWithFormat:@"%d", CMClient.inputConnections[i].source.uniqueID];
        NSNumber * buttonIndex = [sourceIDMap objectForKey:sourceID];
        
        //if button index is null that means the application created a midi input connection but the source was removed (ie the device was turned off)
        //but CMidi input connection mapping/registration is maintained so if the source device is turned back on it will reappear as active in the list
#if TARGET_OS_OSX
        if(buttonIndex) ((CocoaButton*)[self.midiInputFilterView.buttons objectAtIndex:buttonIndex.intValue]).state = NSControlStateValueOn;
#else
        
#endif
    }


#if TARGET_OS_OSX
    [self.midiInputScrollView setDocumentView:self.midiInputFilterView];
#else
    self.midiInputScrollView.backgroundColor = CocoaColor.clearColor;
    self.midiInputScrollView.layer.backgroundColor = CocoaColor.clearColor.CGColor;
    [self.midiInputScrollView addSubview:self.midiInputFilterView];
#endif
    
    self.midiInputFilterView.translatesAutoresizingMaskIntoConstraints = NO;
    //[self.midiInputFilterView setAutoresizesSubviews:NO];
    
    NSLayoutConstraint * sCenterX = [NSLayoutConstraint constraintWithItem:self.midiInputFilterView
                                                                 attribute: NSLayoutAttributeCenterX
                                                                 relatedBy:NSLayoutRelationEqual
                                                                    toItem:self.midiInputScrollView
                                                                 attribute:NSLayoutAttributeCenterX
                                                                multiplier:1.f constant:0];
    
    NSLayoutConstraint * swidth = [NSLayoutConstraint constraintWithItem:self.midiInputFilterView
                                                               attribute: NSLayoutAttributeWidth
                                                               relatedBy:NSLayoutRelationEqual
                                                                  toItem:self.midiInputScrollView
                                                               attribute:NSLayoutAttributeWidth
                                                              multiplier:1.0f constant:0];
    
    NSLayoutConstraint * stop = [NSLayoutConstraint constraintWithItem:self.midiInputFilterView
                                                             attribute: NSLayoutAttributeTop
                                                             relatedBy:NSLayoutRelationEqual
                                                                toItem:self.midiInputScrollView
                                                             attribute:NSLayoutAttributeTop
                                                            multiplier:1.0f constant:0.0f];
    
    NSLayoutConstraint * sheight = [NSLayoutConstraint constraintWithItem:self.midiInputFilterView
                                                                attribute: NSLayoutAttributeHeight
                                                                relatedBy:NSLayoutRelationEqual
                                                                   toItem:self.midiInputScrollView
                                                                attribute:NSLayoutAttributeHeight
                                                               multiplier:0.0 constant:self.midiInputFilterView.frame.size.height];
    
    
    [self.midiInputScrollView addConstraints:@[stop, sCenterX, swidth]];
    //[self.midiInputScrollView.documentView setAutoResizesSubviews:NO];
    
#if TARGET_OS_OSX

    //necessary to trigger intrinsicContentSize on updates?
    [self.midiInputScrollView layout];
    [self.midiInputFilterView layout];
    [self.midiInputScrollView.documentView layout];
    
    [self.midiInputScrollView setNeedsLayout:YES];
    [self.midiInputScrollView setNeedsDisplay:YES];
    
    [self.midiInputScrollView.documentView setNeedsLayout:YES];
    [self.midiInputScrollView.documentView setNeedsDisplay:YES];
    
    [self.midiInputFilterView setNeedsLayout:YES];
    [self.midiInputFilterView setNeedsDisplay:YES];
#else
    [self.midiInputScrollView setNeedsLayout];
    [self.midiInputFilterView setNeedsLayout];

    [self.midiInputScrollView setNeedsDisplay];
    [self.midiInputFilterView setNeedsDisplay];

    NSLog(@"MidiInputFterView.size = (%g, %g)", self.midiInputFilterView.frame.size.width, self.midiInputFilterView.frame.size.height);
    self.midiInputScrollView.contentSize = self.midiInputFilterView.intrinsicContentSize;

#endif
    
}

-(void)updateMidiInputDeviceList
{
    [self createMidiInputFilterView]; //repopulate the midi input source device list
}

/*
- (void)drawRect:(CGRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    // Drawing code here.
}
*/

@end
