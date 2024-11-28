//#include "asiosys.h"
//#include "asio.h"

#ifndef PBA_ASIO_EXT_H
#define PBA_ASIO_EXT_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32  //IF WIN32

#define NOMINMAX
    //#define NTDDI_VERSION WDK_NTDDI_VERSION
    //#define _WIN32_WINNT _WIN32_WINNT_WIN10 

    // standard definitions
#define STRICT                                                  // enable strict type-checking of Windows handles
#define WIN32_LEAN_AND_MEAN                                     // allow the exclusion of uncommon features
//#define WINVER                                          _WIN32_WINNT_WIN10  // allow the use of Windows XP specific features
//#define _WIN32_WINNT                                    _WIN32_WINNT_WIN10  // allow the use of Windows XP specific features
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES         1       // use the new secure functions in the CRT
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT   1       // use the new secure functions in the CRT


#ifndef CALL
#define CALL(method, object, ... )  (object->lpVtbl->method(object, ##__VA_ARGS__))
#endif

#ifndef __clsid
#define __clsid(x) (REFIID)(&CLSID_ ## x)
#endif

#ifndef __guid
#define __guid(x) (REFIID)(&GUID_ ## x)
#endif

#ifndef __riid
#define __riid(x) (REFIID)(&_IID_ ## x)
#endif

#endif

#pragma mark -- ASIO Typedefs

// force 4 byte alignment
#if defined(_MSC_VER) && !defined(__MWERKS__) 
#pragma pack(push,4)
#elif PRAGMA_ALIGN_SUPPORTED
#pragma options align = native
#endif


#pragma mark -- ASIO Sys

//#ifndef __asiosys__
//#define __asiosys__

	#if defined(_WIN32) || defined(_WIN64)
		#undef MAC 
		#define PPC 0
		#define WINDOWS 1
		#define SGI 0
		#define SUN 0
		#define LINUX 0
		#define BEOS 0

		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1
	
	#elif BEOS
		#define MAC 0
		#define PPC 0
		#define WINDOWS 0
		#define PC 0
		#define SGI 0
		#define SUN 0
		#define LINUX 0
		
		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1
		
		#ifndef DEBUG
			#define DEBUG 0
		 	#if DEBUG
		 		void DEBUGGERMESSAGE(char *string);
		 	#else
		  		#define DEBUGGERMESSAGE(a)
			#endif
		#endif

	#elif SGI
		#define MAC 0
		#define PPC 0
		#define WINDOWS 0
		#define PC 0
		#define SUN 0
		#define LINUX 0
		#define BEOS 0
		
		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1
		
		#ifndef DEBUG
			#define DEBUG 0
		 	#if DEBUG
		 		void DEBUGGERMESSAGE(char *string);
		 	#else
		  		#define DEBUGGERMESSAGE(a)
			#endif
		#endif

	#else	// MAC

		#define MAC 1
		#define PPC 1
		#define WINDOWS 0
		#define PC 0
		#define SGI 0
		#define SUN 0
		#define LINUX 0
		#define BEOS 0

		#define NATIVE_INT64 0
		#define IEEE754_64FLOAT 1

		#ifndef DEBUG
			#define DEBUG 0
			#if DEBUG
				void DEBUGGERMESSAGE(char *string);
			#else
				#define DEBUGGERMESSAGE(a)
			#endif
		#endif
	#endif

//#endif


//- - - - - - - - - - - - - - - - - - - - - - - - -
// Type definitions
//- - - - - - - - - - - - - - - - - - - - - - - - -

// number of samples data type is 64 bit integer
#if NATIVE_INT64
typedef long long int ASIOSamples;
#else
typedef struct ASIOSamples {
	unsigned long hi;
	unsigned long lo;
} ASIOSamples;
#endif

// Timestamp data type is 64 bit integer,
// Time format is Nanoseconds.
#if NATIVE_INT64
typedef long long int ASIOTimeStamp;
#else
typedef struct ASIOTimeStamp {
	unsigned long hi;
	unsigned long lo;
} ASIOTimeStamp;
#endif

// Samplerates are expressed in IEEE 754 64 bit double float,
// native format as host computer
#if IEEE754_64FLOAT
typedef double ASIOSampleRate;
#else
typedef struct ASIOSampleRate {
	char ieee[8];
} ASIOSampleRate;
#endif

// Boolean values are expressed as long
typedef long ASIOBool;
enum {
	ASIOFalse = 0,
	ASIOTrue = 1
};

// Sample Types are expressed as long
typedef long ASIOSampleType;
enum {
	ASIOSTInt16MSB = 0,
	ASIOSTInt24MSB = 1,		// used for 20 bits as well
	ASIOSTInt32MSB = 2,
	ASIOSTFloat32MSB = 3,		// IEEE 754 32 bit float
	ASIOSTFloat64MSB = 4,		// IEEE 754 64 bit double float

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can be more easily used with these
	ASIOSTInt32MSB16 = 8,		// 32 bit data with 16 bit alignment
	ASIOSTInt32MSB18 = 9,		// 32 bit data with 18 bit alignment
	ASIOSTInt32MSB20 = 10,		// 32 bit data with 20 bit alignment
	ASIOSTInt32MSB24 = 11,		// 32 bit data with 24 bit alignment

	ASIOSTInt16LSB = 16,
	ASIOSTInt24LSB = 17,		// used for 20 bits as well
	ASIOSTInt32LSB = 18,
	ASIOSTFloat32LSB = 19,		// IEEE 754 32 bit float, as found on Intel x86 architecture
	ASIOSTFloat64LSB = 20, 		// IEEE 754 64 bit double float, as found on Intel x86 architecture

	// these are used for 32 bit data buffer, with different alignment of the data inside
	// 32 bit PCI bus systems can more easily used with these
	ASIOSTInt32LSB16 = 24,		// 32 bit data with 18 bit alignment
	ASIOSTInt32LSB18 = 25,		// 32 bit data with 18 bit alignment
	ASIOSTInt32LSB20 = 26,		// 32 bit data with 20 bit alignment
	ASIOSTInt32LSB24 = 27,		// 32 bit data with 24 bit alignment

	//	ASIO DSD format.
	ASIOSTDSDInt8LSB1 = 32,		// DSD 1 bit data, 8 samples per byte. First sample in Least significant bit.
	ASIOSTDSDInt8MSB1 = 33,		// DSD 1 bit data, 8 samples per byte. First sample in Most significant bit.
	ASIOSTDSDInt8NER8 = 40,		// DSD 8 bit data, 1 sample per byte. No Endianness required.

	ASIOSTLastEntry
};

/*-----------------------------------------------------------------------------
// DSD operation and buffer layout
// Definition by Steinberg/Sony Oxford.
//
// We have tried to treat DSD as PCM and so keep a consistant structure across
// the ASIO interface.
//
// DSD's sample rate is normally referenced as a multiple of 44.1Khz, so
// the standard sample rate is refered to as 64Fs (or 2.8224Mhz). We looked
// at making a special case for DSD and adding a field to the ASIOFuture that
// would allow the user to select the Over Sampleing Rate (OSR) as a seperate
// entity but decided in the end just to treat it as a simple value of
// 2.8224Mhz and use the standard interface to set it.
//
// The second problem was the "word" size, in PCM the word size is always a
// greater than or equal to 8 bits (a byte). This makes life easy as we can
// then pack the samples into the "natural" size for the machine.
// In DSD the "word" size is 1 bit. This is not a major problem and can easily
// be dealt with if we ensure that we always deal with a multiple of 8 samples.
//
// DSD brings with it another twist to the Endianness religion. How are the
// samples packed into the byte. It would be nice to just say the most significant
// bit is always the first sample, however there would then be a performance hit
// on little endian machines. Looking at how some of the processing goes...
// Little endian machines like the first sample to be in the Least Significant Bit,
//   this is because when you write it to memory the data is in the correct format
//   to be shifted in and out of the words.
// Big endian machine prefer the first sample to be in the Most Significant Bit,
//   again for the same reasion.
//
// And just when things were looking really muddy there is a proposed extension to
// DSD that uses 8 bit word sizes. It does not care what endianness you use.
//
// Switching the driver between DSD and PCM mode
// ASIOFuture allows for extending the ASIO API quite transparently.
// See kAsioSetIoFormat, kAsioGetIoFormat, kAsioCanDoIoFormat
//
//-----------------------------------------------------------------------------*/


//- - - - - - - - - - - - - - - - - - - - - - - - -
// Error codes
//- - - - - - - - - - - - - - - - - - - - - - - - -

typedef long ASIOError;
enum {
	ASE_OK = 0,             // This value will be returned whenever the call succeeded
	ASE_SUCCESS = 0x3f4847a0,	// unique success return value for ASIOFuture calls
	ASE_NotPresent = -1000, // hardware input or output is not present or available
	ASE_HWMalfunction,      // hardware is malfunctioning (can be returned by any ASIO function)
	ASE_InvalidParameter,   // input parameter invalid
	ASE_InvalidMode,        // hardware is in a bad mode or used in a bad mode
	ASE_SPNotAdvancing,     // hardware is not running when sample position is inquired
	ASE_NoClock,            // sample clock or rate cannot be determined or is not present
	ASE_NoMemory            // not enough memory for completing the request
};

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - -
// Time Info support
//- - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct ASIOTimeCode
{
	double          speed;                  // speed relation (fraction of nominal speed)
											// optional; set to 0. or 1. if not supported
	ASIOSamples     timeCodeSamples;        // time in samples
	unsigned long   flags;                  // some information flags (see below)
	char future[64];
} ASIOTimeCode;

typedef enum ASIOTimeCodeFlags
{
	kTcValid = 1,
	kTcRunning = 1 << 1,
	kTcReverse = 1 << 2,
	kTcOnspeed = 1 << 3,
	kTcStill = 1 << 4,

	kTcSpeedValid = 1 << 8
}  ASIOTimeCodeFlags;

typedef struct AsioTimeInfo
{
	double          speed;                  // absolute speed (1. = nominal)
	ASIOTimeStamp   systemTime;             // system time related to samplePosition, in nanoseconds
											// on mac, must be derived from Microseconds() (not UpTime()!)
											// on windows, must be derived from timeGetTime()
	ASIOSamples     samplePosition;
	ASIOSampleRate  sampleRate;             // current rate
	unsigned long flags;                    // (see below)
	char reserved[12];
} AsioTimeInfo;

typedef enum AsioTimeInfoFlags
{
	kSystemTimeValid = 1,            // must always be valid
	kSamplePositionValid = 1 << 1,       // must always be valid
	kSampleRateValid = 1 << 2,
	kSpeedValid = 1 << 3,

	kSampleRateChanged = 1 << 4,
	kClockSourceChanged = 1 << 5
} AsioTimeInfoFlags;

typedef struct ASIOTime                          // both input/output
{
	long reserved[4];                       // must be 0
	struct AsioTimeInfo     timeInfo;       // required
	struct ASIOTimeCode     timeCode;       // optional, evaluated if (timeCode.flags & kTcValid)
} ASIOTime;

/*

using time info:
it is recommended to use the new method with time info even if the asio
device does not support timecode; continuous calls to ASIOGetSamplePosition
and ASIOGetSampleRate are avoided, and there is a more defined relationship
between callback time and the time info.

see the example below.
to initiate time info mode, after you have received the callbacks pointer in
ASIOCreateBuffers, you will call the asioMessage callback with kAsioSupportsTimeInfo
as the argument. if this returns 1, host has accepted time info mode.
now host expects the new callback bufferSwitchTimeInfo to be used instead
of the old bufferSwitch method. the ASIOTime structure is assumed to be valid
and accessible until the callback returns.

using time code:
if the device supports reading time code, it will call host's asioMessage callback
with kAsioSupportsTimeCode as the selector. it may then fill the according
fields and set the kTcValid flag.
host will call the future method with the kAsioEnableTimeCodeRead selector when
it wants to enable or disable tc reading by the device. you should also support
the kAsioCanTimeInfo and kAsioCanTimeCode selectors in ASIOFuture (see example).

note:
the AsioTimeInfo/ASIOTimeCode pair is supposed to work in both directions.
as a matter of convention, the relationship between the sample
position counter and the time code at buffer switch time is
(ignoring offset between tc and sample pos when tc is running):

on input:	sample 0 -> input  buffer sample 0 -> time code 0
on output:	sample 0 -> output buffer sample 0 -> time code 0

this means that for 'real' calculations, one has to take into account
the according latencies.

example:

ASIOTime asioTime;

in createBuffers()
{
	memset(&asioTime, 0, sizeof(ASIOTime));
	AsioTimeInfo* ti = &asioTime.timeInfo;
	ti->sampleRate = theSampleRate;
	ASIOTimeCode* tc = &asioTime.timeCode;
	tc->speed = 1.;
	timeInfoMode = false;
	canTimeCode = false;
	if(callbacks->asioMessage(kAsioSupportsTimeInfo, 0, 0, 0) == 1)
	{
		timeInfoMode = true;
#if kCanTimeCode
		if(callbacks->asioMessage(kAsioSupportsTimeCode, 0, 0, 0) == 1)
			canTimeCode = true;
#endif
	}
}

void switchBuffers(long doubleBufferIndex, bool processNow)
{
	if(timeInfoMode)
	{
		AsioTimeInfo* ti = &asioTime.timeInfo;
		ti->flags =	kSystemTimeValid | kSamplePositionValid | kSampleRateValid;
		ti->systemTime = theNanoSeconds;
		ti->samplePosition = theSamplePosition;
		if(ti->sampleRate != theSampleRate)
			ti->flags |= kSampleRateChanged;
		ti->sampleRate = theSampleRate;

#if kCanTimeCode
		if(canTimeCode && timeCodeEnabled)
		{
			ASIOTimeCode* tc = &asioTime.timeCode;
			tc->timeCodeSamples = tcSamples;						// tc in samples
			tc->flags = kTcValid | kTcRunning | kTcOnspeed;			// if so...
		}
		ASIOTime* bb = callbacks->bufferSwitchTimeInfo(&asioTime, doubleBufferIndex, processNow ? ASIOTrue : ASIOFalse);
#else
		callbacks->bufferSwitchTimeInfo(&asioTime, doubleBufferIndex, processNow ? ASIOTrue : ASIOFalse);
#endif
	}
	else
		callbacks->bufferSwitch(doubleBufferIndex, ASIOFalse);
}

ASIOError ASIOFuture(long selector, void *params)
{
	switch(selector)
	{
		case kAsioEnableTimeCodeRead:
			timeCodeEnabled = true;
			return ASE_SUCCESS;
		case kAsioDisableTimeCodeRead:
			timeCodeEnabled = false;
			return ASE_SUCCESS;
		case kAsioCanTimeInfo:
			return ASE_SUCCESS;
		#if kCanTimeCode
		case kAsioCanTimeCode:
			return ASE_SUCCESS;
		#endif
	}
	return ASE_NotPresent;
};

*/

//- - - - - - - - - - - - - - - - - - - - - - - - -
// application's audio stream handler callbacks
//- - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct ASIOCallbacks
{
	void (*bufferSwitch) (long doubleBufferIndex, ASIOBool directProcess);
	// bufferSwitch indicates that both input and output are to be processed.
	// the current buffer half index (0 for A, 1 for B) determines
	// - the output buffer that the host should start to fill. the other buffer
	//   will be passed to output hardware regardless of whether it got filled
	//   in time or not.
	// - the input buffer that is now filled with incoming data. Note that
	//   because of the synchronicity of i/o, the input always has at
	//   least one buffer latency in relation to the output.
	// directProcess suggests to the host whether it should immedeately
	// start processing (directProcess == ASIOTrue), or whether its process
	// should be deferred because the call comes from a very low level
	// (for instance, a high level priority interrupt), and direct processing
	// would cause timing instabilities for the rest of the system. If in doubt,
	// directProcess should be set to ASIOFalse.
	// Note: bufferSwitch may be called at interrupt time for highest efficiency.

	void (*sampleRateDidChange) (ASIOSampleRate sRate);
	// gets called when the AudioStreamIO detects a sample rate change
	// If sample rate is unknown, 0 is passed (for instance, clock loss
	// when externally synchronized).

	long (*asioMessage) (long selector, long value, void* message, double* opt);
	// generic callback for various purposes, see selectors below.
	// note this is only present if the asio version is 2 or higher

	ASIOTime* (*bufferSwitchTimeInfo) (ASIOTime* params, long doubleBufferIndex, ASIOBool directProcess);
	// new callback with time info. makes ASIOGetSamplePosition() and various
	// calls to ASIOGetSampleRate obsolete,
	// and allows for timecode sync etc. to be preferred; will be used if
	// the driver calls asioMessage with selector kAsioSupportsTimeInfo.
} ASIOCallbacks;

// asioMessage selectors
enum
{
	kAsioSelectorSupported = 1,	// selector in <value>, returns 1L if supported,
								// 0 otherwise
								kAsioEngineVersion,			// returns engine (host) asio implementation version,
															// 2 or higher
															kAsioResetRequest,			// request driver reset. if accepted, this
																						// will close the driver (ASIO_Exit() ) and
																						// re-open it again (ASIO_Init() etc). some
																						// drivers need to reconfigure for instance
																						// when the sample rate changes, or some basic
																						// changes have been made in ASIO_ControlPanel().
																						// returns 1L; note the request is merely passed
																						// to the application, there is no way to determine
																						// if it gets accepted at this time (but it usually
																						// will be).
																						kAsioBufferSizeChange,		// not yet supported, will currently always return 0L.
																													// for now, use kAsioResetRequest instead.
																													// once implemented, the new buffer size is expected
																													// in <value>, and on success returns 1L
																													kAsioResyncRequest,			// the driver went out of sync, such that
																																				// the timestamp is no longer valid. this
																																				// is a request to re-start the engine and
																																				// slave devices (sequencer). returns 1 for ok,
																																				// 0 if not supported.
																																				kAsioLatenciesChanged, 		// the drivers latencies have changed. The engine
																																											// will refetch the latencies.
																																											kAsioSupportsTimeInfo,		// if host returns true here, it will expect the
																																																		// callback bufferSwitchTimeInfo to be called instead
																																																		// of bufferSwitch
																																																		kAsioSupportsTimeCode,		// 
																																																		kAsioMMCCommand,			// unused - value: number of commands, message points to mmc commands
																																																		kAsioSupportsInputMonitor,	// kAsioSupportsXXX return 1 if host supports this
																																																		kAsioSupportsInputGain,     // unused and undefined
																																																		kAsioSupportsInputMeter,    // unused and undefined
																																																		kAsioSupportsOutputGain,    // unused and undefined
																																																		kAsioSupportsOutputMeter,   // unused and undefined
																																																		kAsioOverload,              // driver detected an overload

																																																		kAsioNumMessageSelectors
};

//---------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------------------

//- - - - - - - - - - - - - - - - - - - - - - - - -
// (De-)Construction
//- - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct ASIODriverInfo
{
	long asioVersion;		// currently, 2
	long driverVersion;		// driver specific
	char name[32];
	char errorMessage[124];
	void* sysRef;			// on input: system reference
							// (Windows: application main window handle, Mac & SGI: 0)
} ASIODriverInfo;

typedef struct ASIOClockSource
{
	long index;					// as used for ASIOSetClockSource()
	long associatedChannel;		// for instance, S/PDIF or AES/EBU
	long associatedGroup;		// see channel groups (ASIOGetChannelInfo())
	ASIOBool isCurrentSource;	// ASIOTrue if this is the current clock source
	char name[32];				// for user selection
} ASIOClockSource;

typedef struct ASIOChannelInfo
{
	long channel;			// on input, channel index
	ASIOBool isInput;		// on input
	ASIOBool isActive;		// on exit
	long channelGroup;		// dto
	ASIOSampleType type;	// dto
	char name[32];			// dto
} ASIOChannelInfo;

//- - - - - - - - - - - - - - - - - - - - - - - - -
// Buffer preparation
//- - - - - - - - - - - - - - - - - - - - - - - - -

typedef struct ASIOBufferInfo
{
	ASIOBool isInput;			// on input:  ASIOTrue: input, else output
	long channelNum;			// on input:  channel index
	void* buffers[2];			// on output: double buffer addresses
} ASIOBufferInfo;

enum
{
	kAsioEnableTimeCodeRead = 1,	// no arguments
	kAsioDisableTimeCodeRead,		// no arguments
	kAsioSetInputMonitor,			// ASIOInputMonitor* in params
	kAsioTransport,					// ASIOTransportParameters* in params
	kAsioSetInputGain,				// ASIOChannelControls* in params, apply gain
	kAsioGetInputMeter,				// ASIOChannelControls* in params, fill meter
	kAsioSetOutputGain,				// ASIOChannelControls* in params, apply gain
	kAsioGetOutputMeter,			// ASIOChannelControls* in params, fill meter
	kAsioCanInputMonitor,			// no arguments for kAsioCanXXX selectors
	kAsioCanTimeInfo,
	kAsioCanTimeCode,
	kAsioCanTransport,
	kAsioCanInputGain,
	kAsioCanInputMeter,
	kAsioCanOutputGain,
	kAsioCanOutputMeter,
	kAsioOptionalOne,

	//	DSD support
	//	The following extensions are required to allow switching
	//	and control of the DSD subsystem.
	kAsioSetIoFormat = 0x23111961,		/* ASIOIoFormat * in params.			*/
	kAsioGetIoFormat = 0x23111983,		/* ASIOIoFormat * in params.			*/
	kAsioCanDoIoFormat = 0x23112004,		/* ASIOIoFormat * in params.			*/

	// Extension for drop out detection
	kAsioCanReportOverload = 0x24042012,	/* return ASE_SUCCESS if driver can detect and report overloads */

	kAsioGetInternalBufferSamples = 0x25042012	/* ASIOInternalBufferInfo * in params. Deliver size of driver internal buffering, return ASE_SUCCESS if supported */
};

typedef struct ASIOInputMonitor
{
	long input;		// this input was set to monitor (or off), -1: all
	long output;	// suggested output for monitoring the input (if so)
	long gain;		// suggested gain, ranging 0 - 0x7fffffffL (-inf to +12 dB)
	ASIOBool state;	// ASIOTrue => on, ASIOFalse => off
	long pan;		// suggested pan, 0 => all left, 0x7fffffff => right
} ASIOInputMonitor;

typedef struct ASIOChannelControls
{
	long channel;			// on input, channel index
	ASIOBool isInput;		// on input
	long gain;				// on input,  ranges 0 thru 0x7fffffff
	long meter;				// on return, ranges 0 thru 0x7fffffff
	char future[32];
} ASIOChannelControls;

typedef struct ASIOTransportParameters
{
	long command;		// see enum below
	ASIOSamples samplePosition;
	long track;
	long trackSwitches[16];		// 512 tracks on/off
	char future[64];
} ASIOTransportParameters;

enum
{
	kTransStart = 1,
	kTransStop,
	kTransLocate,		// to samplePosition
	kTransPunchIn,
	kTransPunchOut,
	kTransArmOn,		// track
	kTransArmOff,		// track
	kTransMonitorOn,	// track
	kTransMonitorOff,	// track
	kTransArm,			// trackSwitches
	kTransMonitor		// trackSwitches
};

/*
// DSD support
//	Some notes on how to use ASIOIoFormatType.
//
//	The caller will fill the format with the request types.
//	If the board can do the request then it will leave the
//	values unchanged. If the board does not support the
//	request then it will change that entry to Invalid (-1)
//
//	So to request DSD then
//
//	ASIOIoFormat NeedThis={kASIODSDFormat};
//
//	if(ASE_SUCCESS != ASIOFuture(kAsioSetIoFormat,&NeedThis) ){
//		// If the board did not accept one of the parameters then the
//		// whole call will fail and the failing parameter will
//		// have had its value changes to -1.
//	}
//
// Note: Switching between the formats need to be done before the "prepared"
// state (see ASIO 2 documentation) is entered.
*/
typedef long int ASIOIoFormatType;
enum ASIOIoFormatType_e
{
	kASIOFormatInvalid = -1,
	kASIOPCMFormat = 0,
	kASIODSDFormat = 1,
};

typedef struct ASIOIoFormat_s
{
	ASIOIoFormatType	FormatType;
	char				future[512 - sizeof(ASIOIoFormatType)];
} ASIOIoFormat;

// Extension for drop detection
// Note: Refers to buffering that goes beyond the double buffer e.g. used by USB driver designs
typedef struct ASIOInternalBufferInfo
{
	long inputSamples;			// size of driver's internal input buffering which is included in getLatencies
	long outputSamples;			// size of driver's internal output buffering which is included in getLatencies
} ASIOInternalBufferInfo;

// restore old alignment
#if defined(_MSC_VER) && !defined(__MWERKS__) 
#pragma pack(pop)
#elif PRAGMA_ALIGN_SUPPORTED
#pragma options align = reset
#endif

#pragma mark -- IASIO [COM] Driver

#include "UnknwnBase.h"

/* Forward Declarations */

#ifndef __ASIODRIVER_FWD_DEFINED__
#define __ASIODRIVER_FWD_DEFINED__
typedef interface IASIO IASIO;
#endif 	/* __ASIODRIVER_FWD_DEFINED__ */

/*
interface IASIO : public IUnknown
{

	virtual ASIOBool init(void *sysHandle) = 0;
	virtual void getDriverName(char *name) = 0;
	virtual long getDriverVersion() = 0;
	virtual void getErrorMessage(char *string) = 0;
	virtual ASIOError start() = 0;
	virtual ASIOError stop() = 0;
	virtual ASIOError getChannels(long *numInputChannels, long *numOutputChannels) = 0;
	virtual ASIOError getLatencies(long *inputLatency, long *outputLatency) = 0;
	virtual ASIOError getBufferSize(long *minSize, long *maxSize,
		long *preferredSize, long *granularity) = 0;
	virtual ASIOError canSampleRate(ASIOSampleRate sampleRate) = 0;
	virtual ASIOError getSampleRate(ASIOSampleRate *sampleRate) = 0;
	virtual ASIOError setSampleRate(ASIOSampleRate sampleRate) = 0;
	virtual ASIOError getClockSources(ASIOClockSource *clocks, long *numSources) = 0;
	virtual ASIOError setClockSource(long reference) = 0;
	virtual ASIOError getSamplePosition(ASIOSamples *sPos, ASIOTimeStamp *tStamp) = 0;
	virtual ASIOError getChannelInfo(ASIOChannelInfo *info) = 0;
	virtual ASIOError createBuffers(ASIOBufferInfo *bufferInfos, long numChannels,
		long bufferSize, ASIOCallbacks *callbacks) = 0;
	virtual ASIOError disposeBuffers() = 0;
	virtual ASIOError controlPanel() = 0;
	virtual ASIOError future(long selector,void *opt) = 0;
	virtual ASIOError outputReady() = 0;
};
*/

typedef struct IASIOVtbl
{
	IUnknownVtbl Base;

    STDMETHOD_(ASIOBool, init)(IASIO* This, void* sysHandle) PURE;

    STDMETHOD_(void, getDriverName)(IASIO* This, char* name) PURE;
    STDMETHOD_(long, getDriverVersion)(IASIO* This) PURE;
    STDMETHOD_(long, getErrorMessage)(IASIO* This, char* string) PURE;

    STDMETHOD_(ASIOError, start)(IASIO* This) PURE;
    STDMETHOD_(ASIOError,  stop)(IASIO* This) PURE;
        
    STDMETHOD_(ASIOError, getChannels)(IASIO* This, long* numInputChannels, long* numOutputChannels) PURE;
    STDMETHOD_(ASIOError, getLatencies)(IASIO* This, long* inputLatency, long* outputLatency) PURE;
    STDMETHOD_(ASIOError, getBufferSize)(IASIO* This, long* minSize, long* maxSize, long* preferredSize, long* granularity) PURE;

    STDMETHOD_(ASIOError, canSampleRate)(IASIO* This, ASIOSampleRate sampleRate) PURE;
    STDMETHOD_(ASIOError, getSampleRate)(IASIO* This, ASIOSampleRate* sampleRate) PURE;
    STDMETHOD_(ASIOError, setSampleRate)(IASIO* This, ASIOSampleRate sampleRate) PURE;

    STDMETHOD_(ASIOError, getClockSources)(IASIO* This, ASIOClockSource* clocks, long* numSources) PURE;
    STDMETHOD_(ASIOError, setClockSource)(IASIO* This, long reference) PURE;

    STDMETHOD_(ASIOError, getSamplePosition)(IASIO* This, ASIOSamples* sPos, ASIOTimeStamp* tStamp) PURE;
    STDMETHOD_(ASIOError, getChannelInfo)(IASIO* This, ASIOChannelInfo* info) PURE;

    STDMETHOD_(ASIOError, createBuffers)(IASIO* This, ASIOBufferInfo* bufferInfos, long numChannels, long bufferSize, ASIOCallbacks* callbacks) PURE;
    STDMETHOD_(ASIOError, disposeBuffers)(IASIO* This) PURE;

    STDMETHOD_(ASIOError, controlPanel)(IASIO* This) PURE;
    STDMETHOD_(ASIOError, future)(IASIO* This, long selector, void* opt) PURE;
    STDMETHOD_(ASIOError, outputReady)(IASIO* This) PURE;


    /*
    STDMETHOD_(D2D1_SIZE_U, GetPixelSize)(
        ID2D1Bitmap* This
        ) PURE;

    STDMETHOD_(D2D1_PIXEL_FORMAT, GetPixelFormat)(
        ID2D1Bitmap* This
        ) PURE;

    STDMETHOD_(void, GetDpi)(
        ID2D1Bitmap* This,
        _Out_ FLOAT* dpiX,
        _Out_ FLOAT* dpiY
        ) PURE;

    STDMETHOD(CopyFromBitmap)(
        ID2D1Bitmap* This,
        _In_opt_ CONST D2D1_POINT_2U* destPoint,
        _In_ ID2D1Bitmap* bitmap,
        _In_opt_ CONST D2D1_RECT_U* srcRect
        ) PURE;

    STDMETHOD(CopyFromRenderTarget)(
        ID2D1Bitmap* This,
        _In_opt_ CONST D2D1_POINT_2U* destPoint,
        _In_ ID2D1RenderTarget* renderTarget,
        _In_opt_ CONST D2D1_RECT_U* srcRect
        ) PURE;

    STDMETHOD(CopyFromMemory)(
        ID2D1Bitmap* This,
        _In_opt_ CONST D2D1_RECT_U* dstRect,
        _In_ CONST void* srcData,
        UINT32 pitch
        ) PURE;
    */
} IASIOVtbl;

interface IASIO { CONST struct IASIOVtbl* lpVtbl; };

#define IASIO_QueryInterface(This, riid, ppv) \
    ((This)->lpVtbl->Base.QueryInterface((IUnknown *)This, riid, ppv))

#define IASIO_AddRef(This) \
    ((This)->lpVtbl->Base.AddRef((IUnknown *)This))

#define IASIO_Release(This) \
    ((This)->lpVtbl->Base.Release((IUnknown *)This))

#define IASIO_GetChannels(This, numInputChannels, numOutputChannels) \
    ((This)->lpVtbl->getChannels(numInputChannels, numOutputChannels))


//global 
extern IASIO* theAsioDriver;


#pragma mark -- ASIO Driver List

#define DRVERR			            -5000
#define DRVERR_INVALID_PARAM		DRVERR-1
#define DRVERR_DEVICE_ALREADY_OPEN	DRVERR-2
#define DRVERR_DEVICE_NOT_FOUND		DRVERR-3

#define MAXPATHLEN		        	512
#define MAXDRVNAMELEN		        128

struct asiodrvstruct
{
    int						drvID;
    CLSID					clsid;
    char					dllpath[MAXPATHLEN];
    char					drvname[MAXDRVNAMELEN];
    LPVOID					asiodrv;
    struct asiodrvstruct* next;
};

typedef struct asiodrvstruct ASIODRVSTRUCT;
typedef		ASIODRVSTRUCT* LPASIODRVSTRUCT;

//API methods
void asio_init_driver_list(void);
void asio_release_driver_list(void);

LONG asio_get_num_devices(void);

LONG asio_open_driver(int drvID, LPVOID* asiodrv);
LONG asio_close_driver(int drvID);

LONG asio_get_driver_name(int drvID, char* drvname, int drvnamesize);


#pragma mark -- ASIO Drivers

//int asio_get_current_driver_name(char* name);
long asio_get_driver_names(char** names, long maxDrivers);


//int asio_load_driver(char* name);
//void asio_remove_current_driver(void);


#pragma mark -- ASIO.cpp

//#include "windows.h"
//#include "iasiodrv.h"
//#include "asiodrivers.h"

//extern IASIO* theAsioDriver;

ASIOError asio_init(IASIO* driver, ASIODriverInfo* info);
ASIOError asio_shutdown(void** driverRef, int* driverID);

ASIOError asio_start(IASIO* driver);
ASIOError asio_stop(IASIO* driver);

ASIOError asio_get_channels(IASIO* driver, long* numInputChannels, long* numOutputChannels);
ASIOError asio_get_latencies(IASIO* driver, long* inputLatency, long* outputLatency);
ASIOError asio_get_buffer_size(IASIO* driver, long* minSize, long* maxSize, long* preferredSize, long* granularity);

ASIOError asio_can_sample_rate(IASIO* driver, ASIOSampleRate sampleRate);
ASIOError asio_get_sample_rate(IASIO* driver, ASIOSampleRate* currentRate);
ASIOError asio_set_sample_rate(IASIO* driver, ASIOSampleRate sampleRate);

ASIOError asio_get_sample_position(IASIO* driver, ASIOSamples* sPos, ASIOTimeStamp* tStamp);
ASIOError asio_get_channel_info(IASIO* driver, ASIOChannelInfo* info);

ASIOError asio_create_buffers(IASIO* driver, ASIOBufferInfo* bufferInfos, long numChannels, long bufferSize, ASIOCallbacks* callbacks);
ASIOError asio_dispose_buffers(IASIO* driver);

ASIOError asio_control_panel(IASIO* driver);
ASIOError asio_future(IASIO* driver, long selector, void* opt);
ASIOError asio_output_ready(IASIO* driver);

#ifdef __cplusplus
}
#endif

#endif