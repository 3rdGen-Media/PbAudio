//
//  CMDevice.h
//  MCUCommander
//
//  Created by Joe Moulton on 11/25/23.
//

#import "CMDOM.h"

NS_ASSUME_NONNULL_BEGIN

//Create a short MACRO reference to the global instance constructor
#define CMDevice [CMHardwareDevice sharedInstance]

@interface CMHardwareDevice : CMDOM

+(CMHardwareDevice*)sharedInstance;

@property (nonatomic, retain) NSString * DeviceID;

+(void)loadHardwareDevice;

//Create & Delete are called as class methods
+(CMHardwareDevice*)createHardwareDevice:(NSString*)DeviceID Description:(CMDeviceDescription*)description;
+(CMHardwareDevice*)createHardwareDevice:(NSString*)DeviceID Type:(CMDeviceType)deviceType;

+(void)deleteHardwareDevice:(CMHardwareDevice*)device;

//but a data model instance can save its internal params to core midi when changed
-(void)saveHardwareDevice;
-(void)replaceHardwareDevice:(NSString*)DeviceID atIndex:(NSUInteger)domIndex;


-(id)initWithDeviceDescription:(CMDeviceDescription*)connection;

-(CMDeviceDescription*)deviceDescription;
-(CMDisplay*)deviceDisplays;
-(CMControl*)deviceControls;
-(CMControl*)deviceSwitches;

-(void)setConnection:(CMConnection*)connection;
-(void)sendMCUDeviceInitializationMessages;

-(void)purgeMessageQueue;
-(void)processMessagePacket:(const MIDIEventPacket*)packet;//(MCUMessage)message;
-(void)sendMessagesOnConnection:(CMConnection*)connection;

//Forward messages
-(void)sendEventList:(const MIDIEventList *)eventList onConnection:(CMConnection*)connection;

//Immediate Send Messages
-(void)sendSysExDisplayMessage:(int)displayIndex Line1:(NSString*)line1 Line2:(NSString*)line2;
-(void)sendSysExDisplayMessage:(int)displayIndex ChannelIndex:(int)channelIndex Line1:(char*)line1 Line2:(char*)line2;

-(void)sendMessage:(uint8_t*)txMessage size:(uint8_t)txMessageSize type:(MIDIMessageType)txMessageType;



@end

NS_ASSUME_NONNULL_END
