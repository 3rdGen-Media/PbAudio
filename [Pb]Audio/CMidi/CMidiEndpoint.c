//
//  CMEndpoint.c
//  CoreMIDI Thru
//
//  Created by Joe Moulton on 1/4/23.
//

#include "CMidiEndpoint.h"

#include <assert.h>

#ifdef __APPLE__

#if !TARGET_OS_OSX
#include <Endian.h>
#endif

//
// Code taken from http://developer.apple.com/qa/qa2004/qa1374.html
//////////////////////////////////////
// Obtain the name of an endpoint without regard for whether it has connections.
// The result should be released by the caller.
CFStringRef CMEndpointName(MIDIEndpointRef endpoint, bool isExternal, CMDriverID *driverID)
{
    CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
    CFStringRef str;
    //*driverID = FALSE;
  
    // begin with the endpoint's name
    str = NULL;
    MIDIObjectGetStringProperty(endpoint, kMIDIPropertyName, &str);
    if (str != NULL) {
        CFStringAppend(result, str);
        CFRelease(str);
    }
    MIDIEntityRef entity = 0;
    MIDIEndpointGetEntity(endpoint, &entity);
    if (entity == 0) return result; //probably virtual

    if (!isExternal) // detect IAC devices
    {
        //extern const CFStringRef kMIDIPropertyDriverOwner;
        MIDIObjectGetStringProperty(entity, kMIDIPropertyDriverOwner, &str);
        if (str != NULL) {
            char s[32]; // driver name may truncate, but that's OK
            CFStringGetCString(str, s, 31, kCFStringEncodingUTF8);
            s[31] = 0;  // make sure it is terminated just to be safe
            //fprintf(stderr, "driver %s\n", s);
            
            int isIAC = (strcmp(s, cm_driver_list[CM_DRIVER_APPLE_IAC]) == 0);
            *driverID = CM_DRIVER_APPLE_IAC * isIAC + *driverID * !isIAC;
        }
    }

    if (CFStringGetLength(result) == 0) {
        // endpoint name has zero length -- try the entity
        str = NULL;
        MIDIObjectGetStringProperty(entity, kMIDIPropertyName, &str);
        if (str != NULL) {
            CFStringAppend(result, str);
            CFRelease(str);
        }
    }
    // now consider the device's name
    MIDIDeviceRef device = 0;
    MIDIEntityGetDevice(entity, &device);
    if (device == 0)
        return result;
  
    str = NULL;
    MIDIObjectGetStringProperty(device, kMIDIPropertyName, &str);
    if (CFStringGetLength(result) == 0) {
        CFRelease(result);
        return str;
    }
    if (str != NULL) {
        // if an external device has only one entity, throw away
        // the endpoint name and just use the device name
        if (isExternal && MIDIDeviceGetNumberOfEntities(device) < 2) {
            CFRelease(result);
            return str;
        } else {
            if (CFStringGetLength(str) == 0) {
                CFRelease(str);
                return result;
            }
            // does the entity name already start with the device name?
            // (some drivers do this though they shouldn't)
            // if so, do not prepend
            if (CFStringCompareWithOptions(result, /* endpoint name */
                        str, /* device name */
                        CFRangeMake(0, CFStringGetLength(str)), 0) !=
                kCFCompareEqualTo) {
                // prepend the device name to the entity name
                if (CFStringGetLength(result) > 0)
                    CFStringInsert(result, 0, CFSTR(" "));
                CFStringInsert(result, 0, str);
            }
            CFRelease(str);
        }
    }
    return result;
}


// Obtain the name of an endpoint, following connections.
// The result should be released by the caller.
CFStringRef CMConnectedEndpointName(MIDIEndpointRef endpoint, CMDriverID *driverID)
{
    CFMutableStringRef result = CFStringCreateMutable(NULL, 0);
    CFStringRef str;
    OSStatus err;
    long i;
  
    // Does the endpoint have connections?
    CFDataRef connections = NULL;
    long nConnected = 0;
    bool anyStrings = false;
    err = MIDIObjectGetDataProperty(endpoint, kMIDIPropertyConnectionUniqueID,
                                    &connections);
    if (connections != NULL) {
        // It has connections, follow them
        // Concatenate the names of all connected devices
        nConnected = CFDataGetLength(connections) /
                     (int32_t) sizeof(MIDIUniqueID);
        if (nConnected) {
            const SInt32 *pid = (const SInt32 *)(CFDataGetBytePtr(connections));
            for (i = 0; i < nConnected; ++i, ++pid) {
                MIDIUniqueID id = EndianS32_BtoN(*pid);
                MIDIObjectRef connObject;
                MIDIObjectType connObjectType;
                err = MIDIObjectFindByUniqueID(id, &connObject,
                                               &connObjectType);
                if (err == noErr) {
                    if (connObjectType == kMIDIObjectType_ExternalSource  ||
                        connObjectType == kMIDIObjectType_ExternalDestination) {
                        // Connected to an external device's endpoint (>=10.3)
                        str = CMEndpointName((MIDIEndpointRef)(connObject), true, driverID);
                    } else {
                        // Connected to an external device (10.2)
                        // (or something else, catch-all)
                        str = NULL;
                        MIDIObjectGetStringProperty(connObject,
                                                    kMIDIPropertyName, &str);
                    }
                    if (str != NULL) {
                        if (anyStrings)
                            CFStringAppend(result, CFSTR(", "));
                        else anyStrings = true;
                        CFStringAppend(result, str);
                        CFRelease(str);
                    }
                }
            }
        }
        CFRelease(connections);
    }
    if (anyStrings)
        return result; // caller should release result

    CFRelease(result);

    // Here, either the endpoint had no connections, or we failed to
    // obtain names for any of them.
    return CMEndpointName(endpoint, false, driverID);
}

char *CMFullEndpointName(MIDIEndpointRef endpoint, char* cmname, CMDriverID *driverID)
{
    /* Thanks to Dan Wilcox for fixes for Unicode handling */
    CFStringRef fullName = CMConnectedEndpointName(endpoint, driverID);
    CFIndex utf16_len = CFStringGetLength(fullName) + 1;
    CFIndex max_byte_len = CFStringGetMaximumSizeForEncoding(utf16_len, kCFStringEncodingUTF8) + 1;
    //char* cmname = (char *)malloc(CFStringGetLength(fullName) + 1);

    /* copy the string into our buffer; note that there may be some wasted
       space, but the total waste is not large */
    CFStringGetCString(fullName, cmname, max_byte_len, kCFStringEncodingUTF8);

    /* clean up */
    if (fullName) CFRelease(fullName);
    return cmname;
}

#else

char* CMFullEndpointName(MIDIEndpointRef endpoint, char* cmname, CMDriverID* driverID)
{
    assert(1 == 0);
    return NULL;
}

#endif
