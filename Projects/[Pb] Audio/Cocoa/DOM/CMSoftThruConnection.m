//
//  CMSoftThruConnection.m
//  C4Commander
//
//  Created by Joe Moulton on 11/24/23.
//

#import "CMSoftThruConnection.h"

@interface CMSoftThruConnection()
{
    struct CMConnection _softThruConnection;
}

@end

@implementation CMSoftThruConnection

+(NSArray*)loadSoftThruConnectionKeysFromCache
{
    return (NSArray*)[[NSUserDefaults standardUserDefaults] objectForKey:[NSString stringWithUTF8String:CM_SOFT_THRU_BASE_ID]];
}

+(void)saveSoftThruConnectionKeysInCache
{
    NSLog(@"saveSoftThruConnectionKeysInCache: %@", CMSoftThru.keys);
    //Save key dictionary in NSUserDefaults KeyStore
    [[NSUserDefaults standardUserDefaults] setObject:CMSoftThru.keys forKey:[NSString stringWithUTF8String:CM_SOFT_THRU_BASE_ID]];
    [[NSUserDefaults standardUserDefaults] synchronize];
}


+(void)deleteSoftThruConnectionKeysFromCache
{
    [[NSUserDefaults standardUserDefaults] setObject:nil forKey:[NSString stringWithUTF8String:CM_SOFT_THRU_BASE_ID]];
    [[NSUserDefaults standardUserDefaults] synchronize];

}

/*
-(id)initWithThruID:(NSString*)thruID inputs:(int*)inputIDs numInputs:(int) numInputs outputs:(int*)outputIDs numOutputs:(int)numOutputs
{
    
    
}
*/

+(void)deleteSoftThruConnection:(CMSoftThruConnection*)thruConnectionDocument
{
    assert(1==0);
    if( [CMSoftThru.dictionary objectForKey:thruConnectionDocument.primaryKey] )
    {
        //Look up or create extended ThruID for deletion of CoreMidi Thru Connection via CMidi API
        NSString * extendedThruID = [NSString stringWithFormat:@"%s.%@", CM_SOFT_THRU_BASE_ID, thruConnectionDocument.primaryKey];
        NSLog(@"CMSoftThruConnection::deleteThruConnection (%@)", extendedThruID);
        OSStatus cmError = CMDeleteSoftThruConnection([extendedThruID cStringUsingEncoding:NSUTF8StringEncoding]);
        assert(cmError == 0);
        
        [CMSoftThru.dictionary removeObjectForKey:thruConnectionDocument.primaryKey];
        [CMSoftThru.documents removeObject:thruConnectionDocument];
        [CMSoftThru.keys removeObject:thruConnectionDocument.ThruID];
        
        NSLog(@"CMSoftThru.dictionary = %@", CMSoftThru.dictionary);
        NSLog(@"CMSoftThru.documents = %@", CMSoftThru.documents);
        NSLog(@"CMSoftThru.keys = %@", CMSoftThru.keys);
        
        [CMSoftThruConnection saveSoftThruConnectionKeysInCache];
    }
    else assert(1==0);
    
}

+(CMSoftThruConnection*)createSoftThruConnection:(NSString*)thruID Params:(MIDIThruConnectionParams*)params
{
    NSString * extendedThruID = [NSString stringWithFormat:@"%s.%@", CM_SOFT_THRU_BASE_ID, thruID];
    NSLog(@"CMSoftThruConnection::createSoftThruConnection (%@)", extendedThruID);
    
    //Create CoreMidi soft thru connection via CMidi
    CMConnection* thruConnection = CMCreateSoftThruConnection([extendedThruID cStringUsingEncoding:NSUTF8StringEncoding], params);
    assert(thruConnection);
    
    //Create a CMSoftThruConnection DOM object wrapping the CMConnection
    CMSoftThruConnection * thruConnectionDOM = [[CMSoftThruConnection alloc] initWithCMConnection:thruConnection];
    
    //overwrite the extended thru connection id with the short suffix
    thruConnectionDOM.ThruID = thruID;
    
    if( ![CMSoftThru.dictionary objectForKey:thruConnectionDOM.ThruID] )
    {
        [CMSoftThru.keys addObject:thruConnectionDOM.ThruID];
        [CMSoftThru.documents addObject:thruConnectionDOM];
        [CMSoftThru.dictionary setObject:thruConnectionDOM forKey:thruConnectionDOM.ThruID];
    }
    else assert(1==0);
    
    //Update Cache containing Thru Connection Keys
    [CMSoftThruConnection saveSoftThruConnectionKeysInCache];
    
    return thruConnectionDOM;//thruConnectionDOM;
}

+(void)loadSoftThruConnections
{
    int numThruConnections = 0;
    CFDataRef data = nil;
    MIDIThruConnectionParams thruParams = {0};
 
    //CMUpdateInputDevices();
    //CMUpdateOutputDevices();
    
    //Uncomment when you fuck up:
    //[CMSoftThruConnection deleteThruConnectionKeysFromCache];
    
    //Load stored thru connection keys from cache
    NSArray * StoredKeys = [CMSoftThruConnection loadSoftThruConnectionKeysFromCache];
    NSLog(@"loadSoftThruConnectionKeysFromCache: %@", StoredKeys);

    //NSArray * StoredKeys = @[@"TM-6"];
    
    //early out (no stored keys to load)
    if( !StoredKeys || ![StoredKeys isKindOfClass:[NSArray class]] || StoredKeys.count < 1) return;
    
    //const char * stored_keys[] = {"eDrumIn"};
    int numStoredKeys = (int)StoredKeys.count;
    
    //For each stored key find a midi thru connection
    for(int keyIndex = 0; keyIndex<numStoredKeys; keyIndex++)
    {
        NSString * nsStoredKey = [StoredKeys objectAtIndex:keyIndex];
        NSString * nsExtendedKey = [NSString stringWithFormat:@"%s.%@", CM_SOFT_THRU_BASE_ID, nsStoredKey];
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
            CMDriverID destDriver = 0;
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
    
    CMClient.numThruConnections = numThruConnections;
}


+(int)type
{
    return CM_SOFT_THRU;
}



+ (CMSoftThruConnection *)sharedInstance {
    static CMSoftThruConnection *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[CMSoftThruConnection alloc] init];
    });
    return sharedInstance;
}

+(NSArray*)requiredProperties
{
    return @[@"ThruID",
             @"Input",
             @"Output",
    ];
}


-(void)saveSoftThruConnection
{
    assert(1==0);
    
    //Save existing thru connection params to CoreMIDI,
    CMSaveThruConnectionParams(self.connection);
    
    //Update DOM with changes
    [self deserializeConnectionParams:self.connection];
    
    //overwrite the extended thru connection id with the short suffix
    //thruConnectionDOM.ThruID = thruID;
    
    //CoreMIDI will only trigger update causing UI to update when connections are created/deleted
    //Not when they are modified
    
    //[[PbAudioAppDelegate sharedInstance] updateThruConnectionWidgets];
}

-(void)replaceSoftThruConnection:(NSString*)ThruID atIndex:(NSUInteger)domIndex
{
    assert(1==0);

    //make sure the new key doesn't already exist on another persistent thru connection
    if( [CMSoftThru.dictionary objectForKey:ThruID]) assert(1==0);
    
    CMSoftThruConnection * thruConnectionDOM = [CMSoftThru.dictionary objectForKey:self.primaryKey];
    if( thruConnectionDOM )
    {
        assert(thruConnectionDOM == [CMSoftThru.documents objectAtIndex:domIndex]);
        assert([thruConnectionDOM.primaryKey localizedCompare:[CMSoftThru.keys objectAtIndex:domIndex]] == NSOrderedSame);

        //Look up or create extended ThruID for deletion of CoreMidi Thru Connection via CMidi API
        NSString * oldThruID = self.primaryKey;
        NSString * oldExtendedThruID = [NSString stringWithFormat:@"%s.%@", CM_SOFT_THRU_BASE_ID, oldThruID];
        NSString * replacementThruID = [NSString stringWithFormat:@"%s.%@", CM_SOFT_THRU_BASE_ID, ThruID];
        
        NSLog(@"CMSoftThruConnection::replaceThruConnection (%@) with (%@)", oldExtendedThruID, replacementThruID);
        
        //find the CMidi thru connection matching this one
        int thruIndex = 0;
        for(thruIndex = 0; thruIndex < CMClient.numThruConnections; thruIndex++)
        {
            if( CMClient.thruConnections[thruIndex].connection == self.connection->connection) break;
        }
        
        //ensure the thru connection we found has the same string id
        assert([oldExtendedThruID localizedCompare:[NSString stringWithUTF8String:CMClient.thruConnections[thruIndex].name]] == NSOrderedSame);
        
        //delete and recreate the CoreMIDI thru connection with a new string id
        CMReplaceThruConnectionAtIndex(self.connection, replacementThruID.UTF8String, thruIndex);

        //set the new thru connection id on our DOM object
        [self setPrimaryKey:ThruID];
        [self deserializeConnectionParams:self.connection];

        //Update our master DOM lists
        [CMSoftThru.dictionary removeObjectForKey:oldThruID];
        [CMSoftThru.dictionary setObject:self forKey:ThruID];
        [CMSoftThru.keys replaceObjectAtIndex:domIndex withObject:ThruID];

        NSLog(@"CMSoftThru.dictionary = %@", CMSoftThru.dictionary);
        NSLog(@"CMSoftThru.documents = %@", CMSoftThru.documents);
        NSLog(@"CMSoftThru.keys = %@", CMSoftThru.keys);
        
        [CMSoftThruConnection saveSoftThruConnectionKeysInCache];
    }
    else assert(1==0);
}

-(id)init
{
    self = [super init];
    if(self)
    {

    }
    return self;
}

-(CMConnection*)connection
{
    return &_softThruConnection;
}

-(void)deserializeConnectionParams:(CMConnection*)connection
{
    //deserialize values needed for hashing and UI display
    //self.ThruID = [NSString stringWithUTF8String:connection->name];
    self.Input = [NSString stringWithUTF8String:connection->source.name];
    self.Output = [NSString stringWithUTF8String:connection->destination.name];
    self.InputDriver = [NSString stringWithUTF8String:cm_driver_list_pp[connection->source.driverID]];
    self.OutputDriver = [NSString stringWithUTF8String:cm_driver_list_pp[connection->destination.driverID]];

}

-(id)initWithCMConnection:(struct CMConnection*)connection
{
    self = [super init];
    if(self)
    {
        memcpy(&_softThruConnection, connection, sizeof(struct CMConnection));

        //self.ThruID = [NSString stringWithUTF8String:connection->name];
        [self setPrimaryKey:[NSString stringWithUTF8String:connection->name]];
        [self deserializeConnectionParams:connection];
    }
    return self;
}


+(NSString*)primaryKey
{
    return @"ThruID";
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
    NSString * artistMovements = document[@"SoftThruConnections"];
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
        CMSoftThruConnection * thruConnectionDOM = [documents objectAtIndex:i];
        [CMSoftThruConnection deleteSoftThruConnection:thruConnectionDOM];
    }
    
    //Return query result to client
    closure(self, error, results);
}
+(NSString*)domTitle
{
    return @"Thru Connection";
}



@end
