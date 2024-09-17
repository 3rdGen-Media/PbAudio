//
//  AppDelegate.m
//  [Pb] Audio
//
//  Created by Joe Moulton on 6/28/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#import "PbAudioAppDelegate.h"

//#import "CRAppDelegate.h"
//#import <dlfcn.h>
//#include "CoreRender/crPlatform.h"

#if TARGET_OS_OSX
//#import "CRWindowController.h"
//#import "CRMenu.h"
//#import "CRAppInterface.h"
#import "PbAudioAppMenu.h"
#else

#endif


//PbAudio
#import "PbAudioAppInterface.h"

//View
//#import "MCUControlProView.h"
//#import "MCUExtenderView.h"
//#import "MCUCommanderView.h"

//#import "MCUDeviceQueueViewController.h"
//#import "C4CommanderViewController.h"
#import "MainWindowViewController.h"

//Modal

#import "AudioMidiSettingsView.h"
#import "DeviceManagerView.h"
#import "ProxyManagerView.h"
#import "CMProxyModalView.h"
#import "ControlManagerView.h"
#import "DisplayManagerView.h"
#import "SettingsModalViewController.h"

//Button
#import "CustomButtonView.h"

//AppIcon Generation
#import "CocoaImage+Logo.h"

#import "NSString+Ext.h"

static NSString * const NSToolbarForwardItemIdentifier  = @"ForwardToolbarItem";
static NSString * const NSToolbarBackwardItemIdentifier = @"BackwardToolbarItem";
static NSString * const NSToolbarAddItemIdentifier      = @"AddToolbarItem";


@interface PbAudioAppDelegate ()
{
    //CMThruView * _thruView;
    //CMThruScrollListView * _scrollListView;
}

//@property (nonatomic, retain) CustomButtonView* createThruButton;
@property (nonatomic, retain) NSTitlebarAccessoryViewController * accessoryViewController;

@property NSStatusItem                                    * barItem ;

@property (nonatomic, retain) NSToolbar                   * toolbar;
@property (nonatomic, retain) NSToolbar                   * modalToolbar;

@property (nonatomic, retain) CocoaView                   * rootView;
@property (nonatomic, retain) MainWindowViewController    * rootViewController;    //root VC

@property (nonatomic, retain) NSView                      * modalView;
@property (nonatomic, retain) SettingsModalViewController * modalViewController;

@property (nonatomic, retain) NSImage                     * statusOnImage;
@property (nonatomic, retain) NSImage                     * statusOffImage;

@end

@implementation PbAudioAppDelegate


+ (id)sharedInstance
{
    static PbAudioAppDelegate *appDelegate = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        appDelegate = [[self alloc] init];
    });
    return appDelegate;
}


-(void)openSettingsManagerModalWindow
{
    NSLog(@"openSettingsManagerModalWindow");
    
    //Create a view and view controller for the data model as an overlay window, popover or sheet
    CGSize popoverSize = CGSizeMake(500, 450); //ToonTrack Window Size
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    self.modalView = [[AudioMidiSettingsView alloc] initWithFrame:viewFrame];// andActiveDevice:self.rootViewController.deviceView.device];
    self.modalViewController = [[SettingsModalViewController alloc] initWithView:self.modalView];
    self.modalView.translatesAutoresizingMaskIntoConstraints = NO;
    self.modalView.autoresizingMask = NSViewNotSizable;
    
    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    [self.rootViewController presentViewControllerAsModalWindow:self.modalViewController];
    
    [self.modalView.window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable]; // | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    self.modalView.window.title = [NSString stringWithFormat:@"Audio/Midi Setup"];
    //self.modalView.titleLabel.stringValue = [NSString stringWithFormat:@"New Midi Route"];
    
    [self.modalView.window makeKeyAndOrderFront:self];     // Show the window

}

-(void)openProxyConnectionModalWindow
{
    NSLog(@"openProxyConnectionModalWindow");
 
    //Create a view and view controller for the data model as an overlay window, popover or sheet
    CGSize popoverSize = CGSizeMake(550, 550);
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    NSView * modalView = [[CMProxyModalView alloc] initWithFrame:viewFrame andProxy:nil];
    NSViewController * modalVC = [[SettingsModalViewController alloc] initWithView:modalView];
    
    modalView.translatesAutoresizingMaskIntoConstraints = NO;
    modalView.autoresizingMask = NSViewHeightSizable;
    
    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    [self.modalViewController presentViewControllerAsModalWindow:modalVC];

    [modalView.window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    modalView.window.title = [NSString stringWithFormat:@"New Proxy Connection"];

    
}

/*
-(void)openDeviceManagerModalWindow
{
    NSLog(@"openDeviceManagerModalWindow");
    
    //Create a view and view controller for the data model as an overlay window, popover or sheet
    CGSize popoverSize = CGSizeMake(550, 550);
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    self.modalView = [[DeviceManagerView alloc] initWithFrame:viewFrame andActiveDevice:self.rootViewController.deviceView.device];
    self.modalViewController = [[SettingsModalViewController alloc] initWithView:self.modalView];
    self.modalView.translatesAutoresizingMaskIntoConstraints = NO;
    self.modalView.autoresizingMask = NSViewNotSizable;
    
    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    [self.rootViewController presentViewControllerAsModalWindow:self.modalViewController];
    
    [self.modalView.window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable]; // | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    self.modalView.window.title = [NSString stringWithFormat:@"Control Surfaces"];
    //self.modalView.titleLabel.stringValue = [NSString stringWithFormat:@"New Midi Route"];
    
    [self.modalView.window makeKeyAndOrderFront:self];     // Show the window

}

-(void)openActiveDeviceWindow
{
    [self.window makeKeyAndOrderFront:self];
    [self.window orderFrontRegardless];
    if( self.modalView ) [self.modalWindow orderFrontRegardless];
}

-(void)openDeviceManagerModalWindow:(NSString*)deviceID
{
    NSLog(@"openDeviceManagerModalWindow: %@", deviceID);
 
    //Create a view and view controller for the data model as an overlay window, popover or sheet
    CGSize popoverSize = CGSizeMake(550, 550);
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    self.modalView = [[DeviceManagerView alloc] initWithFrame:viewFrame andActiveDevice:self.rootViewController.deviceView.device];
    self.modalViewController = [[SettingsModalViewController alloc] initWithView:self.modalView];
    self.modalView.translatesAutoresizingMaskIntoConstraints = NO;
    self.modalView.autoresizingMask = NSViewHeightSizable;
    

    //[self.modalWindow setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable]; // | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    self.modalWindow.title = [NSString stringWithFormat:@"Control Surfaces"];
    //self.modalView.titleLabel.stringValue = [NSString stringWithFormat:@"New Midi Route"];


    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    //[self.rootViewController presentViewControllerAsModalWindow:self.modalViewController];

    [self.modalWindow setContentViewController:self.modalViewController];
    [self.modalWindow makeKeyAndOrderFront:self];
    [self.modalWindow orderFrontRegardless];
    
}


-(void)createModalToolbar
{
    self.modalToolbar = [[NSToolbar alloc] init];

    // Toolbar **need1s** a delegate
    self.modalToolbar.delegate = self;
    
    // Assign the toolbar to the window object
    self.modalWindow.toolbar = self.modalToolbar;
}


-(void)openProxyConnectionModalWindow
{
    NSLog(@"openProxyConnectionModalWindow");
 
    //Create a view and view controller for the data model as an overlay window, popover or sheet
    CGSize popoverSize = CGSizeMake(550, 550);
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    NSView * modalView = [[CMProxyModalView alloc] initWithFrame:viewFrame andProxy:nil];
    NSViewController * modalVC = [[SettingsModalViewController alloc] initWithView:modalView];
    
    modalView.translatesAutoresizingMaskIntoConstraints = NO;
    modalView.autoresizingMask = NSViewHeightSizable;
    
    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    [self.modalViewController presentViewControllerAsModalWindow:modalVC];

    [modalView.window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    modalView.window.title = [NSString stringWithFormat:@"New Proxy Connection"];

    
}

-(void)openProxyConnectionModalWindow:(NSString*)proxyID
{
    NSLog(@"openProxyConnectionModalWindow: %@", proxyID);
 
    //Create a view and view controller for the data model as an overlay window, popover or sheet
    CGSize popoverSize = CGSizeMake(550, 550);
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    NSView * modalView = [[ProxyManagerView alloc] initWithFrame:viewFrame];// andProxy:proxyID];
    NSViewController * modalVC = [[SettingsModalViewController alloc] initWithView:modalView];
    
    modalView.translatesAutoresizingMaskIntoConstraints = NO;
    modalView.autoresizingMask = NSViewHeightSizable;
    
    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    [self.modalViewController presentViewControllerAsModalWindow:modalVC];

    [modalView.window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    modalView.window.title = [NSString stringWithFormat:@"Edit Proxy Connection"];

    
}

-(void)openProxyManagerModalWindow:(NSString*)proxyID
{
    NSLog(@"openProxyManagerModalWindow: %@", proxyID);
 
    //Create a view and view controller for the data model as an overlay window, popover or sheet
    CGSize popoverSize = CGSizeMake(1024, 768);
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    self.modalView = [[ProxyManagerView alloc] initWithFrame:viewFrame];// andProxy:proxyID];
    self.modalViewController = [[SettingsModalViewController alloc] initWithView:self.modalView];
    self.modalView.translatesAutoresizingMaskIntoConstraints = NO;
    self.modalView.autoresizingMask = NSViewHeightSizable;
    
    //NSUInteger windowStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskFullSizeContentView ;
    //[self.modalWindow setStyleMask:windowStyleMask];//NSWindowStyleMaskTitled | NSWindowStyleMaskClosable]; // | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    self.modalWindow.title = [NSString stringWithFormat:@"Proxy Connections"];
    //self.modalView.titleLabel.stringValue = [NSString stringWithFormat:@"New Midi Route"];

    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    //[self.rootViewController presentViewControllerAsModalWindow:self.modalViewController];

    [self.modalWindow setContentViewController:self.modalViewController];
    [self.modalWindow makeKeyAndOrderFront:self];
    [self.modalWindow orderFrontRegardless];
    
}

-(void)openControlManagerModalWindow:(CMControl*)control
{
    NSLog(@"openControlManagerModalWindow: %d", control->type);
    
    //Create a view and view controller for the data model as an overlay window, popover or sheet
    CGSize popoverSize = CGSizeMake(550, 550);
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    self.modalView = [[ControlManagerView alloc] initWithFrame:viewFrame andControl:control];
    self.modalViewController = [[SettingsModalViewController alloc] initWithView:self.modalView];
    self.modalView.translatesAutoresizingMaskIntoConstraints = NO;
    self.modalView.autoresizingMask = NSViewHeightSizable;
    
    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    //[self.rootViewController presentViewControllerAsModalWindow:self.modalViewController];

    //[self.modalView.window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable]; // | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    self.modalWindow.title = [NSString stringWithFormat:@"Control Mapping"];
    //self.modalView.titleLabel.stringValue = [NSString stringWithFormat:@"New Midi Route"];
    
    [self.modalWindow setContentViewController:self.modalViewController];
    [self.modalWindow makeKeyAndOrderFront:self];
    [self.modalWindow orderFrontRegardless];
    
}

-(void)openDisplayManagerModalWindow:(CMDisplay*)display
{
    NSLog(@"openDisplayManagerModalWindow");
    
    //Create a view and view controller for the data model as an overlay window, popover or sheet
    CGSize popoverSize = CGSizeMake(550, 550);
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    self.modalView = [[DisplayManagerView alloc] initWithFrame:viewFrame andDisplay:display];
    self.modalViewController = [[SettingsModalViewController alloc] initWithView:self.modalView];
    self.modalView.translatesAutoresizingMaskIntoConstraints = NO;
    self.modalView.autoresizingMask = NSViewHeightSizable;
    
    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    //[self.rootViewController presentViewControllerAsModalWindow:self.modalViewController];
    
    //[self.modalView.window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable]; // | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    self.modalWindow.title = [NSString stringWithFormat:@"Display Mapping"];
    //self.modalView.titleLabel.stringValue = [NSString stringWithFormat:@"New Midi Route"];
    
    [self.modalWindow setContentViewController:self.modalViewController];
    [self.modalWindow makeKeyAndOrderFront:self];
    [self.modalWindow orderFrontRegardless];
}
*/


-(void)dismissModalWindow
{

    //dismiss the current view controller modal window/view controller, if present, before recreating the current view controller
    //if( [self.rootViewController.presentedViewControllers containsObject:self.modalViewController] )
    //{
    //    NSLog(@"dismissModalWindow");
    //    [self.modalViewController dismissViewController:self.modalViewController];
    //}
    
    [self.modalWindow orderOut:nil];
    
    //Bastard Modal Window left our menu items disabled
    //Call custom fix:
    //[[CMMenu sharedInstance] enableAllMenuItems];
    
    //dealloc modaView/VC
    self.modalView = nil;
    self.modalViewController = nil;
    
}

/*
-(CMHardwareDevice*)activeDevice
{
    return self.rootViewController.deviceView.device;
}

-(void)setActiveDevice:(CMHardwareDevice*)device
{
    [self.rootViewController setActiveDevice:(CMHardwareDevice*)device];
}


-(void)setDevice:(CMHardwareDevice*)device DisplayAtIndex:(int)displayIndex
{
    [self.rootViewController setDevice:(device ? device : self.rootViewController.deviceView.device) DisplayAtIndex:displayIndex];
}

-(void)setDevice:(CMHardwareDevice*)device DisplayForControlType:(CMControlType)controlType atIndex:(CMControlIndex)controlIndex
{
    [self.rootViewController setDevice:(device ? device : self.rootViewController.deviceView.device) DisplayForControlType:controlType atIndex:controlIndex];
    
}

-(void)setDevice:(CMHardwareDevice*)device ControlType:(CMControlType)controlType atIndex:(CMControlIndex)controlIndex
{
    [self.rootViewController setDevice:(device ? device : self.rootViewController.deviceView.device) ControlType:controlType atIndex:controlIndex];
}


//-(MCUDeviceView*)activeDeviceView
//{
//    return self.rootViewController ;
//}

-(void)addRouteButtonClicked:(id)sender
{
    
    NSLog(@"addRouteButtonClicked");
}


- (NSArray<NSToolbarItemIdentifier> *)toolbarDefaultItemIdentifiers:(NSToolbar *)toolbar
{
    if( toolbar == self.toolbar )
        return @[NSToolbarBackwardItemIdentifier, NSToolbarForwardItemIdentifier, NSToolbarAddItemIdentifier];
    else if( toolbar == self.modalToolbar)
        return @[NSToolbarAddItemIdentifier];
    
    return nil;
}

- (NSArray<NSToolbarItemIdentifier> *)toolbarAllowedItemIdentifiers:(NSToolbar *)toolbar
{
    if( toolbar == self.toolbar )
        return @[NSToolbarBackwardItemIdentifier, NSToolbarForwardItemIdentifier, NSToolbarAddItemIdentifier];
    else if( toolbar == self.modalToolbar)
        return @[NSToolbarAddItemIdentifier];
    
    return nil;
}

- (NSToolbarItem *)toolbar:(NSToolbar *)toolbar itemForItemIdentifier:(NSToolbarItemIdentifier)itemIdentifier willBeInsertedIntoToolbar:(BOOL)flag
{
    if( toolbar == self.toolbar )
    {
        
        if( [itemIdentifier localizedCompare:NSToolbarBackwardItemIdentifier] == NSOrderedSame)
        {
            NSButton * button = [NSButton buttonWithImage:[NSImage imageNamed:NSImageNameGoBackTemplate] target:self action: @selector(GoBackwardButtonClicked)];
            button.bezelStyle = NSBezelStyleTexturedRounded;
            button.frame = CGRectMake(0,0,button.frame.size.height,button.frame.size.height);
            
            //self.createThruButton = [self createButtonViewWithImage:[NSImage imageNamed:NSImageNameAddTemplate]];
            return [self customToolbarItem:NSToolbarBackwardItemIdentifier label:nil paletteLabel:@"Previous Device" toolTip:nil itemContent:button];
        }
        else if( [itemIdentifier localizedCompare:NSToolbarForwardItemIdentifier] == NSOrderedSame)
        {
            NSButton * button = [NSButton buttonWithImage:[NSImage imageNamed:NSImageNameGoForwardTemplate] target:self action: @selector(GoForwardButtonClicked)];
            button.bezelStyle = NSBezelStyleTexturedRounded;
            button.frame = CGRectMake(0,0,button.frame.size.height,button.frame.size.height);
            
            //self.createThruButton = [self createButtonViewWithImage:[NSImage imageNamed:NSImageNameAddTemplate]];
            return [self customToolbarItem:NSToolbarForwardItemIdentifier label:nil paletteLabel:@"Next Device" toolTip:nil itemContent:button];
        }
        else if( [itemIdentifier localizedCompare:NSToolbarAddItemIdentifier] == NSOrderedSame)
        {
            NSButton * button = [NSButton buttonWithImage:[NSImage imageNamed:NSImageNameTouchBarAddTemplate] target:self action: @selector(AddItemButtonClicked)];
            button.bezelStyle = NSBezelStyleTexturedRounded;
            button.frame = CGRectMake(0,0,button.frame.size.height,button.frame.size.height);
            
            //self.createThruButton = [self createButtonViewWithImage:[NSImage imageNamed:NSImageNameAddTemplate]];
            return [self customToolbarItem:NSToolbarAddItemIdentifier label:nil paletteLabel:@"Add Device" toolTip:nil itemContent:button];
        }
    }
    else if( toolbar == self.modalToolbar)
    {
        if( [itemIdentifier localizedCompare:NSToolbarAddItemIdentifier] == NSOrderedSame)
        {
            NSButton * button = [NSButton buttonWithImage:[NSImage imageNamed:NSImageNameAddTemplate] target:self action: @selector(openProxyConnectionModalWindow)];
            button.bezelStyle = NSBezelStyleTexturedRounded;
            button.frame = CGRectMake(0,0,button.frame.size.height,button.frame.size.height);
            
            //self.createThruButton = [self createButtonViewWithImage:[NSImage imageNamed:NSImageNameAddTemplate]];
            return [self customToolbarItem:NSToolbarAddItemIdentifier label:@"New" paletteLabel:@"New Proxy Connection" toolTip:@"Create a New Proxy Connection" itemContent:button];
        }
    }

    
    return nil;
    
}

-(void)GoBackwardButtonClicked
{
    [self.rootViewController showPrevDevice];

}


-(void)GoForwardButtonClicked
{
    [self.rootViewController showNextDevice];
    



}
*/

-(void)showPopupMenu
{
    /*
    NSEvent* event = [NSEvent otherEventWithType: NSEventTypeApplicationDefined
                                                         location: NSMakePoint(0,MCUApplicationDelegate.sharedInstance.window.contentView.safeAreaInsets.top)
                                                   modifierFlags: 0
                                                       timestamp: 0.0
                                                    windowNumber: 0
                                                         context: nil
                                                         subtype: 0
                                                           data1: 0
                                                           data2: 0];
    */
    //[NSMenu popUpContextMenu:[MCUCommanderMainMenu sharedInstance] withEvent:NSApp.currentEvent forView:MCUApplicationDelegate.sharedInstance.window.contentView];
}

-(void)AddItemButtonClicked
{
    //[self.rootViewController showNextDevice];
    //[self openDeviceManagerModalWindow:nil];

    [self showPopupMenu];
}

-(void) cbvTouchUpInside:(id)sender
{
    /*
    if( sender == self.createThruButton )
    {
        NSLog(@"cbvTouchUpInside");
        //[self openThruConnectionModalWindow];
    }
     */
}
     
/**
Mostly base on Apple sample code: https://developer.apple.com/documentation/appkit/touch_bar/integrating_a_toolbar_and_touch_bar_into_your_app
*/

-(CustomButtonView*)createButtonViewWithImage:(NSImage*)image
{
    CustomButtonView * button = [[CustomButtonView alloc] initWithFrame:CGRectMake(0,20,image.size.width * 2., image.size.height * 1.5)];
    
    NSString * title = @"";
    button.title = title;
    button.selectedTitle = title;
    //[button sizeToFit];
    
    button.wantsLayer = YES;
    button.layer.borderWidth = 1;
    button.layer.borderColor = [NSColor grayColor].CGColor;
    button.layer.cornerRadius = 4;
    //button.layer.masksToBounds = YES;
    //button.layer.maskedCorners = YES;
    
    button.layer.borderColor = [CocoaColor clearColor].CGColor;
    
    button.translatesAutoresizingMaskIntoConstraints = NO;
    button.layer.backgroundColor = [CocoaColor clearColor].CGColor;
    button.normalBackgroundColor = [CocoaColor clearColor];
    button.highlightedBackgroundColor = [CocoaColor lightGrayColor];
    button.selectedBackgroundColor = [CocoaColor redColor];
    //button.backgroundImageView.contentMode
    
    button.delegate = self;
    
    button.titleColor = [CocoaColor whiteColor];
    button.highlightedTitleColor = [CocoaColor grayColor];
    button.selectedTitleColor = button.titleColor;
    
    button.titleLabel.backgroundColor = [CocoaColor clearColor];
    
    button.layer.backgroundColor = [CocoaColor clearColor].CGColor;
    if(image)
        button.backgroundImage = image;
    
    return button;
}

-(NSToolbarItem*) customToolbarItem:(NSToolbarItemIdentifier)itemIdentifier
       label:(NSString*)label
       paletteLabel:(NSString*)paletteLabel
       toolTip:(NSString*)toolTip
       itemContent: (NSButton*)itemContent
{
       
    NSToolbarItem* toolbarItem = [[NSToolbarItem alloc] initWithItemIdentifier:itemIdentifier];
    toolbarItem.label = label;
    toolbarItem.paletteLabel = paletteLabel;
    toolbarItem.toolTip = toolTip;
    toolbarItem.target = self;
    toolbarItem.action = @selector(addRouteButtonClicked:);

    //[toolbarItem.label sizeToFit];
    //CGSize size = [label sizeWithAttributes:[NSAttributedStringKey.font: font])

    //minSize is deprecated but it is the only mechanism I can get to size the toolbar item properly
    //toolbarItem.minSize = CGSizeMake(itemContent.frame.size.height,itemContent.frame.size.height);    // maybe + x
    
    //set toolbar view
    toolbarItem.view = itemContent;
    //toolbarItem.view.wantsLayer = YES;
    //toolbarItem.view.layer.backgroundColor = NSColor.darkGrayColor.CGColor;
    //toolbarItem.image = itemContent.image;
    
    // We actually need an NSMenuItem here, so we construct one.
    NSMenuItem * menuItem = [[NSMenuItem alloc] init];
    menuItem.submenu = nil;
    menuItem.title = paletteLabel;
    toolbarItem.menuFormRepresentation = menuItem;

    //toolbarItem.rou
    return toolbarItem;
}

-(void)createToolbar
{
    self.toolbar = [[NSToolbar alloc] init];
    // Toolbar **needs** a delegate
    self.toolbar.delegate = self;
    
    // Assign the toolbar to the window object
    self.window.toolbar = self.toolbar;
    //self.window.titleVisibility = NSWindowTitleHidden;
    //window.setFrameAutosaveName("Main Window")
    //window.contentView = NSHostingView(rootView: contentView)
    //window.makeKeyAndOrderFront(nil)
}

-(void) createAccessoryViewController
{
    self.accessoryViewController = [[NSTitlebarAccessoryViewController alloc] init];
    //self.accessoryViewController.view = accessoryView;
    self.accessoryViewController.layoutAttribute = NSLayoutAttributeRight;
    [self.window addTitlebarAccessoryViewController:self.accessoryViewController];
}

/*
-(void)createWindow
{
    if( !self.window )
    {
#if TARGET_OS_OSX
    
    CGFloat aspect = 1071./1556.;//1.69;
    CGFloat height = NSScreen.mainScreen.frame.size.height * 0.88;
    CGFloat width  = aspect * height;
    NSLog(@"Creating Window");
    NSRect contentSize = NSMakeRect(0,0, width, height);
    NSUInteger windowStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskFullSizeContentView ;
    self.window = [[NSWindow alloc] initWithContentRect:contentSize styleMask:windowStyleMask backing:NSBackingStoreBuffered defer:YES];
    //self.window.frame = CGRectMake(NSScreen.mainScreen.frame.size.width/2. - width/2., NSScreen.mainScreen.frame.size.height/2. -  height/2., width, height);
    [self.window center];
    self.window.backgroundColor = [NSColor whiteColor];
    //self.window.title = @"C4 Commander";

    self.window.appearance = [NSAppearance appearanceNamed: NSAppearanceNameVibrantDark];
    self.window.titlebarAppearsTransparent = YES;
    self.window.titlebarSeparatorStyle = NSTitlebarSeparatorStyleNone;
        
    [self.window setCollectionBehavior: NSWindowCollectionBehaviorTransient];
    //[self.window setLevel:NSPopUpMenuWindowLevel];
        
    [self createToolbar];
    
    //NSButton *closeButton = [self.window standardWindowButton:NSWindowCloseButton]; // Get the existing close button of the window. Check documentation for the other window buttons.
    //NSView *titleBarView = closeButton.superview; // Get the view that encloses that standard window buttons.
    //NSButton *myButton = …; // Create custom button to be added to the title bar.
    //myButton.frame = …; // Set the appropriate frame for your button. Use titleBarView.bounds to determine the bounding rect of the view that encloses the standard window buttons.
    //[titleBarView addSubview:myButton]; // Add the custom button to the title bar.
#else

    //on ios >= 13.0 we'll defer window creation to the scene view delegate methods
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.backgroundColor = [UIColor purpleColor];
#endif
    }
}
*/


-(void)createWindow
{
    if( !self.window )
    {
#if TARGET_OS_OSX
    
    //SetWindowModeState(&_windowMode, 0, 0); //set initial state to WINDOWED
        
    NSRect         contentSize = NSScreen.mainScreen.frame;//NSMakeRect(500.0, 500.0, 1000.0, 1000.0);
    //NSUInteger windowStyleMask = styleMaskForWindowMode(_windowMode);
    const NSUInteger windowStyleMask   = NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable |NSWindowStyleMaskMiniaturizable;

    self.window = [[NSWindow alloc] initWithContentRect:contentSize styleMask:windowStyleMask backing:NSBackingStoreBuffered defer:YES];
    self.window.title = @"Main Window"; //self.window.titlebarAppearsTransparent = YES;
    self.window.backgroundColor = nil;//[NSColor blackColor]; //Use magenta to identify/debug platform window default layer, black for release builds
    [self.window makeKeyAndOrderFront:self];
#else
    self.window = [[CRWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.backgroundColor = nil;//[UIColor magentaColor]; //Use magenta to identify/debug platform window default layers, nil for release builds
#endif
    }
}


-(void)createModalWindow
{
    if( !self.modalWindow )
    {
#if TARGET_OS_OSX
    
    //CGFloat aspect = 1071./1556.;//1.69;
    CGFloat height = 1024.;//NSScreen.mainScreen.frame.size.height * 0.88;
    CGFloat width  = 768.;
    NSLog(@"Creating Secondary Window");
    NSRect contentSize = NSMakeRect(0,0, width, height);
    NSUInteger windowStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskFullSizeContentView;///*NSWindowStyleMaskResizable | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | */ NSWindowStyleMaskFullSizeContentView ;
    //NSUInteger windowStyleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskResizable | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskFullSizeContentView ;
    self.modalWindow = [[NSWindow alloc] initWithContentRect:contentSize styleMask:windowStyleMask backing:NSBackingStoreBuffered defer:YES];
    //self.window.frame = CGRectMake(NSScreen.mainScreen.frame.size.width/2. - width/2., NSScreen.mainScreen.frame.size.height/2. -  height/2., width, height);
    [self.modalWindow center];
    //self.modalWindow.backgroundColor = [NSColor whiteColor];
    //self.window.title = @"C4 Commander";

    self.modalWindow.appearance = [NSAppearance appearanceNamed: NSAppearanceNameVibrantDark];
    self.modalWindow.titlebarAppearsTransparent = YES;
    self.modalWindow.titlebarSeparatorStyle = NSTitlebarSeparatorStyleNone;
        
    [self.modalWindow setCollectionBehavior: NSWindowCollectionBehaviorTransient];
    [self.modalWindow setReleasedWhenClosed:NO];
    //[self.window setLevel:NSPopUpMenuWindowLevel];
        
    //[self createToolbar];
    //[self createModalToolbar];
        
        /*
        NSButton *closeButton = [self.window standardWindowButton:NSWindowCloseButton]; // Get the existing close button of the window. Check documentation for the other window buttons.
        NSView *titleBarView = closeButton.superview; // Get the view that encloses that standard window buttons.
        NSButton *myButton = …; // Create custom button to be added to the title bar.
        myButton.frame = …; // Set the appropriate frame for your button. Use titleBarView.bounds to determine the bounding rect of the view that encloses the standard window buttons.
        [titleBarView addSubview:myButton]; // Add the custom button to the title bar.
         */
#else

    //on ios >= 13.0 we'll defer window creation to the scene view delegate methods
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
    self.window.backgroundColor = [UIColor purpleColor];
#endif
    }
}

/*
-(void)createRootViewController
{
#if TARGET_OS_OSX
    NSRect windowRect = CGRectMake(0,0, self.window.contentView.frame.size.width, self.window.contentView.frame.size.height);
    self.rootView = [[NSView alloc] initWithFrame:windowRect];//[[MCUControlProView alloc] initWithFrame:windowRect];
    self.rootViewController = [[MainWindowViewController alloc] initWithView:self.rootView];
    //self.rootViewController.view = self.rootView;
    
    //Note to self:
    //
    //  If [[NSWindow setContentViewController:] is used then the Window will resize itself based on the view rather than visa versa
    //  However, this seems to prevent autolayout from resizing a scroll view within the root view appropriately so to avoid
    //  I am avoiding this routine, setting the window content view to be the root view manually, and having the view resize based on the window
    //
    [self.window setContentViewController:self.rootViewController];
    //[self.window.contentView addSubview:self.rootViewController.view];
    //self.rootView.translatesAutoresizingMaskIntoConstraints = NO; //Window will not resize unless this is set to NO
        
#else
    self.rootViewController = nil;
    
    //create the camera video view controller
    self.rootViewController = [[CMThruViewController alloc] init];

    self.rootViewController.view.backgroundColor = [UIColor purpleColor];
    self.window.rootViewController = self.rootViewController;
#endif
    
    
}
*/

-(void)createRootViewController
{
    //if you set the pointer to nil here and no other strong references to the camera view controller are in memory
    //the object will deallocate itself, so by using this line you can run this function over and over to recreate your root view controller
    //and navigation stack if needed
    self.rootViewController = nil;
    
    //create the camera video view controller
    self.rootViewController = [[MainWindowViewController alloc] initWithWindow:self.window];
    
#if TARGET_OS_OSX
    //NSRect windowRect = CGRectMake(0,0, self.window.contentView.frame.size.width, self.window.contentView.frame.size.height);
    //self.rootView = [[NSView alloc] initWithFrame:windowRect];//[[MCUControlProView alloc] initWithFrame:windowRect];
    //self.rootViewController = [[MCUDeviceQueueViewController alloc] initWithView:self.rootView];

    //Note to self:
    //
    //  If [[NSWindow setContentViewController:] is used then the Window will resize itself based on the view rather than visa versa
    //  However, this seems to prevent autolayout from resizing a scroll view within the root view appropriately so to avoid
    //  I am avoiding this routine, setting the window content view to be the root view manually, and having the view resize based on the window
    //
    [self.window setContentViewController:self.rootViewController];

#else
    self.window.rootViewController = self.rootViewController;
#endif

    
}

#if TARGET_OS_OSX
-(void)setColorSpaceProfile:(NSColorSpace*)colorSpace
{
    //This is imperative for OpenGL pipelines whereas Metal layers attached to windows set the correct color profile on the window internally
    // 1)  Set the color space on the window
    // 2)  Notify listening services that the window color space has changed
    // 3)  Update window content view (may only be mandatory if setting window.contentView instead of window.contentViewController)
    [self.window setColorSpace:colorSpace];
    [[NSNotificationCenter defaultCenter] postNotificationName:NSWindowDidChangeScreenNotification object:self.window];
    [self.window.contentView viewDidChangeBackingProperties];// In some cases this additional call is
}
#endif

#if TARGET_OS_OSX



#pragma mark -- Create Menu

-(void)setMenuTitle:(NSString*)title
{
#if TARGET_OS_OSX
    NSMenuItem* menuItem = [[NSApp mainMenu] itemAtIndex:0];
    NSMenu *menu = [[[NSApp mainMenu] itemAtIndex:0] submenu];
    //NSString *title = @"Core Render";
    // Append some invisible character to title :)
    
    //NSFont* font = menu.font;

    title = [title stringByAppendingString:@"\x1b"];
    [menu setTitle:title];
    
    NSFont * newFont = menu.font;//[NSFont menuBarFontOfSize:0];
    NSDictionary *attributes = @{ NSFontAttributeName: newFont, NSForegroundColorAttributeName: [NSColor greenColor] };
    
    NSMutableAttributedString *attrTitle = [[NSMutableAttributedString alloc] initWithString:@"[Pb] Audio" attributes:attributes];
    //[attrTitle addAttribute:NSFontAttributeName value:menu.font range:NSMakeRange(0, title.length)];
    //[menu setTitle:attrTitle];
    //[menu setTitle:title];
    [menuItem setAttributedTitle:attrTitle];
    //[menuItem.submenu changeMenuFont:newFont];
    //[[menu setTitleColor:[NSColor redColor]];
    
    //[[NSApp mainMenu] setTitle:title];
    //NSFont* newFont = [NSFont boldSystemFontOfSize:font.pointSize];
    //[submenu setFont:newFont];
#else
    
#endif

}


-(void)createMenuBar
{
#if TARGET_OS_OSX
    [NSApp setMainMenu:[[PbAudioAppMenu alloc] init]];
#else
    
#endif
}


-(void)createStatusBarMenu
{
    self.statusOnImage = [self statusBarIcon:1];
    self.statusOffImage = [self statusBarIcon:0];

    self.barItem = [NSStatusBar.systemStatusBar statusItemWithLength:NSVariableStatusItemLength];
    //self.barItem.button.title = @"MCU";
    //self.barItem.button.font = [NSFont boldSystemFontOfSize:NSFont.systemFontSize];
    self.barItem.button.image = self.statusOffImage;
    self.barItem.button.action = @selector(barItemAction);
    //[self.barItem setAction:@selector(barItemAction)];
    //self.barItem.menu = [MCUCommanderMainMenu sharedInstance];

}



#pragma mark -- Init Delegate

-(id)init
{
    if(self = [super init]) {
        
        NSLog(@"PbAudioAppDelegate::init");

        //[self createWindow];
        
        // Setup Preference Menu Action/Target on MainMenu
        [self createMenuBar];
        
        //CRView * view = [[CRView alloc] initWithFrame:self.window.frame];
        //[self.window.contentView addSubview:view];
        // Create a view
        //view = [[NSTabView alloc] initWithFrame:CGRectMake(0, 0, 700, 700)];
        
    }
    return self;
}


#endif

#pragma mark -- OSX Delegate Methods

#if TARGET_OS_OSX
- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
    //Hold up NSApplication::terminate: from being called so we can notify CoreTransport to clean up and it can notify NSApplication when finished so that shutdown can proceed

    /*
    struct kevent kev;
    EV_SET(&kev, CTProcessEvent_Exit, EVFILT_USER, EV_ADD | EV_ENABLE | EV_ONESHOT, NOTE_FFCOPY|NOTE_TRIGGER|0x1, 0, NULL);
    kevent(CTProcessEventQueue.kq, &kev, 1, NULL, 0, NULL);
    */
    
    return NSTerminateNow;//NSTerminateLater;
    
}


-(void)updateThruConnectionWidgets
{
    //[((C4CommanderView*)(self.rootView)) updateThruConnectionWidgets];
}

-(void)barItemAction {
    
    if( CMDevice.documents.count < 1 )
    {
        if( self.modalWindow.isVisible )
           [self dismissModalWindow];
        //else
            //[self openDeviceManagerModalWindow:nil];
    }
    else
    {
        if(self.modalWindow.isVisible || self.window.isVisible )
        {
            if(self.modalWindow.isVisible) [self dismissModalWindow];
            if(self.window.isVisible) [self.window orderOut:nil];
        }
        else
        {
            if(self.modalWindow.isVisible) [self dismissModalWindow];
            //[self openActiveDeviceWindow];
        }
    }
    
    if( self.barItem.button.image == self.statusOnImage )
        self.barItem.button.image = self.statusOffImage;
    else
        self.barItem.button.image = self.statusOnImage;
}

-(NSImage*)statusBarIcon:(BOOL)state
{
    //CGFloat imgScale = 15./75.;
    CGFloat imageWidth = 18;// * 2.0;
    CGFloat imageHeight = 18.;
    
    CGSize imgSize = CGSizeMake(1024, 1024);
    NSImage *image = [[NSImage alloc] initWithSize:NSMakeSize( imageWidth, imageHeight )];
    //[image setScalesWhenResized:YES];

    CocoaImage * logoImage;
    
    if(state > 0 ) logoImage = [CocoaImage logoImage:imgSize];
    else logoImage = [CocoaImage logoImage2:imgSize];

    // Lock focus of image, make it a destination for drawing
    [image lockFocus];

    // Set background color to white
    [[NSColor clearColor] set];
    NSRectFill( NSMakeRect( 0, 0, imageWidth, imageHeight ));
    //CGPoint center = CGPointMake(imageWidth/2. / scale, imageHeight/2./scale);
    // Construct and draw path as you would in drawRect:

    //CGContextTranslateCTM(context, self.bounds.size.width/2. - circleWidth2/2.0, self.bounds.size.height/2. - circleWidth2/2.0);
    //CGContextRef context = NSGraphicsContext.currentContext.CGContext;
    //CGContextScaleCTM(context, imgScale, imgScale);
    //[[NSBezierPath bezierPathWithRect:fullBounds] setClip];

    /***************************/
    
    CGFloat yOffset = 6./54. * imageHeight;
    
    [logoImage drawInRect:CGRectMake(0, -yOffset* 0.7,imageWidth, imageHeight)];

    
    
    /********************/
    
    [image unlockFocus];

    // Now draw in a view by compositing
    //[image compositeToPoint:NSZeroPoint operation:NSCompositeCopy];

    return image;
    
    /*
    NSBezierPath* bezier772Path = [NSBezierPath bezierPath];//WithRect:CGRectMake(0, 0, 75, 75)];
    

    
    [bezier772Path moveToPoint: NSMakePoint(70.93, 37.89)];
    [bezier772Path curveToPoint: NSMakePoint(74.88, 45.6) controlPoint1: NSMakePoint(70.93, 41.07) controlPoint2: NSMakePoint(72.5, 43.86)];
    [bezier772Path curveToPoint: NSMakePoint(73.58, 50.18) controlPoint1: NSMakePoint(73.45, 52.14) controlPoint2: NSMakePoint(73.58, 50.18)];
    [bezier772Path curveToPoint: NSMakePoint(66.82, 53.58) controlPoint1: NSMakePoint(71.04, 50.34) controlPoint2: NSMakePoint(68.59, 51.49)];
    [bezier772Path curveToPoint: NSMakePoint(65.42, 63.61) controlPoint1: NSMakePoint(64.4, 56.48) controlPoint2: NSMakePoint(63.98, 60.38)];
    [bezier772Path curveToPoint: NSMakePoint(62.37, 66.53) controlPoint1: NSMakePoint(64.46, 64.64) controlPoint2: NSMakePoint(63.44, 65.61)];
    [bezier772Path curveToPoint: NSMakePoint(51.28, 69.16) controlPoint1: NSMakePoint(58.6, 64.91) controlPoint2: NSMakePoint(54.06, 65.85)];
    [bezier772Path curveToPoint: NSMakePoint(49.17, 73.88) controlPoint1: NSMakePoint(50.11, 70.56) controlPoint2: NSMakePoint(49.42, 72.2)];
    [bezier772Path curveToPoint: NSMakePoint(47.43, 74.44) controlPoint1: NSMakePoint(48.59, 74.06) controlPoint2: NSMakePoint(48.03, 74.28)];
    [bezier772Path curveToPoint: NSMakePoint(44.83, 75) controlPoint1: NSMakePoint(46.56, 74.68) controlPoint2: NSMakePoint(45.69, 74.83)];
    [bezier772Path curveToPoint: NSMakePoint(43.59, 73.66) controlPoint1: NSMakePoint(44.45, 74.53) controlPoint2: NSMakePoint(44.07, 74.06)];
    [bezier772Path curveToPoint: NSMakePoint(30.1, 74.84) controlPoint1: NSMakePoint(39.53, 70.26) controlPoint2: NSMakePoint(33.5, 70.79)];
    [bezier772Path curveToPoint: NSMakePoint(30, 74.96) controlPoint1: NSMakePoint(30.06, 74.88) controlPoint2: NSMakePoint(30.04, 74.92)];
    [bezier772Path curveToPoint: NSMakePoint(24.33, 73.36) controlPoint1: NSMakePoint(28.06, 74.57) controlPoint2: NSMakePoint(26.17, 74.04)];
    [bezier772Path curveToPoint: NSMakePoint(21.04, 67.29) controlPoint1: NSMakePoint(24.02, 71.08) controlPoint2: NSMakePoint(22.94, 68.89)];
    [bezier772Path curveToPoint: NSMakePoint(11.67, 65.63) controlPoint1: NSMakePoint(18.34, 65.03) controlPoint2: NSMakePoint(14.77, 64.53)];
    [bezier772Path curveToPoint: NSMakePoint(8.86, 62.75) controlPoint1: NSMakePoint(10.69, 64.72) controlPoint2: NSMakePoint(9.76, 63.76)];
    [bezier772Path curveToPoint: NSMakePoint(6.18, 51.77) controlPoint1: NSMakePoint(10.4, 58.99) controlPoint2: NSMakePoint(9.46, 54.52)];
    [bezier772Path curveToPoint: NSMakePoint(1.32, 49.65) controlPoint1: NSMakePoint(4.75, 50.57) controlPoint2: NSMakePoint(3.06, 49.88)];
    [bezier772Path curveToPoint: NSMakePoint(0.6, 47.4) controlPoint1: NSMakePoint(1.07, 48.9) controlPoint2: NSMakePoint(0.8, 48.16)];
    [bezier772Path curveToPoint: NSMakePoint(0.18, 45.45) controlPoint1: NSMakePoint(0.42, 46.74) controlPoint2: NSMakePoint(0.32, 46.1)];
    [bezier772Path curveToPoint: NSMakePoint(0.42, 45.27) controlPoint1: NSMakePoint(0.26, 45.39) controlPoint2: NSMakePoint(0.34, 45.33)];
    [bezier772Path curveToPoint: NSMakePoint(1.6, 31.77) controlPoint1: NSMakePoint(4.48, 41.87) controlPoint2: NSMakePoint(5.01, 35.83)];
    [bezier772Path curveToPoint: NSMakePoint(0, 30.35) controlPoint1: NSMakePoint(1.13, 31.21) controlPoint2: NSMakePoint(0.56, 30.77)];
    [bezier772Path curveToPoint: NSMakePoint(1.23, 25.55) controlPoint1: NSMakePoint(0.32, 28.72) controlPoint2: NSMakePoint(0.71, 27.11)];
    [bezier772Path curveToPoint: NSMakePoint(6.19, 23.41) controlPoint1: NSMakePoint(2.99, 25.32) controlPoint2: NSMakePoint(4.72, 24.64)];
    [bezier772Path curveToPoint: NSMakePoint(8.79, 12.28) controlPoint1: NSMakePoint(9.51, 20.62) controlPoint2: NSMakePoint(10.43, 16.07)];
    [bezier772Path curveToPoint: NSMakePoint(11.73, 9.29) controlPoint1: NSMakePoint(9.71, 11.23) controlPoint2: NSMakePoint(10.7, 10.24)];
    [bezier772Path curveToPoint: NSMakePoint(21.7, 7.87) controlPoint1: NSMakePoint(14.96, 10.7) controlPoint2: NSMakePoint(18.83, 10.28)];
    [bezier772Path curveToPoint: NSMakePoint(25.06, 1.38) controlPoint1: NSMakePoint(23.73, 6.17) controlPoint2: NSMakePoint(24.84, 3.82)];
    [bezier772Path curveToPoint: NSMakePoint(27.64, 0.55) controlPoint1: NSMakePoint(25.91, 1.09) controlPoint2: NSMakePoint(26.76, 0.79)];
    [bezier772Path curveToPoint: NSMakePoint(28.54, 0.36) controlPoint1: NSMakePoint(27.94, 0.47) controlPoint2: NSMakePoint(28.25, 0.43)];
    [bezier772Path curveToPoint: NSMakePoint(35.76, 4.78) controlPoint1: NSMakePoint(30.1, 2.78) controlPoint2: NSMakePoint(32.68, 4.51)];
    [bezier772Path curveToPoint: NSMakePoint(44.9, 0) controlPoint1: NSMakePoint(39.6, 5.11) controlPoint2: NSMakePoint(43.1, 3.14)];
    [bezier772Path curveToPoint: NSMakePoint(48.68, 0.95) controlPoint1: NSMakePoint(46.18, 0.25) controlPoint2: NSMakePoint(47.44, 0.57)];
    [bezier772Path curveToPoint: NSMakePoint(49.16, 3.14) controlPoint1: NSMakePoint(48.76, 1.68) controlPoint2: NSMakePoint(48.9, 2.42)];
    [bezier772Path curveToPoint: NSMakePoint(61.44, 8.86) controlPoint1: NSMakePoint(50.98, 8.11) controlPoint2: NSMakePoint(56.48, 10.67)];
    [bezier772Path curveToPoint: NSMakePoint(62.39, 8.46) controlPoint1: NSMakePoint(61.77, 8.74) controlPoint2: NSMakePoint(62.08, 8.61)];
    [bezier772Path curveToPoint: NSMakePoint(66.1, 12.12) controlPoint1: NSMakePoint(62.39, 8.46) controlPoint2: NSMakePoint(61.85, 7.34)];
    [bezier772Path curveToPoint: NSMakePoint(68.05, 22.8) controlPoint1: NSMakePoint(64.52, 15.64) controlPoint2: NSMakePoint(65.16, 19.91)];
    [bezier772Path curveToPoint: NSMakePoint(73.8, 25.52) controlPoint1: NSMakePoint(69.66, 24.41) controlPoint2: NSMakePoint(71.7, 25.3)];
    [bezier772Path curveToPoint: NSMakePoint(74.47, 27.6) controlPoint1: NSMakePoint(74.03, 26.22) controlPoint2: NSMakePoint(74.28, 26.89)];
    [bezier772Path curveToPoint: NSMakePoint(75, 30.08) controlPoint1: NSMakePoint(74.7, 28.43) controlPoint2: NSMakePoint(74.84, 29.25)];
    [bezier772Path curveToPoint: NSMakePoint(70.93, 37.89) controlPoint1: NSMakePoint(72.55, 31.81) controlPoint2: NSMakePoint(70.93, 34.65)];

    [bezier772Path closePath];

    //CGFloat internalScale = 0.75;
    //NSAffineTransform *scaleXform = [NSAffineTransform transform];
    //[scaleXform scaleBy:internalScale];
    //[bezier772Path transformUsingAffineTransform:scaleXform];

    
    //Important:  the trick to applying translation prior to rotating a bezier path
    //is to perform the translation on the curve values prior to closing the path!!!
    NSAffineTransform *xform = [NSAffineTransform transform];
    [xform translateXBy:0 yBy:0 ];
    [bezier772Path transformUsingAffineTransform:xform];
    
    
    

    //NSAffineTransform *xform2 = [NSAffineTransform transform];
    //[xform2 translateXBy:75./2. yBy:75./2.];
    //[bezier772Path transformUsingAffineTransform:xform2];
    
    //[xform translateXBy:-1*scale  yBy:-1*scale];
     

    
    [NSColor.whiteColor setFill];
    bezier772Path.lineWidth = 7.;
    [bezier772Path fill];

    CGFloat labelHeight = imageHeight/ 1.5 / scale;///1.65;
    NSFont * font =  [NSFont fontWithName:@"Arial" size:labelHeight];
    NSDictionary *attributes = @{NSFontAttributeName : font, NSForegroundColorAttributeName : [[NSColor blackColor] colorWithAlphaComponent:0.9]};
    NSString * scribbleString = @"⌘";
    NSAttributedString *attributedString = [[NSAttributedString alloc] initWithString:scribbleString attributes:attributes];
    NSSize attrSize = [attributedString size];
    
    //attributedString = [[NSAttributedString alloc] initWithString:tickLabels[tickIndex] attributes:attributes];
    //attrSize = [attributedString size];
    
    [attributedString drawAtPoint:CGPointMake(center.x - attrSize.width/2.0,center.y - attrSize.height/2.)];//center.x - attrSize.width/2.0, center.y - attrSize.height/2.)];

    [image unlockFocus];

    // Now draw in a view by compositing
    //[image compositeToPoint:NSZeroPoint operation:NSCompositeCopy];
    [image drawInRect:CGRectMake(0,0,imageWidth, imageHeight)];
    
    return image;
     */
}

//this is the Mac OS entrypoint
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification {

    // Insert code here to initialize your application
    NSLog(@"PbAudioAppDelegate::applicationDidFinishLaunching");

    //Generate App Icon Images using Core Graphics
    //[CocoaImage createAppIconImages];
    
    //Transform Process to Agent Status Bar Only Application
    ProcessSerialNumber psn = { 0, kCurrentProcess };
    //OSStatus rt = TransformProcessType(&psn, kProcessTransformToUIElementApplication);  //Disable Slaving Focus of App to Desktop Workspace:  Use Custom Popup Menu
    //OSStatus rt = TransformProcessType(&psn, kProcessTransformToForegroundApplication); //Enable  Slaving Focus of App to Desktop Workspace:  Use Traditional App Menu
    //assert(rt == noErr);
    
    //Create Status Bar Menu Item
    //[self createStatusBarMenu];
    
    //Customize Menu Title
    [self setMenuTitle:@"[Pb] Audio"];

    //[[NSApplication sharedApplication] stop];
    //[NSRunLoop mainRunLoop]
    
    /*
    NFInterface * nwInterface = [NFInterface sharedInstance];
    // call the start method, which initializes the Tealium library
    [nwInterface connectWithHost:@"127.0.0.1" port:4710];
    // trigger a new event tracking call from objective-c
    [nwInterface start];
    */
    
    
    //[self createWindow];
    //[self createRootViewController];

    if(!self.window) [self createWindow];
    //[self createModalWindow];
        
    //self.window = [[CRWindow alloc] init];
    //self.window.backgroundColor = [NSColor redColor];
    
    //self.windowController = [[CRWindowController alloc] initWithWindow:_window];
    [self createRootViewController];

    //[self.window makeKeyAndOrderFront:self];   // Show the window
    //[self.window orderOut:self];               // Show the window

    //[self updateThruConnectionWidgets];
}


- (void)applicationWillTerminate:(NSNotification *)aNotification {
    // Insert code here to tear down your application
    NSLog(@"ApplicationWillTerminate");
    
    
}


- (BOOL)applicationSupportsSecureRestorableState:(NSApplication *)app {
    return YES;
}

#else


//This is the iOS entrypoint
- (BOOL)application:(UIApplication *)application didFinishLaunchingWithOptions:(NSDictionary *)launchOptions {
    // Override point for customization after application launch.

    NSLog(@"application:didFinishLaunchingWithOptions:");

    /*
     #if TARGET_OS_IOS || (TARGET_OS_IPHONE && !TARGET_OS_TV)
     // iOS-specific code
     //begin generating device orientation updates immediately at startup
     [[UIDevice currentDevice] beginGeneratingDeviceOrientationNotifications];
     #elif TARGET_OS_TV
     // tvOS-specific code
     #endif
     */
    
    /*
    [[UIDevice currentDevice] endGeneratingDeviceOrientationNotifications];

    if( [[UIDevice currentDevice].systemVersion floatValue] >= 13.0 )
    {
        NSLog(@"Here I am 0!");

    }
    else
    {
        NSLog(@"Here I am!");

        [self createWindow];
        [self createRootViewController];
    }
     */
    

    [self createWindow];
    [self createRootViewController];

    if(!self.window)
        NSLog(@"No Window Loaded!");
    /*
    NSBundle *frameworkBundle = [NSBundle bundleWithIdentifier:@"com.apple.GraphicsServices"];
    const char *frameworkPath = [[frameworkBundle executablePath] UTF8String];
    if (frameworkPath) {
        void *graphicsServices = dlopen(frameworkPath, RTLD_NOLOAD | RTLD_LAZY);
        if (graphicsServices) {
            BOOL (*GSFontAddFromFile)(const char *) = dlsym(graphicsServices, "GSFontAddFromFile");
            if (GSFontAddFromFile) {
                
                NSMutableDictionary *themeFonts = [[NSMutableDictionary alloc] init];
                NSString *fontFileName = nil; NSString *fontFilePath = nil;
                
                //add primary font
                //[self findNameOfFontFileWithType:PrimaryFont fontFileName:&fontFileName fontFilePath:&fontFilePath];
                //if (fontFilePath && fontFileName) {
                    //NSString *path = [NSString stringWithFormat:@"%@/%@", fontFilePath, fontFileName];
                    //newFontCount += GSFontAddFromFile([path UTF8String]);
                    //[themeFonts setObject:[fontFileName removeExtension] forKey:IFEPrimaryFontKey];
                    //[fontFilePath release];
                    //[fontFileName release];
               // }
            }
        }
    }
    */
    self.window.rootViewController = self.rootViewController;
    [self.window makeKeyAndVisible];

    //NSArray *paths = NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES);
    //NSLog(@"Documents DIR: %@", [paths objectAtIndex:0]);
    
    /*
    //notify our Core Render Application Event Loop that the Cocoa Touch application has finished launching
    struct kevent kev;
    EV_SET(&kev, crevent_init, EVFILT_USER, 0, NOTE_TRIGGER, 0, NULL);
    kevent(cr_appEventQueue, &kev, 1, NULL, 0, NULL);
    */
    
    
    return YES;
}



#pragma mark - UISceneSession lifecycle

- (UISceneConfiguration *)application:(UIApplication *)application configurationForConnectingSceneSession:(UISceneSession *)connectingSceneSession options:(UISceneConnectionOptions *)options  API_AVAILABLE(ios(13.0)){
    // Called when a new scene session is being created.
    // Use this method to select a configuration to create the new scene with.
    return [[UISceneConfiguration alloc] initWithName:@"Default Configuration" sessionRole:connectingSceneSession.role];
}


- (void)application:(UIApplication *)application didDiscardSceneSessions:(NSSet<UISceneSession *> *)sceneSessions  API_AVAILABLE(ios(13.0)){
    // Called when the user discards a scene session.
    // If any sessions were discarded while the application was not running, this will be called shortly after application:didFinishLaunchingWithOptions.
    // Use this method to release any resources that were specific to the discarded scenes, as they will not return.
}


#pragma mark - UISceneDelegate methods

- (void)scene:(UIScene *)scene willConnectToSession:(UISceneSession *)session options:(UISceneConnectionOptions *)connectionOptions  API_AVAILABLE(ios(13.0)){
    // Use this method to optionally configure and attach the UIWindow `window` to the provided UIWindowScene `scene`.
    // If using a storyboard, the `window` property will automatically be initialized and attached to the scene.
    // This delegate does not imply the connecting scene or session are new (see `application:configurationForConnectingSceneSession` instead).

    UIWindowScene * windowScene = [[UIWindowScene alloc] initWithSession:session connectionOptions:connectionOptions];
    if( !windowScene ) return;
    
    //self.window = [[UIWindow alloc] initWithFrame:windowScene.coordinateSpace.bounds];
    self.window.windowScene = windowScene;
    self.window = [[UIWindow alloc] initWithWindowScene:windowScene];
    
    self.window.rootViewController = [[UIViewController alloc] init];
    self.window.rootViewController.view.backgroundColor = [UIColor purpleColor];
    [self.window makeKeyAndVisible];
}

- (void)sceneDidDisconnect:(UIScene *)scene  API_AVAILABLE(ios(13.0)){
    // Called as the scene is being released by the system.
    // This occurs shortly after the scene enters the background, or when its session is discarded.
    // Release any resources associated with this scene that can be re-created the next time the scene connects.
    // The scene may re-connect later, as its session was not necessarily discarded (see `application:didDiscardSceneSessions` instead).
}


- (void)sceneDidBecomeActive:(UIScene *)scene  API_AVAILABLE(ios(13.0)){
    // Called when the scene has moved from an inactive state to an active state.
    // Use this method to restart any tasks that were paused (or not yet started) when the scene was inactive.
}


- (void)sceneWillResignActive:(UIScene *)scene  API_AVAILABLE(ios(13.0)){
    // Called when the scene will move from an active state to an inactive state.
    // This may occur due to temporary interruptions (ex. an incoming phone call).
}


- (void)sceneWillEnterForeground:(UIScene *)scene  API_AVAILABLE(ios(13.0)){
    // Called as the scene transitions from the background to the foreground.
    // Use this method to undo the changes made on entering the background.
}


- (void)sceneDidEnterBackground:(UIScene *)scene  API_AVAILABLE(ios(13.0)){
    // Called as the scene transitions from the foreground to the background.
    // Use this method to save data, release shared resources, and store enough scene-specific state information
    // to restore the scene back to its current state.
}


#endif



@end
