//
//  QMidiMenu.m
//  CRViewer-OSX
//
//  Created by Joe Moulton on 2/9/19.
//  Copyright © 2019 Abstract Embedded. All rights reserved.
//

#import "PbAudioPopupMenu.h"
#import "PbAudioAppInterface.h"

@implementation PbAudioPopupMenu

-(BOOL)validateMenuItem:(NSMenuItem*)menuItem
{
    if( !(self.autoenablesItems) )
    {
        return menuItem.enabled;
    }
    
    return YES;
}

+ (id)sharedInstance
{
    static PbAudioPopupMenu *menu = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        menu = [[self alloc] init];
    });
    return menu;
}

-(void)closeCurrentWindow:(id)sender
{
    NSLog(@"Close current window");

}

-(void)openSettingsManager:(id)sender
{
    NSLog(@"PbAudioPopupMenu openSettingsManager");
    [[PbAudioAppDelegate sharedInstance] openSettingsManagerModalWindow];
}

-(void)openDeviceManager:(id)sender
{
    NSLog(@"MCUCommanderMenu openDeviceManager");
    //[[MCUApplicationDelegate sharedInstance] openDeviceManagerModalWindow:nil];
}

-(void)openProxyManager:(id)sender
{
    NSLog(@"MCUCommanderMenu openProxyManager");
    //[[MCUApplicationDelegate sharedInstance] openProxyManagerModalWindow:nil];
}

-(void)openActiveDeviceWindow:(id)sender
{
    //[[MCUApplicationDelegate sharedInstance] openActiveDeviceWindow];
}

-(void)toggleFullscreen:(id)sender
{
    NSLog(@"MCUCommanderMenu toggleFullscreen");
        
    //notify our Core Render Application Event Loop that the Cocoa Touch application has finished launching
    /*
    struct kevent kev;
    uint64_t menuEventType = QMidiMenuEvent_ToggleFullscreen;
    EV_SET(&kev, CTProcessEvent_Menu, EVFILT_USER, 0, NOTE_TRIGGER, 0, (void*)menuEventType);
    kevent(CTProcessEventQueue.kq, &kev, 1, NULL, 0, NULL);
    */
}


-(void)quitApplication:(id)sender
{
    //Hold up NSApplication::terminate: from being called so we can notify CoreTransport to clean up and it can notify NSApplication when finished so that shutdown can proceed
    //struct kevent kev;
    //EV_SET(&kev, crevent_exit, EVFILT_USER, EV_ADD | EV_ENABLE | EV_ONESHOT, NOTE_FFCOPY|NOTE_TRIGGER|0x1, 0, NULL);
    //kevent(cr_appEventQueue, &kev, 1, NULL, 0, NULL);
    
    //struct kevent kev;
    //uint64_t menuEventType = crmenu_quit;
    //EV_SET(&kev, crevent_menu, EVFILT_USER, 0, NOTE_TRIGGER, 0, (void*)menuEventType);
    //kevent(cr_appEventQueue, &kev, 1, NULL, 0, NULL);
    
    //Send message to enable/disable a midi input connection
    CMUniversalMessage * message     = &pba_message_events[pba_message_event_index++]; pba_message_event_index = pba_message_event_index % MAX_MESSAGE_EVENTS;
    message->type                    = CMMessageTypeSystem;
    message->group                   = pba_shutdown;
    //message->system.status           = enabled ? CMStatusStart : CMStatusStop;
    //message->system.uniqueID         = CMClient.sources[index].uniqueID;
        
    struct kevent kev;
    EV_SET(&kev, message->type, EVFILT_USER, 0, NOTE_TRIGGER, 0, message);
    kevent(PBAudio.eventQueue.kq, &kev, 1, NULL, 0, NULL);
}


-(void)createMainSubmenu
{
    //0  Create at  least one  Menu Bar Item (Submenu)
    //NSMenuItem* appMenuItem = [NSMenuItem new];
    //[self addItem:appMenuItem];
    
    //1 Create Main Submenu
    //NSMenu* appMenu = [NSMenu new];
    //[appMenu setTitle:@"C4 Commander"];
    
    
    //1.a Add Items To Main Submenu
    /*
    NSString* toggleFullScreenTitle = @"Toggle Full Screen";
    NSMenuItem* toggleFullScreenMenuItem = [[NSMenuItem alloc] initWithTitle:toggleFullScreenTitle
                                                                      action:@selector(toggleFullScreen:)
                                                               keyEquivalent:@"f"];
    [toggleFullScreenTitle setTarget:self];
    [appMenu addItem:toggleFullScreenMenuItem];
    */
    

    NSMenuItem* AudioMidiSetupMenuItem = [[NSMenuItem alloc] initWithTitle:@"Audio/Midi Settings..."
                                                                      action:@selector(openSettingsManager:)
                                                                      keyEquivalent:@""];

    
    NSMenuItem* deviceManagerMenuItem = [[NSMenuItem alloc] initWithTitle:@"Control Surfaces"
                                                                      action:@selector(openDeviceManager:)
                                                                      keyEquivalent:@""];
    
    NSMenuItem* proxyManagerMenuItem = [[NSMenuItem alloc] initWithTitle:@"Proxy Connections"
                                                                      action:@selector(openProxyManager:)
                                                                      keyEquivalent:@""];
    
    [self addItem:AudioMidiSetupMenuItem];
    [AudioMidiSetupMenuItem setTarget:self];
    AudioMidiSetupMenuItem.enabled = YES;

    [self addItem:deviceManagerMenuItem];
    [deviceManagerMenuItem setTarget:self];
    deviceManagerMenuItem.enabled = YES;
    
    [self addItem:proxyManagerMenuItem];
    [proxyManagerMenuItem setTarget:self];
    proxyManagerMenuItem.enabled = YES;
    
    //NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(terminate:) keyEquivalent:@""];

    NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit" action:@selector(quitApplication:) keyEquivalent:@"q"];
    [quitMenuItem setKeyEquivalent:@"q"];
    [quitMenuItem setTarget:self];
    [quitMenuItem setEnabled:YES];

    [self addItem:quitMenuItem];
    
    //1.b Add the submenu to the menu bar
    //[appMenuItem setSubmenu:self];
    
    //[appMenuItem setTarget:self];
    //appMenuItem.enabled = YES;
    
    
}

-(void)createFileSubmenu
{
    //2  Create File SubMenu
    NSMenuItem* fileMenuItem = [NSMenuItem new];
    [self addItem:fileMenuItem];
    
    NSMenu* fileMenu = [NSMenu new];
    [fileMenu setTitle:@"File"];
    
    //Add Items To File Submenu
    NSMenuItem* closeWindowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Close Window"
                                                                 action:@selector(closeCurrentWindow:)
                                                          keyEquivalent:@"w"];
    [closeWindowMenuItem setKeyEquivalentModifierMask: NSEventModifierFlagShift | NSEventModifierFlagCommand];
    [closeWindowMenuItem setKeyEquivalent:@"w"];
    
    [fileMenu addItem:closeWindowMenuItem];
    
    //Add the submenu to the menu bar
    [fileMenuItem setSubmenu:fileMenu];
    
}

-(void)createViewSubmenu
{
    //2  Create File SubMenu
    NSMenuItem* subMenuItem = [NSMenuItem new];
    [subMenuItem setEnabled:YES];

    [self addItem:subMenuItem];
    
    NSMenu* submenu = [NSMenu new];
    //NSString * title = @"View";
    [submenu setTitle:@"View"];
    
    //Add Items To File Submenu
    NSMenuItem* toggleFullscreenItem = [[NSMenuItem alloc] initWithTitle:@"Toggle Fullscreen"
                                                                 action:@selector(toggleFullscreen:)
                                                          keyEquivalent:@"f"];
    [toggleFullscreenItem setKeyEquivalentModifierMask: NSEventModifierFlagControl | NSEventModifierFlagCommand];
    [toggleFullscreenItem setKeyEquivalent:@"f"];
    [toggleFullscreenItem setTarget:self];
    [toggleFullscreenItem setEnabled:YES];

    [submenu addItem:toggleFullscreenItem];
    
    //Add the submenu to the menu bar
    [subMenuItem setSubmenu:submenu];
    
    //[self itemChanged:subMenuItem];
    //[self itemChanged:toggleFullscreenItem];

}

-(void)createCustomSubmenu
{
    NSView * myView1 = [[NSView alloc] initWithFrame:CGRectMake(0,0,512,512)];
    myView1.layer.backgroundColor = [NSColor yellowColor].CGColor;
    
    NSMenuItem* menuBarItem = [[NSMenuItem alloc]
                               initWithTitle:@"Custom" action:NULL keyEquivalent:@""];
    // title localization is omitted for compactness
    NSMenu* newMenu = [[NSMenu alloc] initWithTitle:@"Custom"];
    [menuBarItem setSubmenu:newMenu];
    [self addItem:menuBarItem];
    //[[NSApp mainMenu] insertItem:menuBarItem atIndex:3];
    
    /*
     Assume that myView1 and myView2 are existing view objects;
     for example, you may have created them in a NIB file.
     */
    
    /*
    NSMenuItem* newItem;
    newItem = [[NSMenuItem alloc]
               initWithTitle:@"Custom Item 1"
               action:@selector(menuItem1Action:)
               keyEquivalent:@""];
    [newItem setView: myView1];
    [newItem setTarget:self];
    [newMenu addItem:newItem];
    
    newItem = [[NSMenuItem alloc]
               initWithTitle:@"Custom Item 2"
               action:@selector(menuItem2Action:)
               keyEquivalent:@""];
    
    [newItem setView: myView1];
    [newItem setTarget:self];
    [newMenu addItem:newItem];
     */
     
}


-(id)init{
    
    self = [super initWithTitle:@"C4 Commander"];
    if(self)
    {
        [self setAutoenablesItems:NO];
        
        //Create the Submenus
        [self createMainSubmenu];
        //[self createFileSubmenu];
        //[self createViewSubmenu];
        //[self createCustomSubmenu];
    }
    return self;
}

@end
