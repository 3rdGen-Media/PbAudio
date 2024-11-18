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

#import "AudioMidiSettingsView.h"
#import "SettingsModalViewController.h"

#if TARGET_OS_OSX
//#import "CRWindowController.h"
//#import "CRMenu.h"
//#import "CRAppInterface.h"
#import "PbAudioAppMenu.h"

//Modal

//#import "AudioMidiSettingsView.h"
#import "DeviceManagerView.h"
#import "ProxyManagerView.h"
#import "CMProxyModalView.h"
#import "ControlManagerView.h"
#import "DisplayManagerView.h"
//#import "SettingsModalViewController.h"

//AppIcon Generation
#import "CocoaImage+Logo.h"

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
#import "AudioUnitViewController.h"


//Button
#import "CustomButtonView.h"



#import "NSString+Ext.h"

static NSString * const NSToolbarForwardItemIdentifier  = @"ForwardToolbarItem";
static NSString * const NSToolbarBackwardItemIdentifier = @"BackwardToolbarItem";
static NSString * const NSToolbarAddItemIdentifier      = @"AddToolbarItem";


@interface PbAudioAppDelegate ()
{
    //CMThruView * _thruView;
    //CMThruScrollListView * _scrollListView;
}

#if TARGET_OS_OSX

//@property (nonatomic, retain) CustomButtonView* createThruButton;
@property (nonatomic, retain) NSTitlebarAccessoryViewController * accessoryViewController;

@property                     NSStatusItem                       * barItem ;
#endif


@property (nonatomic, retain) CocoaToolbar                * toolbar;
@property (nonatomic, retain) CocoaToolbar                * modalToolbar;

@property (nonatomic, retain) CocoaView                   * rootView;
@property (nonatomic, retain) AudioUnitViewController    * rootViewController;    //root VC

@property (nonatomic, retain) CocoaView                   * modalView;
@property (nonatomic, retain) SettingsModalViewController * modalViewController;

@property (nonatomic, retain) CocoaImage                  * statusOnImage;
@property (nonatomic, retain) CocoaImage                  * statusOffImage;

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
    CGSize popoverSize = CGSizeMake(550, 450); //ToonTrack Window Size
    CGRect viewFrame = CGRectMake(0,0,popoverSize.width, popoverSize.height);
    
    //Create the view/view controller pair that will manage the UI for a single abstract data model database entry as a modal window, popover or sheet
    self.modalView           = [[AudioMidiSettingsView       alloc] initWithFrame:viewFrame];
    self.modalViewController = [[SettingsModalViewController alloc] initWithView:self.modalView];
    self.modalView.translatesAutoresizingMaskIntoConstraints = NO;
    
#if TARGET_OS_OSX
    self.modalView.autoresizingMask = NSViewNotSizable;
    
    //Present the ModalView/ViewCOntroller pair as a modal window, popover, or sheet on top of the parent window/view controller pair
    [self.rootViewController presentViewControllerAsModalWindow:self.modalViewController];
    
    [self.modalView.window setStyleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable]; // | NSWindowStyleMaskMiniaturizable];// | NSWindowStyleMaskFullSizeContentView ];
    self.modalView.window.title = [NSString stringWithFormat:@"Audio/Midi Setup"];
    //self.modalView.titleLabel.stringValue = [NSString stringWithFormat:@"New Midi Route"];
    
    /*
    //Override window close button action
    NSButton *closeButton = [self.modalView.window standardWindowButton:NSWindowCloseButton];
    //[closeButton setHidden:YES];
    [closeButton setTarget:self]; // alternatively you can make it self.windowController
    [closeButton setAction:@selector(dismissModalWindow)];
    */
    
    CGPoint windowCenter = CGPointMake( self.window.frame.origin.x + self.window.frame.size.width/2., self.window.frame.origin.y + self.window.frame.size.height/2.);
    CGRect  windowFrame = self.modalViewController.view.window.frame;
    windowFrame.origin = CGPointMake(windowCenter.x - windowFrame.size.width/2., windowCenter.y - windowFrame.size.height/2);
    [self.modalView.window setFrame: windowFrame display:YES];
    
    [self.modalView.window makeKeyAndOrderFront:self];     // Show the window
#else
    
    CGRect sourceRect = CGRectMake(0,0,1,1);
    self.modalViewController.modalPresentationStyle = UIModalPresentationPopover;
    //self.modalViewController.popoverPresentationController.delegate = self;
    self.modalViewController.preferredContentSize = CGSizeMake(viewFrame.size.width, viewFrame.size.height);
    self.modalViewController.popoverPresentationController.sourceView = self.rootViewController.view;
    self.modalViewController.popoverPresentationController.sourceRect = sourceRect;
    
    self.modalView.autoresizingMask = UIViewAutoresizingNone;
    [self.rootViewController presentViewController:self.modalViewController animated:NO completion:nil];
    
#endif
}

#if TARGET_OS_OSX

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

#endif


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
        
    CGPoint origin = CGPointMake(NSScreen.mainScreen.frame.size.width/2. - NSScreen.mainScreen.frame.size.width/4., -NSScreen.mainScreen.frame.size.height/4.);
    [self.window setFrame:CGRectMake(origin.x, origin.y, self.window.frame.size.width, self.window.frame.size.height) display:YES];
#else
    self.window = [[UIWindow alloc] initWithFrame:[[UIScreen mainScreen] bounds]];
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
    self.rootViewController = [[AudioUnitViewController alloc] initWithView:self.rootView];
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
    self.rootViewController = [[AudioUnitViewController alloc] initWithWindow:self.window];
    
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
    //self.window.rootViewController = self.rootViewController;
    [self.window makeKeyAndVisible];

    [self openSettingsManagerModalWindow];
    
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


/*
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
*/

#endif



@end
