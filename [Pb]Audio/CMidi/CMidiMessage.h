//
//  CMidiMessage.h
//  MCUCommander
//
//  Created by Joe Moulton on 12/5/23.
//

#ifndef CMidiMessage_h
#define CMidiMessage_h

#include "CMEndian.h"
#include <CoreMIDI/MIDIMessages.h>                   //Core Midi Protocol Definitions

#define MIDI_SYSEX       0xf0
#define MIDI_EOX         0xf7
#define MIDI_CLOCK       0xf8
#define MIDI_STATUS_MASK 0x80

typedef int32_t CMTimestamp;
typedef CMTimestamp (*CMTimeProcPtr)(void *time_info);
typedef uint32_t CMMessage; /**< @brief see #PmEvent */

typedef CF_ENUM(uint32_t,  CMMessageType)
{
    //CMMessageTypeUnknownF        =   0x00,
    //CMMessageTypeUtility         =   1<<0,    // 1 word
    //CMMessageTypeSystem          =   1<<1,    // 1 word
    //CMMessageTypeChannelVoice1   =   1<<2,    // 1 word - MIDI 1.0
    //CMMessageTypeSysEx           =   1<<3,    // 2 words (Data, but primarily SysEx)
    //CMMessageTypeChannelVoice2   =   1<<4,    // 2 words - MIDI 2.0
    //CMMessageTypeData128         =   1<<5,    // 4 words
    
    CMMessageTypeUtility         =   0x0,    // 1 word
    CMMessageTypeSystem          =   0x1,    // 1 word
    CMMessageTypeChannelVoice1   =   0x2,    // 1 word - MIDI 1.0
    CMMessageTypeSysEx           =   0x3,    // 2 words (Data, but primarily SysEx)
    CMMessageTypeChannelVoice2   =   0x4,    // 2 words - MIDI 2.0
    CMMessageTypeData128         =   0x5,    // 4 words
    CMMessageTypeTimeout         =   0x6,    // Custom KQUEUE Event
    CMMessageTypeUnknownF        =   0xF
    
    // Sizes of undefined message types:
    // 6: 1 word
    // 7: 1 word
    // 8: 2 words
    // 9: 2 words
    // A: 2 words
    // B: 3 words
    // C: 3 words
    // D: 4 words
    // E: 4 words
    // F: 4 words
};

static const char*  CMStringForMIDIMessageType(CMMessageType messageType)
{
    switch (messageType)
    {
        case (CMMessageTypeUtility):
            return "Utility";
        case (CMMessageTypeSystem):
            return "System";
        case (CMMessageTypeChannelVoice1):
            return "MIDI 1.0 Channel Voice Message";
        case (CMMessageTypeSysEx):
            return "Sysex";
        case (CMMessageTypeChannelVoice2):
            return "MIDI 2.0 Channel Voice Message";
        case (CMMessageTypeData128):
            return "MIDI Message Type 128";
        default:
            return "";
    }
}


// kMIDIMessageTypeChannelVoice1 / kMIDIMessageTypeChannelVoice2 status nibbles.
typedef CF_ENUM(uint32_t, CMMessageCVStatus)
{
    // MIDI 1.0
    CMMessageCVStatusNoteOff                =    0x8,
    CMMessageCVStatusNoteOn                 =    0x9,
    CMMessageCVStatusPolyPressure           =    0xA,
    CMMessageCVStatusControlChange          =    0xB,
    CMMessageCVStatusProgramChange          =    0xC,
    CMMessageCVStatusChannelPressure        =    0xD,
    CMMessageCVStatusPitchBend              =    0xE,

    // MIDI 2.0
    CMMessageCVStatusRegisteredPNC           =    0x0, // Per-Note Controller
    CMMessageCVStatusAssignablePNC           =    0x1,
    CMMessageCVStatusRegisteredControl       =    0x2, // Registered Parameter Number (RPN)
    CMMessageCVStatusAssignableControl       =    0x3, // Non-Registered Parameter Number (NRPN)
    CMMessageCVStatusRelRegisteredControl    =    0x4, // Relative
    CMMessageCVStatusRelAssignableControl    =    0x5, // Relative
    CMMessageCVStatusPerNotePitchBend        =    0x6,
    CMMessageCVStatusPerNoteMgmt             =    0xF
};


static const char * CMStringForMIDICVStatus(CMMessageCVStatus status)
{
    switch (status)
    {
        // MIDI 1.0
        case (CMMessageCVStatusNoteOff):
            return "Note Off";
        case (CMMessageCVStatusNoteOn):
            return "Note On";
        case (CMMessageCVStatusPolyPressure):
            return "Poly Pressure";
        case (CMMessageCVStatusControlChange):
              return "Control Change";
        case (CMMessageCVStatusProgramChange):
              return "Program Change";
        case (CMMessageCVStatusChannelPressure):
            return "Channel Pressure";
        case (CMMessageCVStatusPitchBend):
              return "Pitch Bend";
        // MIDI 2.0
        case (CMMessageCVStatusRegisteredPNC):
              return "Registered PNC";
        case (CMMessageCVStatusAssignablePNC):
              return "Assignable PNC";
        case (CMMessageCVStatusRegisteredControl):
            return "Registered Control";
        case (CMMessageCVStatusAssignableControl):
              return "Assignable Control";
        case (CMMessageCVStatusRelRegisteredControl):
            return "Rel Registered Control";
        case (CMMessageCVStatusRelAssignableControl):
            return "Rel Assignable Control";
        case (CMMessageCVStatusPerNotePitchBend):
            return "Per Note PitchBend";
        case(CMMessageCVStatusPerNoteMgmt):
            return "Per Note Mgmt";
        default:
            return "";
    }
    
}

// CMMessageTypeSystem status bytes.
typedef CF_ENUM(uint32_t, CMSystemStatus)
{
    // MIDI 1.0 only
    CMStatusStartOfExclusive        = 0xF0,
    CMStatusEndOfExclusive          = 0xF7,
    
    // MIDI 1.0 and 2.0
    CMStatusMTC                     = 0xF1,
    CMStatusSongPosPointer          = 0xF2,
    CMStatusSongSelect              = 0xF3,
    CMStatusTuneRequest             = 0xF6,
    
    CMStatusTimingClock             = 0xF8,
    CMStatusStart                   = 0xFA,
    CMStatusContinue                = 0xFB,
    CMStatusStop                    = 0xFC,
    CMStatusActiveSending           = 0xFE,
    CMStatusActiveSensing           = CMStatusActiveSending,
    CMStatusSystemReset             = 0xFF
};

// CMMessageTypeSysEx / CMMessageTypeData128 status nibbles.
typedef CF_ENUM(uint32_t, CMSysExStatus) 
{
    CMSysExStatusComplete              = 0x0,
    CMSysExStatusStart                 = 0x1,
    CMSysExStatusContinue              = 0x2,
    CMSysExStatusEnd                   = 0x3,

    // MIDI 2.0
    CMSysExStatusMixedDataSetHeader    = 0x8,
    CMSysExStatusMixedDataSetPayload   = 0x9
};

// CMMessageTypeUtility status nibbles.
typedef CF_ENUM(uint32_t, CMUtilityStatus) 
{
    CMUtilityStatusNOOP                        = 0x0,
    CMUtilityStatusJitterReductionClock        = 0x1,
    CMUtilityStatusJitterReductionTimestamp    = 0x2
};

// MIDI 2.0 Note On/Off Message Attribute Types
typedef CF_ENUM(uint8_t, CMNoteAttribute)
{
    CMNoteAttributeNone                     = 0x0,    // no attribute data
    CMNoteAttributeManufacturerSpecific     = 0x1,    // Manufacturer-specific = unknown
    CMNoteAttributeProfileSpecific          = 0x2,    // MIDI-CI profile-specific data
    CMNoteAttributePitch                    = 0x3    // Pitch 7.9
};

// MIDI 2.0 Program Change Options
typedef CF_OPTIONS(uint8_t, CMProgramChangeOptions)
{
    CMProgramChangeBankValid = 0x1
};

// MIDI 2.0 Per Note Management Options
typedef CF_OPTIONS(uint8_t, CMPerNoteManagementOptions)
{
    CMPerNoteManagementReset = 0x1,
    CMPerNoteManagementDetach = 0x2
};

//==================================================================================================
#pragma mark -
#pragma mark Universal MIDI Packet structs

#pragma pack(push, 4)

typedef uint32_t CMMessage_32;

typedef struct CMMessage_64
{
    uint32_t word0;
    uint32_t word1;
} CMMessage_64;

typedef struct CMMessage_96
{
    uint32_t word0;
    uint32_t word1;
    uint32_t word2;
} CMMessage_96;

typedef struct CMMessage_128
{
    uint32_t word0;
    uint32_t word1;
    uint32_t word2;
    uint32_t word3;
} CMMessage_128;

#pragma pack(pop)


//==================================================================================================
#pragma mark -
#pragma mark UMP message helper functions for reading

/*!
    @struct    MIDIUniversalMessage
    @abstract  A representation of all possible messages stored in a Universal MIDI packet.
*/
typedef struct CMUniversalMessage
{
    CMMessageType      type;    //!< determines which variant in the union is active
    uint8_t            group;   //!< 4 bit MIDI group
    uint8_t            reserved[3];

    union {
        struct {
            CMUtilityStatus status;  //!< determines which variant is active
            union {
                UInt16 jitterReductionClock;      //!< active when status is kMIDIUtilityStatusJitterReductionClock
                UInt16 jitterReductionTimestamp;  //!< active when status is kMIDIUtilityStatusJitterReductionTimestamp
            };
        } utility;   //!< active when type is kMIDIMessageTypeUtility

        struct {
            CMSystemStatus status;  //!< determines which variant is active
            union {
                uint8_t  timeCode;             //!< active when status is kMIDIStatusMTC
                uint16_t songPositionPointer;  //!< active when status is kMIDIStatusSongPosPointer
                uint8_t  songSelect;           //!< active when status is kMIDIStatusSongSelect
                int32_t  deviceSelect;         //!< active when status is kMIDIStatusSongSelect
            };
        } system;   //!< active when type is kMIDIMessageTypeSystem

        struct {
            CMMessageCVStatus  status;   //!< determines which variant is active
            uint8_t            channel;  //!< MIDI channel 0-15
            uint8_t            reserved[3];
            union {
                struct {
                    uint8_t    number;    //!< 7 bit note number
                    uint8_t    velocity;  //!< 7 bit note velocity
                } note;   //!< active when status is kMIDICVStatusNoteOff or kMIDICVStatusNoteOn

                struct
                {
                    uint8_t noteNumber;  //!< 7 bit note number
                    uint8_t pressure;    //!< 7 bit poly pressure data
                } polyPressure;   //!< active when status is kMIDICVStatusPolyPressure

                struct {
                    uint8_t index;  //!< 7 bit index of control parameter
                    uint8_t data;   //!< 7 bit value for control parameter
                } controlChange;  //!< active when status is kMIDICVStatusControlChange

                uint8_t  program;          //!< 7 bit program nr, active when status is kMIDICVStatusProgramChange
                uint8_t  channelPressure;  //!< 7 bit channel pressure, active when status is kMIDICVStatusChannelPressure
                UInt16   pitchBend;        //!< 7 bit pitch bend active when status is kMIDICVStatusPitchBend
            };
        } channelVoice1;   //!< active when type is kMIDIMessageTypeChannelVoice1

        struct {
            CMSysExStatus     status;
            uint8_t           channel;  //!< MIDI channel 0-15
            uint8_t           data[6];  //!< SysEx data, 7 bit values
            uint8_t           reserved;
        } sysEx;   //!< active when type is kMIDIMessageTypeSysEx

        struct {
            CMMessageCVStatus  status;   //!< determines which variant is active
            uint8_t            channel;  //!< MIDI channel
            uint8_t            reserved[3];
            union {
                struct {
                    uint8_t                number;         //!< 7 bit note number
                    CMNoteAttribute        attributeType;  //!< attribute type
                    uint8_t                velocity;       //!< note velocity
                    uint8_t                attribute;      //!< attribute data
                } note;    //!< active when status is kMIDICVStatusNoteOff or kMIDICVStatusNoteOn

                struct
                {
                    uint8_t                noteNumber;  //!< 7 bit note number
                    uint8_t                reserved;
                    uint32_t               pressure;    //!< pressure value
                } polyPressure;         //!< active when status is kMIDICVStatusPolyPressure

                struct {
                    uint8_t                index;     //!< 7 bit controller number
                    uint8_t                reserved;
                    uint32_t               data;      //!< controller value
                } controlChange;      //!< active when status is kMIDICVStatusControlChange

                struct {
                    CMProgramChangeOptions options;
                    uint8_t                program;     //!< 7 bit program number
                    uint8_t                reserved[2];
                    uint16_t               bank;        //!< 14 bit bank
                } programChange;         //!< active when status is kMIDICVStatusProgramChange

                struct {
                    uint8_t data;         //!< channel pressure data
                    uint8_t  reserved[2];
                } channelPressure;       //!< active when status is kMIDICVStatusChannelPressure

                struct {
                    uint8_t data;         //!< pitch bend data
                    uint8_t  reserved[2];
                } pitchBend;             //!< active when status is kMIDICVStatusPitchBend

                struct {
                    uint8_t  noteNumber;   //!< 7 bit note number
                    uint8_t  index;        //!< 7 bit controller number
                    uint32_t data;         //!< controller data
                } perNoteController;     //!< active when status is kMIDICVStatusRegisteredPNC or kMIDICVStatusAssignablePNC

                struct {
                    uint8_t  bank;    //!< 7 bit bank
                    uint8_t  index;   //!< 7 bit controller number
                    uint32_t data;    //!< controller data
                } controller;       //!< active when status is any of kMIDICVStatusRegisteredControl, kMIDICVStatusAssignableControl, kMIDICVStatusRelRegisteredControl, or kMIDICVStatusRelAssignableControl

                struct {
                    uint8_t  noteNumber;   //!< 7 bit note number
                    uint8_t  reserved;
                    uint32_t bend;         //!< per note pitch bend value
                } perNotePitchBend;      //!< active when status is kMIDICVStatusPerNotePitchBend

                struct {
                    uint8_t note;         //!< 7 bit note number
                    CMPerNoteManagementOptions options;
                    uint8_t reserved[4];
                } perNoteManagement;    //!< active when status is kMIDICVStatusPerNoteMgmt
            };
        } channelVoice2;  //!< active when type is kMIDIMessageTypeChannelVoice2

        struct {
            CMSysExStatus status;    //!< determines which variant is active
            union {
                struct {
                    uint8_t byteCount;   //!< byte count of data including stream ID (1-14 bytes)
                    uint8_t streamID;
                    uint8_t data[13];
                    uint8_t reserved;
                } sysex8;   //!< active when status any of kMIDISysExStatusComplete, kMIDISysExStatusStart, kMIDISysExStatusContinue, or kMIDISysExStatusEnd

                struct {
                    uint8_t mdsID;      //!< mixed data set ID
                    uint8_t data[14];
                    uint8_t reserved;
                } mixedDataSet;   //!< active when status is kMIDISysExStatusMixedDataSetHeader or kMIDISysExStatusMixedDataSetPayload
            };
        } data128;   //!< active when type is kMIDIMessageTypeData128

        struct {
            uint32_t words[4];  //!< up to four 32 bit words
        } unknown;            //!< active when type is kMIDIMessageTypeUnknownF
    };
} CMUniversalMessage;


typedef struct CMEvent
{
    CMMessage      message;
    CMTimestamp    timestamp;
}CMEvent;

#pragma mark -- MIDI Note

/*
Represents a MIDI note
1. Categorizes incoming notes (messages) in to 9 categories (subset of full MIDI specification)
2. Useful conversion routines:
   A) Translate MIDI status byte into correct category (statusToType).
   B) Channel Number from packet
   C) e.g. 'C4' from 60, string from packet
   D) e.g. note 60 from packet - noteNumberFromnPacket
   E) octave from packet
   F) e.g. 'C4' from 60 - noteNumberToNoteName
*/


/*
typedef enum
{
    kNoteOff,
    kNoteOn,
    kActiveSensing,
    kAfterTouch,
    kControlChange,
    kProgramChange,
    kChannelPressure,
    kPitchWheel,
    kUndefined
} MidiType;
*/

typedef struct CMidiNote
{
    //const MIDIPacket *packet;
    const MIDIEventPacket* packet;
    
    MIDITimeStamp        timeStamp; /*UInt64*/
    
    UInt8 status;
    UInt8 data1;
    UInt8 data2;
    UInt8 channel;
    
    CMMessageCVStatus statusType;

    //NSString *NoteName;
    //NSString *statusTypeDescription;
    
}CMidiNote;

typedef struct CMTriggerCursor
{
    uint64_t offset;
}CMTriggerCursor;

typedef struct CMTriggerMessage
{
    MIDITimeStamp       timestamp;
    CMTriggerCursor        cursor; //sample cursor value for rescheduling
    union
    {
        uint32_t          word;
        uint8_t           status, channel, note, velocity;
    };
    
}CMTriggerMessage;

/*
void CMidiNoteInitWithPacket(CMidiNote* note, MIDIEventPacket * packet);
{
       packet   = p;
       timeStamp = p->timeStamp;
       
       statusType = [MidiNote statusToType:p]; // moved to here
       
       channel = [MidiNote packetToChannelNumber:p];    // only set for this category
       status=(p->length > 0) ? p->data[0] : 0;
       data1=(p->length > 1) ? p->data[1] : 0;
       data2=(p->length > 2) ? p->data[2] : 0;
           
       statusTypeDescription = MidiType_toString[statusType];
}
*/

static uint8_t CMNoteNumberFromEventWord(const UInt32 packetWord)
{
    //return (packet->wordCount > 0) ? packet->data[1] : 0;
    return (packetWord >> 8) & 0x000000ff;
}

static uint8_t CMNoteVelocityFromEventWord(const UInt32 packetWord)
{
    //return (packet->wordCount > 0) ? packet->data[1] : 0;
    return packetWord & 0x000000ff;
}

//+(MidiType ) statusToType:(const MIDIPacket *)packet;
//+(UInt8) packetToChannelNumber:(const MIDIPacket *) packet;
//+(NSString *) stringFromMIDIPacket:(const MIDIPacket *) packet;
//+(UInt8) noteNumberFromMIDIPacket:(const MIDIPacket *) packet;
//+(UInt8) octaveFromMIDIPacket:(const MIDIPacket *) packet;

//+(NSString *)noteNumberToNoteName:(int)noteNumber;
//+(int)       noteNumberToOctave:(int)noteNumber;
//+(NSString *)noteNumberToString:(int)noteNumber;

#pragma mark -- MCU Protocol

typedef union MCUMessage
{
    CMMessage word;
    uint8_t   type, status, identifier, value;  //Always MIDI 1.0
    //uint8_t status, // CC = V-POT (0xB0), PB = FADER (0xE0)
}MCUMessage;

enum MCUNoteEventEnum
{
    MCUNoteEvent_REC1               = 0x00,
    MCUNoteEvent_REC2               = 0x01,
    MCUNoteEvent_REC3               = 0x02,
    MCUNoteEvent_REC4               = 0x03,
    MCUNoteEvent_REC5               = 0x04,
    MCUNoteEvent_REC6               = 0x05,
    MCUNoteEvent_REC7               = 0x06,
    MCUNoteEvent_REC8               = 0x07,
    
    MCUNoteEvent_SOLO1              = 0x08,
    MCUNoteEvent_SOLO2              = 0x09,
    MCUNoteEvent_SOLO3              = 0x0A,
    MCUNoteEvent_SOLO4              = 0x0B,
    MCUNoteEvent_SOLO5              = 0x0C,
    MCUNoteEvent_SOLO6              = 0x0D,
    MCUNoteEvent_SOLO7              = 0x0E,
    MCUNoteEvent_SOLO8              = 0x0F,
    
    MCUNoteEvent_MUTE1              = 0x10,
    MCUNoteEvent_MUTE2              = 0x11,
    MCUNoteEvent_MUTE3              = 0x12,
    MCUNoteEvent_MUTE4              = 0x13,
    MCUNoteEvent_MUTE5              = 0x14,
    MCUNoteEvent_MUTE6              = 0x15,
    MCUNoteEvent_MUTE7              = 0x16,
    MCUNoteEvent_MUTE8              = 0x17,
    
    MCUNoteEvent_SELECT1            = 0x18,
    MCUNoteEvent_SELECT2            = 0x19,
    MCUNoteEvent_SELECT3            = 0x1A,
    MCUNoteEvent_SELECT4            = 0x1B,
    MCUNoteEvent_SELECT5            = 0x1C,
    MCUNoteEvent_SELECT6            = 0x1D,
    MCUNoteEvent_SELECT7            = 0x1E,
    MCUNoteEvent_SELECT8            = 0x1F,
    
    MCUNoteEvent_VSELECT1           = 0x20,
    MCUNoteEvent_VSELECT2           = 0x21,
    MCUNoteEvent_VSELECT3           = 0x22,
    MCUNoteEvent_VSELECT4           = 0x23,
    MCUNoteEvent_VSELECT5           = 0x24,
    MCUNoteEvent_VSELECT6           = 0x25,
    MCUNoteEvent_VSELECT7           = 0x26,
    MCUNoteEvent_VSELECT8           = 0x27,
    
    //VPOT ASSIGN
    MCUNoteEvent_ASSIGN_TRACK       = 0x28,
    MCUNoteEvent_ASSIGN_SEND        = 0x29,
    MCUNoteEvent_ASSIGN_PAN         = 0x2A,
    MCUNoteEvent_ASSIGN_PLUGIN      = 0x2B,
    MCUNoteEvent_ASSIGN_EQ          = 0x2C,
    MCUNoteEvent_ASSIGN_INSTRUMENT  = 0x2D,
    
    //FADER BANKS
    MCUNoteEvent_BANK_LEFT          = 0x2E,
    MCUNoteEvent_BANK_RIGHT         = 0x2F,
    MCUNoteEvent_CHANNEL_LEFT       = 0x30,
    MCUNoteEvent_CHANNEL_RIGHT      = 0x31,
    MCUNoteEvent_FLIP               = 0x32,
    MCUNoteEvent_GLOBAL_VIEW        = 0x33,
    
    //DISPLAY
    MCUNoteEvent_NAME_VALUE         = 0x34,
    MCUNoteEvent_SMPTE_BEATS        = 0x35,

    //FUNCTION SELECT
    MCUNoteEvent_F1                 = 0x36,
    MCUNoteEvent_F2                 = 0x37,
    MCUNoteEvent_F3                 = 0x38,
    MCUNoteEvent_F4                 = 0x39,
    MCUNoteEvent_F5                 = 0x3A,
    MCUNoteEvent_F6                 = 0x3B,
    MCUNoteEvent_F7                 = 0x3C,
    MCUNoteEvent_F8                 = 0x3D,

    //GLOBAL VIEW
    MCUNoteEvent_MIDI_TRACKS        = 0x3E,
    MCUNoteEvent_INPUTS             = 0x3F,
    MCUNoteEvent_AUDIO_TRACKS       = 0x40,
    MCUNoteEvent_AUDIO_INSTRUMENT   = 0x41,
    MCUNoteEvent_AUX                = 0x42,
    MCUNoteEvent_BUSSES             = 0x43,
    MCUNoteEvent_OUTPUTS            = 0x44,
    MCUNoteEvent_USER               = 0x45,
    
    //MODIFIERS
    MCUNoteEvent_SHIFT              = 0x46,
    MCUNoteEvent_OPTION             = 0x47,
    MCUNoteEvent_CTRL               = 0x48,
    MCUNoteEvent_CMD_ALT            = 0x49,
    
    //AUTOMATION
    MCUNoteEvent_AUTOMATION_READ    = 0x4A,
    MCUNoteEvent_AUTOMATION_WRITE   = 0x4B,
    MCUNoteEvent_AUTOMATION_TRIM    = 0x4C,
    MCUNoteEvent_AUTOMATION_TOUCH   = 0x4D,
    MCUNoteEvent_AUTOMATION_LATCH   = 0x4E,
    MCUNoteEvent_AUTOMATION_GROUP   = 0x4F,
    
    //UTILITIES
    MCUNoteEvent_UTIL_SAVE          = 0x50,
    MCUNoteEvent_UTIL_UNDO          = 0x51,
    MCUNoteEvent_UTIL_CANCEL        = 0x52,
    MCUNoteEvent_UTIL_ENTER         = 0x53,
    
    //DAW
    MCUNoteEvent_MARKER             = 0x54,
    MCUNoteEvent_NUDGE              = 0x55,
    MCUNoteEvent_CYCLE              = 0x56,
    MCUNoteEvent_DROP               = 0x57,
    MCUNoteEvent_REPLACE            = 0x58,
    MCUNoteEvent_CLICK              = 0x59,
    MCUNoteEvent_SOLO               = 0x5A,

    //TRANSPORT
    MCUNoteEvent_TRANSPORT_REWIND   = 0x5B,
    MCUNoteEvent_TRANSPORT_FASTFWD  = 0x5C,
    MCUNoteEvent_TRANSPORT_STOP     = 0x5D,
    MCUNoteEvent_TRANSPORT_PLAY     = 0x5E,
    MCUNoteEvent_TRANSPORT_RECORD   = 0x5F,

    //STAR CURSOR
    MCUNoteEvent_CURSOR_UP          = 0x60,
    MCUNoteEvent_CURSOR_DOWN        = 0x61,
    MCUNoteEvent_CURSOR_LEFT        = 0x62,
    MCUNoteEvent_CURSOR_RIGHT       = 0x63,
    MCUNoteEvent_CURSOR_ZOOM        = 0x64,
    MCUNoteEvent_CURSOR_SCRUB       = 0x65,

    //USER (Where are these on the hardware?)
    MCUNoteEvent_USER_SWITCH_A      = 0x66,
    MCUNoteEvent_USER_SWITCH_B      = 0x67,
    
    //FADER TOUCH
    MCUNoteEvent_TOUCH_FADER1       = 0x68,
    MCUNoteEvent_TOUCH_FADER2       = 0x69,
    MCUNoteEvent_TOUCH_FADER3       = 0x6A,
    MCUNoteEvent_TOUCH_FADER4       = 0x6B,
    MCUNoteEvent_TOUCH_FADER5       = 0x6C,
    MCUNoteEvent_TOUCH_FADER6       = 0x6D,
    MCUNoteEvent_TOUCH_FADER7       = 0x6E,
    MCUNoteEvent_TOUCH_FADER8       = 0x6F,
    MCUNoteEvent_TOUCH_FADER9       = 0x70,  //MASTER
    
    //MISC LED
    MCUNoteEvent_SMPTE_LED          = 0x71,  //
    MCUNoteEvent_BEATS_LED          = 0x72,  //
    MCUNoteEvent_RUDE_SOLO_LIGHT    = 0x73,  //??
    //Missing
    //Missing
    MCUNoteEvent_TOUCH_RELAY_CLICK  = 0x76  //??

};

#define MCUNoteEvent_MAX MCUNoteEvent_TOUCH_RELAY_CLICK + 1
typedef uint8_t MCUNoteEvent;




#endif /* CMidiMessage_h */
