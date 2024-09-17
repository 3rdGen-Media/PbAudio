//
//  CMDevice.m
//  MCUCommander
//
//  Created by Joe Moulton on 11/25/23.
//

#import "CMHardwareDevice.h"

#define TX_MESSAGE_QUEUE_SIZE  64
#define SYSEX_MAX_BUFFER_SIZE  256

@interface CMHardwareDevice()
{
    //struct CMConnection _softThruConnection;
    struct CMDeviceDescription _deviceDescription;

    struct CMDisplay * _deviceDisplays;
    struct CMControl * _deviceControls;
    struct CMControl * _deviceSwitches;

    //Tx Message Byte Buffer (both SysEx and Notes)
    MIDIMessageType _txMessageType;
    uint8_t         _txMessageSize;
    uint8_t         _txMessage[TX_MESSAGE_QUEUE_SIZE];
    
    //Rx SysEx Byte Buffer
    uint8_t    _sysexMessageSize;
    uint8_t    _sysexMessage[SYSEX_MAX_BUFFER_SIZE];
    
    volatile bool _sysexInflight;
    
    CMConnection * _connection;
}

@end

@implementation CMHardwareDevice

+(NSArray*)loadHardwareDeviceKeysFromCache
{
    return (NSArray*)[[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:CM_HARDWARE_DEVICE_BASE_ID]];
}

+(void)saveHardwareDeviceKeysInCache
{
    NSLog(@"saveHardwareDeviceKeysInCache: %@", CMDevice.keys);
    //Save key dictionary in NSUserDefaults KeyStore
    [[NSUserDefaults standardUserDefaults] setObject:CMDevice.keys forKey:[NSString stringWithUTF8String:CM_HARDWARE_DEVICE_BASE_ID]];
    [[NSUserDefaults standardUserDefaults] synchronize];
}


+(void)deleteSoftThruConnectionKeysFromCache
{
    [[NSUserDefaults standardUserDefaults] setObject:nil forKey:[NSString stringWithUTF8String:CM_HARDWARE_DEVICE_BASE_ID]];
    [[NSUserDefaults standardUserDefaults] synchronize];

}

/*
-(id)initWithThruID:(NSString*)thruID inputs:(int*)inputIDs numInputs:(int) numInputs outputs:(int*)outputIDs numOutputs:(int)numOutputs
{
    
    
}
*/


+(void)deleteHardwareDevice:(CMHardwareDevice*)deviceDocument
{
    assert(1==0);
    if( [CMDevice.dictionary objectForKey:deviceDocument.primaryKey] )
    {
        //Look up or create extended ThruID for deletion of CoreMidi Thru Connection via CMidi API
        NSString * extendedDeviceID = [NSString stringWithFormat:@"%s.%@", CM_HARDWARE_DEVICE_BASE_ID, deviceDocument.primaryKey];
        NSLog(@"CMHardwareDevice::deleteHardwareDevice (%@)", extendedDeviceID);
        OSStatus cmError = CMDeleteHardwareDevice([extendedDeviceID cStringUsingEncoding:NSUTF8StringEncoding]);
        assert(cmError == 0);
        
        [CMDevice.dictionary removeObjectForKey:deviceDocument.primaryKey];
        [CMDevice.documents removeObject:deviceDocument];
        [CMDevice.keys removeObject:deviceDocument.DeviceID];
        
        NSLog(@"CMDevice.dictionary = %@", CMDevice.dictionary);
        NSLog(@"CMDevice.documents = %@", CMDevice.documents);
        NSLog(@"CMDevice.keys = %@", CMDevice.keys);
        
        [CMHardwareDevice saveHardwareDeviceKeysInCache];
    }
    else assert(1==0);
    
}

+(CMHardwareDevice*)createHardwareDevice:(NSString*)DeviceID Description:(CMDeviceDescription*)deviceDescription
{
    assert(deviceDescription);
    
    NSString * extendedDeviceID = [NSString stringWithFormat:@"%s.%@", CM_HARDWARE_DEVICE_BASE_ID, DeviceID];
    NSLog(@"CMHardwareDevice::createHardwareDevice (%@)", extendedDeviceID);
    
    //Create CoreMidi soft thru connection via CMidi
    //CMConnection* thruConnection = CMCreateSoftThruConnection([extendedThruID cStringUsingEncoding:NSUTF8StringEncoding], params);
    //assert(thruConnection);

    //Copy the extendedDeviceID into the deviceDescription name property
    strcpy(deviceDescription->name, [extendedDeviceID cStringUsingEncoding:NSUTF8StringEncoding]);//, strlen(thruID));

    //Create a CMHardwareDevice DOM object wrapping the ...
    CMHardwareDevice * hardwareDeviceDOM = [[CMHardwareDevice alloc] initWithDeviceDescription:deviceDescription];
        
    //overwrite the extended thru connection id with the short suffix
    hardwareDeviceDOM.DeviceID = DeviceID;
    
    if( ![CMDevice.dictionary objectForKey:hardwareDeviceDOM.DeviceID] )
    {
        [CMDevice.keys addObject:hardwareDeviceDOM.DeviceID];
        [CMDevice.documents addObject:hardwareDeviceDOM];
        [CMDevice.dictionary setObject:hardwareDeviceDOM forKey:hardwareDeviceDOM.DeviceID];
    }
    else assert(1==0);
    
    //Update Cache containing Thru Connection Keys
    [CMHardwareDevice saveHardwareDeviceKeysInCache];
    
    return hardwareDeviceDOM;
}

+(CMHardwareDevice*)createHardwareDevice:(NSString*)DeviceID Type:(CMDeviceType)deviceType
{
    //grab a device descrition from CMidi
    struct CMDeviceDescription * deviceDescription = &(CMClient.hardwareDevices[CMClient.numHardwareDevices]);

    NSString * extendedDeviceID = [NSString stringWithFormat:@"%s.%@", CM_HARDWARE_DEVICE_BASE_ID, DeviceID];
    NSLog(@"CMHardwareDevice::createHardwareDevice (%@)", extendedDeviceID);
    
    //Create CoreMidi soft thru connection via CMidi
    //CMConnection* thruConnection = CMCreateSoftThruConnection([extendedThruID cStringUsingEncoding:NSUTF8StringEncoding], params);
    //assert(thruConnection);
    
    //Copy the extendedDeviceID into the deviceDescription name property
    strcpy(deviceDescription->name, [extendedDeviceID cStringUsingEncoding:NSUTF8StringEncoding]);//, strlen(thruID));

    //set the deviceType on the deviceDescription
    deviceDescription->type = deviceType;
    
    //Create a CMHardwareDevice DOM object wrapping the ...
    CMHardwareDevice * hardwareDeviceDOM = [[CMHardwareDevice alloc] initWithDeviceDescription:deviceDescription];
    
    //overwrite the extended thru connection id with the short suffix
    hardwareDeviceDOM.DeviceID = DeviceID;
    
    if( ![CMDevice.dictionary objectForKey:hardwareDeviceDOM] )
    {
        [CMDevice.keys addObject:hardwareDeviceDOM.DeviceID];
        [CMDevice.documents addObject:hardwareDeviceDOM];
        [CMDevice.dictionary setObject:hardwareDeviceDOM forKey:hardwareDeviceDOM.DeviceID];
    }
    else assert(1==0);
    
    //Update Cache containing Thru Connection Keys
    [CMHardwareDevice saveHardwareDeviceKeysInCache];
    
    return hardwareDeviceDOM;
    
}

+(void)loadHardwareDevice
{
    int numHardwareDevices = 0;
    //CFDataRef data = nil;
    //CMDeviceDescription deviceDescription = {0};
 
    //CMUpdateInputDevices();
    //CMUpdateOutputDevices();
    
    //Uncomment when you fuck up:
    //[CMSoftThruConnection deleteThruConnectionKeysFromCache];
    
    //Load stored thru connection keys from cache
    NSArray * StoredKeys = [CMHardwareDevice loadHardwareDeviceKeysFromCache];
    NSLog(@"loadHardwareDevicesKeysFromCache: %@", StoredKeys);

    //NSArray * StoredKeys = @[@"TM-6"];
    
    //early out (no stored keys to load)
    if( !StoredKeys || ![StoredKeys isKindOfClass:[NSArray class]] || StoredKeys.count < 1) return;
    
    //const char * stored_keys[] = {"eDrumIn"};
    //int numStoredKeys = (int)StoredKeys.count;
    
    //TO DO: iterate and load hardware devices
    assert(1==0);
    /*
    //For each stored key find a hardware device
    for(int keyIndex = 0; keyIndex<numStoredKeys; keyIndex++)
    {
        NSString * nsStoredKey = [StoredKeys objectAtIndex:keyIndex];
        NSString * nsExtendedKey = [NSString stringWithFormat:@"%s.%@", CM_HARDWARE_DEVICE_BASE_ID, nsStoredKey];
        const char * storedKey = nsExtendedKey.UTF8String;
        
        CFStringRef cfKey = CFStringCreateWithCString(CFAllocatorGetDefault(), storedKey, kCFStringEncodingUTF8);
        OSStatus cmError = MIDIThruConnectionFind(cfKey, &data);
        if( cmError != noErr ) assert(1==0);
        
        unsigned long n = CFDataGetLength(data) / sizeof(MIDIThruConnectionRef);
        fprintf(stderr, "\nFound %lu existing thru connection(s)!\n", n);
        assert(n < CM_MAX_CONNECTIONS);
        
        MIDIThruConnectionRef * con = (MIDIThruConnectionRef*)CFDataGetBytePtr(data);
        for(int i=0;i<n;i++)
        {
            CMDriverID sourceDriver = 0;
            CMDriverID destDriver   = 0;
            //cmError = MIDIThruConnectionSetParams(thruConnection, cfDataParams);
            //cassert(cmError == noErr);
            
            memset(&thruParams, 0, sizeof(MIDIThruConnectionParams));
            MIDIThruConnectionParamsInitialize(&thruParams);
            CFDataRef cfDataParams = nil;//CFDataCreate(CFAllocatorGetDefault(), (const UInt8 *)(&thruParams),  sizeof(MIDIThruConnectionParams));
            cmError = MIDIThruConnectionGetParams(*con, &cfDataParams);
            if( cmError != noErr ) assert(1==0);

            MIDIThruConnectionParams * tmpThruParams = (MIDIThruConnectionParams *)CFDataGetBytePtr(cfDataParams);
            
            CMConnection * insertThruConnection = &(CMClient.thruConnections[numThruConnections]);
            insertThruConnection->connection = *con;
            memcpy(insertThruConnection->name, storedKey, strlen(storedKey));
            memcpy(&(insertThruConnection->params), tmpThruParams, MIDIThruConnectionParamsSize(tmpThruParams));

            //use the thru connection params to find core midi source and destination endpoint refs
            insertThruConnection->source.endpoint = tmpThruParams->sources[0].endpointRef;
            insertThruConnection->destination.endpoint = tmpThruParams->destinations[0].endpointRef;

            
            MIDIEntityRef srcEntity = 0;
            MIDIEntityRef dstEntity = 0;

            MIDIEndpointGetEntity(tmpThruParams->sources[0].endpointRef, &srcEntity);
            MIDIEndpointGetEntity(tmpThruParams->destinations[0].endpointRef, &dstEntity);

            
            CFStringRef srcStr = NULL;
            CFStringRef dstStr = NULL;

            char sSrc[32]; // driver name may truncate, but that's OK
            char sDst[32]; // driver name may truncate, but that's OK
            
            // begin with the endpoint's name
            //extern const CFStringRef kMIDIPropertyDriverOwner;
            MIDIObjectGetStringProperty(srcEntity, kMIDIPropertyDriverOwner, &srcStr);
            if (srcStr != NULL) {
                
                CFStringGetCString(srcStr, sSrc, 31, kCFStringEncodingUTF8);
                sSrc[31] = 0;  // make sure it is terminated just to be safe
                
                for ( int dID = 1; dID < cm_driver_list_size; dID++)
                {
                    if( strcmp(cm_driver_list[dID], sSrc) == 0 )
                    {
                        sourceDriver = dID;
                        
                    }
                }
            }
            
            //extern const CFStringRef kMIDIPropertyDriverOwner;
            MIDIObjectGetStringProperty(dstEntity, kMIDIPropertyDriverOwner, &dstStr);
            if (dstStr != NULL) {
                
                CFStringGetCString(dstStr, sDst, 31, kCFStringEncodingUTF8);
                sDst[31] = 0;  // make sure it is terminated just to be safe
                
                for ( int dID = 1; dID < cm_driver_list_size; dID++)
                {
                    if( strcmp(cm_driver_list[dID], sDst) == 0 )
                    {
                        destDriver = dID;
                        
                    }
                }
            }
            
            CMFullEndpointName(tmpThruParams->sources[0].endpointRef, insertThruConnection->source.name, &sourceDriver);
            CMFullEndpointName(tmpThruParams->destinations[0].endpointRef, insertThruConnection->destination.name, &destDriver);
             
            insertThruConnection->source.driverID = sourceDriver;
            insertThruConnection->destination.driverID = destDriver;
            
            //Create a CMSoftThruConnection DOM object wrapping the CMConnection
            CMSoftThruConnection * insertThruConnectionDOM = [[CMSoftThruConnection alloc] initWithCMConnection:insertThruConnection];
            insertThruConnectionDOM.ThruID = nsStoredKey;
            
            if( ![CMSoftThru.dictionary objectForKey:insertThruConnectionDOM] )
            {
                [CMSoftThru.keys addObject:insertThruConnectionDOM.ThruID];
                [CMSoftThru.documents addObject:insertThruConnectionDOM];
                [CMSoftThru.dictionary setObject:insertThruConnectionDOM forKey:insertThruConnectionDOM.ThruID];
            }
            else assert(1==0);
            
            numThruConnections++;
            //MIDIThruConnectionDispose(*con);
            con++;

            CFRelease(cfDataParams);
        }
        
        CFRelease(cfKey);
    }
    */
    
    CMClient.numThruConnections = numHardwareDevices;
}

-(void)dealloc
{
    NSLog(@"CMHardwareDevice::Dealloc");
    NSString * extendedDeviceID = [NSString stringWithFormat:@"%s.%@", CM_HARDWARE_DEVICE_BASE_ID, self.DeviceID];
    OSStatus cmError = CMDeleteHardwareDevice([extendedDeviceID cStringUsingEncoding:NSUTF8StringEncoding]);
    assert(cmError == noErr);
}

+(int)type
{
    return CM_DEVICE;
}



+ (CMHardwareDevice *)sharedInstance {
    static CMHardwareDevice *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[CMHardwareDevice alloc] init];
    });
    return sharedInstance;
}

+(NSArray*)requiredProperties
{
    return @[@"DeviceID"];
}


-(void)saveHardwareDevice
{
    assert(1==0);
    
    /*
    //Save existing thru connection params to CoreMIDI,
    CMSaveThruConnectionParams(self.connection);
    
    //Update DOM with changes
    [self deserializeConnectionParams:self.connection];
    
    //overwrite the extended thru connection id with the short suffix
    //thruConnectionDOM.ThruID = thruID;
    
    //CoreMIDI will only trigger update causing UI to update when connections are created/deleted
    //Not when they are modified
    [[PbAudioAppDelegate sharedInstance] updateThruConnectionWidgets];
    */
}

-(void)replaceHardwareDevice:(NSString*)DeviceID atIndex:(NSUInteger)domIndex
{
    assert(1==0);

    //make sure the new key doesn't already exist on another persistent thru connection
    if( [CMDevice.dictionary objectForKey:DeviceID]) assert(1==0);
    
    CMHardwareDevice * hardwareDeviceDOM = [CMDevice.dictionary objectForKey:self.primaryKey];
    if( hardwareDeviceDOM )
    {
        assert(hardwareDeviceDOM == [CMDevice.documents objectAtIndex:domIndex]);
        assert([hardwareDeviceDOM.primaryKey localizedCompare:[CMDevice.keys objectAtIndex:domIndex]] == NSOrderedSame);

        //Look up or create extended ThruID for deletion of CoreMidi Thru Connection via CMidi API
        NSString * oldDeviceID = self.primaryKey;
        NSString * oldExtendedDeviceID = [NSString stringWithFormat:@"%s.%@", CM_HARDWARE_DEVICE_BASE_ID, oldDeviceID];
        NSString * replacementDeviceID = [NSString stringWithFormat:@"%s.%@", CM_HARDWARE_DEVICE_BASE_ID, DeviceID];
        
        NSLog(@"CMHardwareDevice::replaceHardwareDevice (%@) with (%@)", oldExtendedDeviceID, replacementDeviceID);
        
        //find the CMidi thru connection matching this one
        assert(1==0);
        /*
        int thruIndex = 0;
        for(thruIndex = 0; thruIndex < CMClient.numThruConnections; thruIndex++)
        {
            if( CMClient.thruConnections[thruIndex].connection == self.connection->connection) break;
        }
        
        //ensure the thru connection we found has the same string id
        assert([oldExtendedThruID localizedCompare:[NSString stringWithUTF8String:CMClient.thruConnections[thruIndex].name]] == NSOrderedSame);
        
        //delete and recreate the CoreMIDI thru connection with a new string id
        CMReplaceThruConnectionAtIndex(self.connection, replacementThruID.UTF8String, thruIndex);
        */
        
        //set the new thru connection id on our DOM object
        [self setPrimaryKey:DeviceID];
        [self deserializeHardwareDeviceDescription:self.deviceDescription];

        //Update our master DOM lists
        [CMDevice.dictionary removeObjectForKey:oldDeviceID];
        [CMDevice.dictionary setObject:self forKey:DeviceID];
        [CMDevice.keys replaceObjectAtIndex:domIndex withObject:DeviceID];

        NSLog(@"CMSoftThru.dictionary = %@", CMDevice.dictionary);
        NSLog(@"CMSoftThru.documents = %@", CMDevice.documents);
        NSLog(@"CMSoftThru.keys = %@", CMDevice.keys);
        
        [CMHardwareDevice saveHardwareDeviceKeysInCache];
    }
    else assert(1==0);
}

-(id)init
{
    self = [super init];
    if(self)
    {
        _txMessageSize = _sysexMessageSize = _sysexInflight = 0;
        memset(&_txMessage, 0,   TX_MESSAGE_QUEUE_SIZE);
        memset(&_sysexMessage, 0, SYSEX_MAX_BUFFER_SIZE);
        
    }
    return self;
}

-(CMDeviceDescription*)deviceDescription
{
    return &_deviceDescription;
}

-(void)deserializeHardwareDeviceDescription:(CMDeviceDescription*)deviceDescription
{
    //deserialize values needed for hashing and UI display
    //self.ThruID = [NSString stringWithUTF8String:connection->name];
    
    /*
    self.Input = [NSString stringWithUTF8String:connection->source.name];
    self.Output = [NSString stringWithUTF8String:connection->destination.name];
    self.InputDriver = [NSString stringWithUTF8String:cm_driver_list_pp[connection->source.driverID]];
    self.OutputDriver = [NSString stringWithUTF8String:cm_driver_list_pp[connection->destination.driverID]];
    */
}



-(CMDisplay*)deviceDisplays
{
    return _deviceDisplays;
}

-(CMControl*)deviceControls
{
    return _deviceControls;
}

-(CMControl*)deviceSwitches
{
    return _deviceSwitches;
}

-(id)initWithDeviceDescription:(CMDeviceDescription*)deviceDescription
{
    self = [super init];
    if(self)
    {
        _txMessageSize = _sysexMessageSize = _sysexInflight = 0;
        memset(&_txMessage, 0,   TX_MESSAGE_QUEUE_SIZE);
        memset(&_sysexMessage, 0, SYSEX_MAX_BUFFER_SIZE);
        
        memcpy(&_deviceDescription, deviceDescription, sizeof(struct CMDeviceDescription));

        //first check the device type on the description -- sometimes that is all we need to know to map an entire device via CMidi

        //Allocate memory for the CMidi CMControls that make up the CMidi hardware device
        CMCreateHardwareDevice(&_deviceDescription, &_deviceDisplays, &_deviceControls, &_deviceSwitches );
        assert(_deviceDisplays);
        assert(_deviceControls);
        assert(_deviceSwitches);

        //self.ThruID = [NSString stringWithUTF8String:connection->name];
        [self setPrimaryKey:[NSString stringWithUTF8String:deviceDescription->name]];
        [self deserializeHardwareDeviceDescription:deviceDescription];
    }
    return self;
}



+(NSString*)primaryKey
{
    return @"DeviceID";
}


+(NSString*)getDocumentTitle:(NSDictionary*)document
{
    NSString * documentTitle = document[self.class.primaryKey];
    if( documentTitle && documentTitle.length > 0 )
    {
        NSArray * domTitleComponents = [documentTitle componentsSeparatedByString:@"+"];
        documentTitle = domTitleComponents.firstObject;
        documentTitle = [documentTitle stringByReplacingOccurrencesOfString:@"_" withString:@" "];
        return documentTitle;
    }
    return documentTitle;
}

+(NSString*)getDocumentDetailString:(NSDictionary*)document
{
    //Artist Movements
    NSString * artistMovements = document[@"HardwareDevices"];
    NSString * detailString = [artistMovements stringByReplacingOccurrencesOfString:@"," withString:@", "];
    return detailString;
}

/*
 -(NSString*)primaryKey
 {
 return [self valueForKey:self.class.primaryKey];
 }
 */

#pragma mark -- NSDocumentQuery Protocol
/*
+(NSString*)NSDocumentStoreURL
{
    return [NSString stringWithFormat:@"%s:%d", VRTDOM_STORE_URL, VRTDOM_STORE_PORT];
}
+(NSString*)NSDocumentStoreContainer
{
    return [NSString stringWithUTF8String:MC_DATABASE];
}
+(NSString*)NSDocumentStoreCollection
{
    return [NSString stringWithUTF8String:MC_ARTISTS];
}

+(NSString*)NSDocumentStoreOrderKey
{
    //return VRTUserProfile.primaryKey;//@"Email";//[NSString stringWithFormat:@"%s:%d", VRTDOM_STORE_URL, VRTDOM_STORE_PORT];
    return self.primaryKey;
}


-(NSString*)NSDocumentStoreURL
{
    return self.class.NSDocumentStoreURL;
}
-(NSString*)NSDocumentStoreContainer
{
    return self.class.NSDocumentStoreContainer;
}
-(NSString*)NSDocumentStoreCollection
{
    return self.class.NSDocumentStoreCollection;
}

-(NSString*)NSDocumentStoreOrderKey
{
    return self.class.NSDocumentStoreOrderKey;
}
*/

-(NSString *)DocumentUpdateNotification
{
    return @"com.3rdGen.CoreMIDI.SoftThru.DocumentUpdateNotification";
}

-(NSString *)InsertedDocumentsNotification
{
    return @"com.3rdGen.CoreMIDI.SoftThru.InsertedUserDocumentsNotification";
}

-(NSString*)DeletedDocumentsNotification
{
    return @"com.3rdGen.CoreMIDI.SoftThru.DeletedUserDocumentsNotification";
}

-(NSString*)ModifiedDocumentsNotification
{
    return @"com.3rdGen.CoreMIDI.SoftThru.ModifiedUserDocumentsNotification";
}


/*
-(VRTDOMError*)populatePropertiesFromDictionary:(NSDictionary*)fields
{
    NSLog(@"MastryArtist::populatePropertiesFromDictionary");
    VRTDOMError * domErr = [super populatePropertiesFromDictionary:fields];
    
    if( !domErr )
    {
        //Do Custom Class Processing
        
        //1)If CacheURL field has not been populated
        //  TO DO:  Check an input cache URL for reachability?
        if( !self.CacheURL || self.CacheURL.length < 0 || self.CacheURL )
    }
        
    unsigned int count = 0;
    //NSMutableDictionary *dictionary = [NSMutableDictionary new];
    objc_property_t *properties = class_copyPropertyList([self class], &count);
    
    //loop over each NSObject property
    for (int i = 0; i < count; i++) {
        
        NSString *propertyName = [NSString stringWithUTF8String:property_getName(properties[i])];
        
        id valueContainer = [fields objectForKey:propertyName];
        id value;
        
        //first
        if( [valueContainer isKindOfClass:[NSTextField class]])
        {
            NSTextField *tf = (NSTextField*)valueContainer;
            value = tf.stringValue;//[propertyName];
        }
        
        if( [value isKindOfClass:[NSString class]] )
            [self setValue:value forKey:propertyName];
        //id value = [self valueForKey:key];
    }
    
    for (int i = 0; i < count; i++) {
        NSString *propertyName = [NSString stringWithUTF8String:property_getName(properties[i])];
        id valueContainer = [self valueForKey:propertyName];
        NSLog(@"%@ = %@", propertyName, valueContainer);
    }
    free(properties);
}
*/


-(void)insertDocuments:(NSArray*)documents withClosure:(NSDocumentQueryClosure)closure andOptions:(NSDictionary* __nullable)options
{
    NSLog(@"CMSoftThruConnection::insertDocuments:withClosure:andOptions");
}


-(void)deleteDocuments:(NSArray*)documents withClosure:(NSDocumentQueryClosure)closure andOptions:(NSDictionary* __nullable)options
{
    NSLog(@"CMSoftThruConnection::deleteDocuments:withClosure:andOptions");
    
    NSArray * results = nil;
    NSError * error = nil;
    for(int i = 0; i < documents.count; i++)
    {
        CMHardwareDevice * hardwareDeviceDOM = [documents objectAtIndex:i];
        [CMHardwareDevice deleteHardwareDevice:hardwareDeviceDOM];
    }
    
    //Return query result to client
    closure(self, error, results);
}

+(NSString*)domTitle
{
    return @"Hardware Device";
}

-(void)setConnection:(CMConnection*)connection;
{
    _connection = connection;
    
    [self sendMCUDeviceInitializationMessages];
}

-(void)sendMCUDeviceInitializationMessages
{
    [self sendDeviceQuery];
    //[self clearSysExDisplay];

}

/* * *
*
*   Received:
*    <Hdr> 00 F7 (Device Query)
*    <Hdr> 02 ss ss ss ss ss ss ss rr rr rr rr F7 (Host Connection Reply)
*    <Hdr> 0F 7F F7 (Go Offline)
*  
*   Transmitted:
*    <Hdr> 01 ss ss ss ss ss ss ss ll ll ll ll F7 (Host Connection Query)
*    <Hdr> 03 ss ss ss ss ss ss ss F7 (Host Connection Confirmation)
*    <Hdr> 04 ss ss ss ss ss ss ss F7 (Host Connection Error)
*
*    ss = Serial number (7 bytes ASCII text, non null-terminated)
*    ll = Challenge code (4 bytes)
*    rr = Response code (4 bytes)
*
* * */

typedef CF_ENUM(uint8_t, MCUGlobalControlMessageType)
{
    MCUGlobalMessage_DeviceQuery                 = 0x00,   //To HW
    MCUGlobalMessage_HostConnectionQuery         = 0x01,   //From HW
    MCUGlobalMessage_HostConnectionReply         = 0x02,   //To HW
    MCUGlobalMessage_ConnectionConfirmation      = 0x03,   //From HW
    MCUGlobalMessage_ConnectionError             = 0x04,   //From HW
    MCUGlobalMessage_TimeCode                    = 0x10,   //To HW
    MCUGlobalMessage_7Segment                    = 0x11,   //To HW
    MCUGlobalMessage_LCD                         = 0x12,   //To HW
    MCUGlobalMessage_LCD0                        = 0x30,   //To HW
    MCUGlobalMessage_LCD1                        = 0x31,   //To HW
    MCUGlobalMessage_LCD2                        = 0x32,   //To HW
    MCUGlobalMessage_LCD3                        = 0x33,   //To HW
};

//static const uint16_t MCUGlobalMessage_GoOffline = 0x0F7F; //To HW

typedef CF_ENUM(uint8_t, MCUDeviceID)
{
    MCUDeviceID_PRO                 = 0x10,
    MCUDeviceID_XT                  = 0x11,
    MCUDeviceID_C4                  = 0x17,
};

-(void)purgeMessageQueue
{
    _txMessageSize = 0;
}

-(void)sendEventList:(const MIDIEventList *)eventList onConnection:(CMConnection*)connection
{
    OSStatus cmError = MIDISendEventList(CMClient.outPort, connection->destination.endpoint, eventList);
    if( cmError != noErr )
    {
        fprintf(stderr, "\nMIDISendEventList failed with error: %d!", cmError);
        assert(1==0);
        //return cmError;
    }
    
}

-(void)sendMessagesOnConnection:(CMConnection*)connection
{
    //MIDIEventList Packets need to be fed bytes in the same endian order as received
    //While MIDIPacketList bytes need to have their order swapped
    OSStatus cmError = 0;
    MIDIEventList eventList = {};
    MIDIEventPacket *packet = MIDIEventListInit(&eventList, kMIDIProtocol_1_0);

    assert(_connection == connection);

    if( _txMessageSize < 1 ) return;
    
    
    if( _txMessageType == kMIDIMessageTypeSysEx )
    {
        NSMutableString * hexString = [NSMutableString string];
        for(int i = 0; i < _txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", _txMessage[i]]];
        NSLog(@"Send SySex Message:  %@", hexString);

        uint8_t bytesWritten = 0;

        //UMP SysEx 1-UP messages sit in 2 words, so the value of kMIDI1UPMaxSysexSize is 6
        //and we have to split our message across these blocks in UMP format
        
        uint8_t sysexMessageSize = _txMessageSize < kMIDI1UPMaxSysexSize ? _txMessageSize : kMIDI1UPMaxSysexSize;
        MIDIMessage_64 sysexStartMessage = MIDI1UPSysExArray(1, kMIDISysExStatusStart, _txMessage, _txMessage + sysexMessageSize);
        packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 2, (UInt32 *)&sysexStartMessage);
        bytesWritten += sysexMessageSize;

        while( bytesWritten < _txMessageSize - kMIDI1UPMaxSysexSize)
        {
            MIDIMessage_64 sysexContinueMessage  = MIDI1UPSysExArray(1, kMIDISysExStatusContinue, _txMessage + bytesWritten, _txMessage + bytesWritten + kMIDI1UPMaxSysexSize);
            packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 2, (UInt32 *)&sysexContinueMessage);
            bytesWritten += kMIDI1UPMaxSysexSize;
        }
        
        if( bytesWritten < _txMessageSize )
        {
            assert(_txMessageSize - bytesWritten <= kMIDI1UPMaxSysexSize);
            MIDIMessage_64 sysexEndMessage = MIDI1UPSysExArray(1, kMIDISysExStatusEnd, _txMessage + bytesWritten, _txMessage + bytesWritten + (_txMessageSize - bytesWritten));
            packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 2, (UInt32 *)&sysexEndMessage);
        }
        
        cmError = MIDISendEventList(CMClient.outPort, connection->destination.endpoint, &eventList);
        if( cmError != noErr )
        {
            fprintf(stderr, "\nMIDISendEventList failed with error: %d!", cmError);
            assert(1==0);
            //return cmError;
        }

    }
    else if( _txMessageType == kMIDIMessageTypeChannelVoice1)
    {
        assert( _txMessageSize % 4 == 0);
        
#if 1
        for( int messageIndex = 0; messageIndex< _txMessageSize / 4; messageIndex++ )
        {
            NSMutableString * hexString = [NSMutableString string];
            for(int i = 0; i < 4; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", _txMessage[messageIndex*4]]];
            NSLog(@"Send MIDI 1.0 Message [%d/%d]:  %@", messageIndex, _txMessageSize / 4, hexString);
            
            
            //int nPackets = 3;
            for(int i = 0; i<1; i++) packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 1, (UInt32 *)(_txMessage + messageIndex * 4));
            cmError = MIDISendEventList(CMClient.outPort, connection->destination.endpoint, &eventList);
            
            if( cmError != noErr )
            {
                fprintf(stderr, "\nMIDISendEventList failed with error: %d!", cmError);
                assert(1==0);
                //return cmError;
            }
        }
#else
        MIDITimeStamp timestamp = 0;
        const ByteCount MESSAGELENGTH = 6;
        Byte buffer[1024];             // storage space for MIDI Packets
        MIDIPacketList *packetlist = (MIDIPacketList*)buffer;
        MIDIPacket *currentpacket  = MIDIPacketListInit(packetlist);
        //Byte msgs[MESSAGELENGTH] = {0xB0, 0x32, (Byte)posX, 0xB0, 0x33, (Byte)posY};
        currentpacket = MIDIPacketListAdd(packetlist, sizeof(buffer), currentpacket, timestamp, _txMessageSize, _txMessage);

        cmError = MIDISend(CMClient.outPort, connection->destination.endpoint, packetlist);
        
        if( cmError != noErr )
        {
            fprintf(stderr, "\nMIDISendEventList failed with error: %d!", cmError);
            assert(1==0);
            //return cmError;
        }
        
#endif
    }

    _txMessageSize = 0;
    _txMessageType = kMIDIMessageTypeUnknownF;
}



-(OSStatus)sendMIDI1UPSysExMessage:(MIDIMessage_64)message port:(MIDIPortRef)port destination:(MIDIEndpointRef)destination 
{
    MIDIEventList eventList = {};
    MIDIEventPacket *packet = MIDIEventListInit(&eventList, kMIDIProtocol_1_0);
    packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 2, (UInt32 *)&message);
    return MIDISendEventList(port, destination, &eventList);
}

//public let kMIDI1UPMaxSysexSize: UInt8
//public func MIDI1UPSysEx(_ group: UInt8, _ status: UInt8, _ bytesUsed: UInt8, _ byte1: UInt8, _ byte2: UInt8, _ byte3: UInt8, _ byte4: UInt8, _ byte5: UInt8, _ byte6: UInt8) -> MIDIMessage_64
//public func MIDI1UPSysExArray(_ group: UInt8, _ status: UInt8, _ begin: UnsafePointer<UInt8>!, _ end: UnsafePointer<UInt8>!) -> MIDIMessage_64


-(void)sendMessage:(uint8_t*)txMessage size:(uint8_t)txMessageSize type:(MIDIMessageType)txMessageType
{
    assert(_connection);
    //MIDIEventList Packets need to be fed bytes in the same endian order as received
    //While MIDIPacketList bytes need to have their order swapped
    OSStatus cmError = 0;
    MIDIEventList eventList = {};
    MIDIEventPacket *packet = MIDIEventListInit(&eventList, kMIDIProtocol_1_0);

    if( txMessageSize < 1 ) return;
    
    if( txMessageType == kMIDIMessageTypeSysEx )
    {
        NSMutableString * hexString = [NSMutableString string];
        for(int i = 0; i < txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", txMessage[i]]];
        NSLog(@"Send SySex Message:  %@", hexString);
        
        uint8_t bytesWritten = 0;

        //UMP SysEx 1-UP messages sit in 2 words, so the value of kMIDI1UPMaxSysexSize is 6
        //and we have to split our message across these blocks in UMP format
        
        uint8_t sysexMessageSize = txMessageSize < kMIDI1UPMaxSysexSize ? txMessageSize : kMIDI1UPMaxSysexSize;
        MIDIMessage_64 sysexStartMessage = MIDI1UPSysExArray(1, kMIDISysExStatusStart, txMessage, txMessage + sysexMessageSize);
        packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 2, (UInt32 *)&sysexStartMessage);
        bytesWritten += sysexMessageSize;

        while( bytesWritten < txMessageSize - kMIDI1UPMaxSysexSize)
        {
            MIDIMessage_64 sysexContinueMessage  = MIDI1UPSysExArray(1, kMIDISysExStatusContinue, txMessage + bytesWritten, txMessage + bytesWritten + kMIDI1UPMaxSysexSize);
            packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 2, (UInt32 *)&sysexContinueMessage);
            bytesWritten += kMIDI1UPMaxSysexSize;
        }
        
        if( bytesWritten < txMessageSize )
        {
            assert(txMessageSize - bytesWritten <= kMIDI1UPMaxSysexSize);
            MIDIMessage_64 sysexEndMessage = MIDI1UPSysExArray(1, kMIDISysExStatusEnd, txMessage + bytesWritten, txMessage + bytesWritten + (txMessageSize - bytesWritten));
            packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 2, (UInt32 *)&sysexEndMessage);
        }

        //int nPackets = 3;
        //for(int i = 0; i<nPackets; i++) packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 2, (UInt32 *)_txMessage);
        cmError = MIDISendEventList(CMClient.outPort, _connection->destination.endpoint, &eventList);

    }
    else if( txMessageType == kMIDIMessageTypeChannelVoice1)
    {
        assert( _txMessageSize % 4 == 0);
#if 1
        for( int messageIndex = 0; messageIndex< txMessageSize / 4; messageIndex++ )
        {
            NSMutableString * hexString = [NSMutableString string];
            for(int i = 0; i < 4; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", txMessage[messageIndex*4]]];
            NSLog(@"Send Direct MIDI 1.0 Message [%d/%d]:  %@", messageIndex, txMessageSize / 4, hexString);
            
            
            //int nPackets = 3;
            for(int i = 0; i<1; i++) packet = MIDIEventListAdd(&eventList, sizeof(MIDIEventList), packet, 0, 1, (UInt32 *)(txMessage + messageIndex * 4));
            cmError = MIDISendEventList(CMClient.outPort, _connection->destination.endpoint, &eventList);
            
            if( cmError != noErr )
            {
                fprintf(stderr, "\nMIDISendEventList failed with error: %d!", cmError);
                assert(1==0);
                //return cmError;
            }
        }
#else
        MIDITimeStamp timestamp = 0;
        const ByteCount MESSAGELENGTH = 6;
        Byte buffer[1024];             // storage space for MIDI Packets
        MIDIPacketList *packetlist = (MIDIPacketList*)buffer;
        MIDIPacket *currentpacket  = MIDIPacketListInit(packetlist);
        //Byte msgs[MESSAGELENGTH] = {0xB0, 0x32, (Byte)posX, 0xB0, 0x33, (Byte)posY};
        currentpacket = MIDIPacketListAdd(packetlist, sizeof(buffer), currentpacket, timestamp, _txMessageSize, _txMessage);

        cmError = MIDISend(CMClient.outPort, connection->destination.endpoint, packetlist);
        
        if( cmError != noErr )
        {
            fprintf(stderr, "\nMIDISendEventList failed with error: %d!", cmError);
            assert(1==0);
            //return cmError;
        }
        
#endif
    }

    //_txMessageSize = 0;
    //_txMessageType = kMIDIMessageTypeUnknownF;
}

-(void)sendDeviceQuery
{
    
    uint8_t txMessage[256];
    uint8_t txMessageSize;
    MIDIMessageType txMessageType = kMIDIMessageTypeSysEx;
    
    MCUDeviceID deviceID = MCUDeviceID_PRO;
    if(_deviceDescription.type == CMDeviceType_MCU_Pro) deviceID = MCUDeviceID_PRO;
    else if(_deviceDescription.type == CMDeviceType_MCU_Extender) deviceID = MCUDeviceID_XT;
    else if(_deviceDescription.type == CMDeviceType_MCU_Commander) deviceID = MCUDeviceID_C4;
    
    //F0 00 00 66 10 00 F7
    txMessage[0] = 0x00;
    txMessage[1] = 0x00;
    txMessage[2] = 0x66;
    txMessage[3] = deviceID;
    txMessage[4] = MCUGlobalMessage_DeviceQuery;
    txMessageSize = 5;
    
    NSMutableString * hexString = [NSMutableString string];
    for(int i = 0; i < txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", txMessage[i]]];
    NSLog(@"sendDeviceQuery:  %@", hexString);
    
    [self sendMessage:txMessage size:txMessageSize type:txMessageType];

}

-(void)sendSysExDisplayMessage:(int)displayIndex Line1:(NSString*)line1 Line2:(NSString*)line2
{
    uint8_t txMessage[256];
    uint8_t txMessageSize;
    MIDIMessageType txMessageType = kMIDIMessageTypeSysEx;
    
    MCUDeviceID deviceID = MCUDeviceID_PRO;
    if(_deviceDescription.type == CMDeviceType_MCU_Pro) deviceID = MCUDeviceID_PRO;
    else if(_deviceDescription.type == CMDeviceType_MCU_Extender) deviceID = MCUDeviceID_XT;
    else if(_deviceDescription.type == CMDeviceType_MCU_Commander) deviceID = MCUDeviceID_C4;

    //F0 00 00 66 10 12 00 48 65 6C 6C 6F F7
    txMessage[0] = 0x00;
    txMessage[1] = 0x00;
    txMessage[2] = 0x66;
    txMessage[3] = deviceID;
    txMessage[4] = (deviceID == MCUDeviceID_C4) ? MCUGlobalMessage_LCD0 + displayIndex : MCUGlobalMessage_LCD;
    txMessageSize = 6 + 56;

    if( line1 )
    {
        //clear the whole message
        memset(txMessage + 6, ' ', 56);
        
        txMessage[5] = 0;// 56/2 - line1.length/2;
        memcpy(txMessage + 6 + 56/2 - line1.length/2, line1.UTF8String, line1.length);
        //txMessageSize = 6 + line1.length;
        
        NSMutableString * hexString = [NSMutableString string];
        for(int i = 0; i < txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", txMessage[i]]];
        NSLog(@"sendSysExDisplayMessageLine1:  %@", hexString);
        
        [self sendMessage:txMessage size:txMessageSize type:txMessageType];
    }
    
    if( line2 )
    {
        //clear the whole message
        memset(txMessage + 6, ' ', 56);

        txMessage[5] = 56;//56 + 56/2 - line2.length/2;
        memcpy(txMessage + 6 + 56/2 - line2.length/2, line2.UTF8String, line2.length);
        //txMessageSize = 6 + line2.length;
        
        NSMutableString * hexString = [NSMutableString string];
        for(int i = 0; i < txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", txMessage[i]]];
        NSLog(@"sendSysExDisplayMessageLine2:  %@", hexString);
        
        [self sendMessage:txMessage size:txMessageSize type:txMessageType];
    }
}


-(void)sendSysExDisplayMessage:(int)displayIndex ChannelIndex:(int)channelIndex Line1:(char*)line1 Line2:(char*)line2
{
    uint8_t txMessage[256];
    uint8_t txMessageSize;
    MIDIMessageType txMessageType = kMIDIMessageTypeSysEx;
    
    MCUDeviceID deviceID = MCUDeviceID_PRO;
    if(_deviceDescription.type == CMDeviceType_MCU_Pro) deviceID = MCUDeviceID_PRO;
    else if(_deviceDescription.type == CMDeviceType_MCU_Extender) deviceID = MCUDeviceID_XT;
    else if(_deviceDescription.type == CMDeviceType_MCU_Commander) deviceID = MCUDeviceID_C4;

    //F0 00 00 66 10 12 00 48 65 6C 6C 6F F7
    txMessage[0] = 0x00;
    txMessage[1] = 0x00;
    txMessage[2] = 0x66;
    txMessage[3] = deviceID;
    txMessage[4] = (deviceID == MCUDeviceID_C4) ? MCUGlobalMessage_LCD0 + displayIndex : MCUGlobalMessage_LCD;

    if( line1 )
    {
        //clear the channel message
        size_t strLength = strlen(line1) > 6 ? 6 : strlen(line1);
        memset(txMessage + 6, ' ', 7);
        txMessage[5] = channelIndex * 7;
        memcpy(txMessage + 6, line1, strLength);
        txMessageSize = 6 + 7;

        NSMutableString * hexString = [NSMutableString string];
        for(int i = 0; i < txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", txMessage[i]]];
        NSLog(@"sendSysExDisplayMessageLine1:  %@", hexString);
        
        [self sendMessage:txMessage size:txMessageSize type:txMessageType];
    }
    
    if( line2 )
    {
        size_t strLength = strlen(line2) > 6 ? 6 : strlen(line2);
        memset(txMessage + 6, ' ', 7);
        txMessage[5] = 56 + channelIndex * 7;
        memcpy(txMessage + 6, line2, strLength);
        txMessageSize = 6 + 7;
        
        NSMutableString * hexString = [NSMutableString string];
        for(int i = 0; i < txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", txMessage[i]]];
        NSLog(@"sendSysExDisplayMessageLine2:  %@", hexString);
        
        [self sendMessage:txMessage size:txMessageSize type:txMessageType];
    }
}

-(void)clearSysExDisplay
{
    uint8_t txMessage[256];
    uint8_t txMessageSize;
    MIDIMessageType txMessageType = kMIDIMessageTypeSysEx;
    
    MCUDeviceID deviceID = MCUDeviceID_PRO;
    if(_deviceDescription.type == CMDeviceType_MCU_Pro) deviceID = MCUDeviceID_PRO;
    else if(_deviceDescription.type == CMDeviceType_MCU_Extender) deviceID = MCUDeviceID_XT;
    else if(_deviceDescription.type == CMDeviceType_MCU_Commander) deviceID = MCUDeviceID_C4;

    //F0 00 00 66 10 12 00 48 65 6C 6C 6F F7
    txMessage[0] = 0x00;
    txMessage[1] = 0x00;
    txMessage[2] = 0x66;
    txMessage[3] = deviceID;
    txMessage[4] = MCUGlobalMessage_LCD;
    txMessage[5] = 0x00;

    for(int i=6; i< 6 + 56; i++) txMessage[i] = ' ';
    txMessageSize = 6 + 56;
        
    NSMutableString * hexString = [NSMutableString string];
    for(int i = 0; i < txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", txMessage[i]]];
    NSLog(@"clearSysExDisplayLine1:  %@", hexString);
    [self sendMessage:txMessage size:txMessageSize type:txMessageType];
    
    txMessage[5] = 0x38;
        
    hexString = [NSMutableString string];
    for(int i = 0; i < txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", txMessage[i]]];
    NSLog(@"clearSysExDisplayLine2:  %@", hexString);
    [self sendMessage:txMessage size:txMessageSize type:txMessageType];

}


-(void)processMessagePacket:(const MIDIEventPacket*)packet//(MCUMessage)message;
{
    for( int packetWord = 0; packetWord < 1/*packet->wordCount*/; packetWord++)
    {
        MCUMessage         message = {packet->words[packetWord]};

        //Unpack Packet
        MIDIMessageType    messageType      = message.word >> 28;
        //MIDICVStatus       status           = (message.word >> 20) & 0x0000000f;
        CMMessageCVStatus  statusHi         = (message.word >> 20) & 0x0000000f;
        CMMessageCVStatus  statusLo         = (message.word >> 16) & 0x0000000f;
        
        //Rotary XX:  0 s v v v v v v
        __block uint8_t XX = (message.word & 0x000000ff);
        
        __block CMControlIndex     controlIndex     = (message.word >> 8) & 0x000000ff;
        __block CMControlDirection controlDirection = (XX & 0x40) >> 6;  //encoder XX sign-bit
        __block CMControlStep      controlStep      = XX & 0x3F; //encoder (message.word & 0x000000f);
        
        __block CMControl *        control          = NULL;
        __block CMControlState     controlState     = _deviceControls[controlIndex].state;
        
        
        //Log Message for Debugging
        NSMutableString * hexString = [NSMutableString string];
        [hexString appendString:[NSString stringWithFormat:@"%02x", message.word]];
        
        /*
        NSLog(@"CMHardware::processMessage(%@)", hexString);
        NSLog(@"MessageType:  %@", [NSString stringForMIDIMessageType:messageType andStatus:(MIDICVStatus)statusHi]);
        NSLog(@"Control Index:  %@", [NSString stringWithFormat:@"%d", controlIndex]);
        NSLog(@"Control Direction:  %@", [NSString stringWithFormat:@"%d", controlDirection]);
        NSLog(@"Control Step:  %@", [NSString stringWithFormat:@"%d", controlStep]);
        NSLog(@"Control State:  %@", [NSString stringWithFormat:@"%d", controlState]);
        */
        
        //type should always be midi 1.0
        //status should always be Control Change
        
        if( messageType == kMIDIMessageTypeSysEx)
        {
            //assert(packet->wordCount == 2);
            
            for(int i=0; i< packet->wordCount; i+=2) //read sysex packets in pairs
            {
                statusHi         = (packet->words[i] >> 20) & 0x0000000f;
                statusLo         = (packet->words[i] >> 16) & 0x0000000f;
                
                //the value of statuHi nibble indicates if this is the start, continuation or end of a sysex message
                //but for now we really only need to know when the message ends
                //for( statusHi )
                
                //copy the sysex bytes from first word (pushing past the MCU HW SysEX Identifier Bytes)
                //the value of statusLo nibble indicates the number of bytes to read from the packet words
                //(we will always copy every byte into our buffer instead of conditionally branching)
                _sysexMessage[_sysexMessageSize] = (packet->words[i] >> 8) & 0x000000ff;;  //first  byte from last 16 bits of first word
                _sysexMessage[_sysexMessageSize+1] = packet->words[i] & 0x000000ff;        //second byte form last 16 bits of first word
                
                UInt32 beWord = cm_htobe32(packet->words[i+1]);
                memcpy(_sysexMessage + _sysexMessageSize + 2, &beWord, sizeof(UInt32));//four  bytes from the second word
                _sysexMessageSize += statusLo;

                //Now we've copied the data, we check if the last byte is the SysEx End message.
                if( statusHi == kMIDISysExStatusEnd || statusHi ==  kMIDISysExStatusComplete)
                {
                    //print the bytes we gathered in our
                    hexString = [NSMutableString string];
                    for(int i = 0; i < _sysexMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", _sysexMessage[i]]];
                    NSLog(@"SysEx End:  %@", hexString);
                    
                    //An MCU Sysex Message HDR looks like =  00 00 66 ii (where ii is either 10 = MCUPro or 11 = MCUExtender)
                    //Push past HDR to determine sysex message type and process sysex message:
                    if( _sysexMessage[4] == MCUGlobalMessage_HostConnectionQuery )
                    {
                        //static const int replySize = 5 + 7 + 4 + 1;
                        //uint8_t replyMessage[replySize];
                        //memset( _txMessage, 0, replySize);
                        memcpy(_txMessage, _sysexMessage, 12);
                        _txMessage[4] = MCUGlobalMessage_HostConnectionReply;
                        
                        //calcualte response bytes
                        uint8_t * l = &_sysexMessage[12];
                        uint8_t * r = &_txMessage[12];
                        
                        r[0] = 0x7F & (l[0] + (l[1] ^ 0xa) - l[3]);
                        r[1] = 0x7F & ((l[2]>>4) ^ (l[0] + l[3]));
                        r[2] = 0x7F & (l[3]-(l[2]<<2) ^ (l[0] | l[1]));
                        r[3] = 0x7F & (l[1]-l[2]+(0xF0^(l[3]<<4)));
                        
                        _txMessage[12+4] = 0xF7;
                        _txMessageSize = 12+4;//+1;
                        
                        hexString = [NSMutableString string];
                        for(int i = 0; i < _txMessageSize; i++) [hexString appendString:[NSString stringWithFormat:@"%02x", _txMessage[i]]];
                        NSLog(@"SysEx Reply:  %@", hexString);
                        
                        _txMessageType = kMIDIMessageTypeSysEx;
                    }
                    else if( _sysexMessage[4] == MCUGlobalMessage_ConnectionConfirmation )
                    {
                        NSLog(@"MCU Host Connection Success!!!!");
                    }
                    else if( _sysexMessage[4] == MCUGlobalMessage_ConnectionError )
                    {
                        NSLog(@"MCU Host Connection Failed!!!!");
                        assert(1==0);
                    }
                    
                    _sysexMessageSize = 0; //reset sysex message buffer
                }
                
                
                /*
                 // Check if the last byte is SysEx End.
                 continueSysEx = (packet->data[nBytes - 1] == 0xF7);
                 
                 If we've finished the message, or if we've filled the buffer then we have  a complete SysEx message to process. Here we're not doing anything with it, but in a proper application we'd pass it to whatever acts on the MIDI messages.
                 
                 if (!continueSysEx || sysExLength == SYSEX_LENGTH) {
                 // We would process the SysEx message here, as it is we're just ignoring it
                 
                 sysExLength = 0;
                 }
                 */
                
            }
            
        }
        else if( messageType == kMIDIMessageTypeChannelVoice1)
        {

            if( statusHi == CMMessageCVStatusControlChange ) //V-Pot B0,1i,XX
            {
                // MCU Pro & XT encoders are indexed as 1i where i is the index of the V-POT 1-7
                // So we will just interpret both nibbles and disregard the first nibble for these devices by modding
                if( _deviceDescription.type == CMDeviceType_MCU_Pro || _deviceDescription.type == CMDeviceType_MCU_Extender ) controlIndex %= 16;
                
                //Get Device Control corresponding to controlIndex
                control = &_deviceControls[controlIndex];
                //controlState = control->state;
                
                //Update V-POT Control state with the received message data (ie the state of the v-pot encoder knob)
                CMControlUpdate(control, controlDirection, controlStep);
                
                //Populate an output message based on V-POT state to control the V-POT LED Ring
                CMControlModeMessage(control, controlIndex, &(_txMessage[0]), &_txMessageSize);
                _txMessageType = kMIDIMessageTypeChannelVoice1;

                //queue UI to pick up the change on the control
                dispatch_async(dispatch_get_main_queue(),
                ^{
                    //[[PbAudioAppDelegate sharedInstance] setDevice:self ControlType:control->type atIndex:controlIndex];
                });
            }
            else if( statusHi == CMMessageCVStatusPitchBend )
            {
                // MCU Pro & XT encoders are indexed as 1i where i is the index of the V-POT 1-7
                // So we will just interpret both nibbles and disregard the first nibble for these devices by modding
                if( _deviceDescription.type == CMDeviceType_MCU_Pro ) controlIndex %= 16; // MCU Pro starts encoders at index 16? Why?// MCU Pro starts encoders at index 16? Why?

                //Pitch values are encoded in 12 bits 0xCXAB
                //The first most signifcant bit is the sign bit and the last nibble wraps at 7 instead of 15
                static int faderControlIndexOffset = 8;
                controlIndex = statusLo;
                control = &_deviceControls[faderControlIndexOffset + controlIndex];
                
                //uint8_t signBit = message.word >> 11;
                uint8_t faderValueMSB = (message.word & 0x0000007f);        //most signiicant 7 bits
                uint8_t faderValueLSB = (message.word & 0x0000f000) >> 12;  //least significant 3 bits
                
                unsigned short faderValue = (faderValueMSB << 3) | faderValueLSB;
                //faderValue |= ((message.word & 0x0000f000) << 8);
                //NSLog(@"%@", [NSString stringWithFormat:@"Fader[%d] Value: %hu", controlIndex, faderValue]);
                
                control->expression = faderValue;
                
                //queue the fader message for return to the MCU device on the output midi port
                //so it doesn't automate itself back to 0
                
                UInt32 beWord = packet->words[0];               //MIDIEventList format
                //UInt32 beWord = cm_htobe32(packet->words[1]); //MIDIPacketList format
                memcpy(_txMessage, &beWord, sizeof(beWord));
                _txMessageSize =  sizeof(beWord);
                _txMessageType = kMIDIMessageTypeChannelVoice1;

                dispatch_async(dispatch_get_main_queue(),
                ^{
                    //[[PbAudioAppDelegate sharedInstance] setDevice:self ControlType:control->type atIndex:controlIndex];
                });
                
            }
            else if( statusHi == CMMessageCVStatusNoteOn )
            {
                if( controlIndex < MCUNoteEvent_VSELECT8 + 1 )
                {
                    control = &_deviceSwitches[controlIndex];
                    
                    //Update the control click based on whether the switch was pressor unpressed
                    control->state = XX;
                    
                    /*
                     if( controlIndex == MCUNoteEvent_VSELECT1)
                     {
                     NSString * deviceName = [NSString stringWithFormat:@"%s", cm_device_names[_deviceDescription.type]];
                     [self sendSysExDisplayMessage:0 Line1:deviceName Line2:@"using MCU Commander by 3rdGen Media"];
                     }
                     */
                    
                    
                    UInt32 beWord = packet->words[0];               //MIDIEventList format
                    //UInt32 beWord = cm_htobe32(packet->words[0]);
                    memcpy(_txMessage, &beWord, sizeof(beWord));
                    _txMessageSize =  sizeof(beWord);
                    _txMessageType = kMIDIMessageTypeChannelVoice1;
                    
                    dispatch_async(dispatch_get_main_queue(),
                    ^{
                        //[[PbAudioAppDelegate sharedInstance] setDevice:self ControlType:control->type atIndex:controlIndex];
                    });
                }
            }
            else if( statusHi == CMMessageCVStatusNoteOff )
            {
                //assert(1==0);
            }
            
        }
    } //end packet word for loop
}

@end

