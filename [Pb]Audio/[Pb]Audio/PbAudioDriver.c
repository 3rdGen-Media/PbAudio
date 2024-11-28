//
//  PbAudioDriver.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/10/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"
#include "pba_asio_ext.h"


#ifdef _WIN32
#include <assert.h>
#endif

//Private
volatile PBAudioDriver _AudioDrivers[PBA_MAX_DEVICES] = { 0 };
volatile char          _DriverNames[PBA_MAX_DEVICES][128] = { 0 };
volatile uint32_t      _DriverCount = 0;

#ifdef _WIN32
//IMMDeviceEnumerator* _PBADeviceEnumerator = NULL;
#endif


#ifdef __APPLE__
//const CFStringRef kPBADeviceDefaultInputChangedNotification  = CFSTR("PBADeviceDefaultInputDeviceChangedNotification");
//const CFStringRef kPBADeviceDefaultOutputChangedNotification = CFSTR("PBADeviceDefaultOutputDeviceChangedNotification");
//const CFStringRef kPBADevicesAvailableChangedNotification    = CFSTR("PBADeviceAvailableDevicesChangedNotification");
#endif

#pragma mark -- Opaque Driver Wrapper API


PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverInitCOM()
{
    OSStatus hr = 0;
    asio_init_driver_list();
    hr = CoInitialize(0); //Single Threaded COM Apartment is Mandatory for IASIO implementation
    return hr;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverReleaseCOM()
{
    OSStatus hr = 0;
    asio_release_driver_list();
    CoUninitialize();
    return hr;
}

//PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverReset()
//{
//
//}


PB_AUDIO_API PB_AUDIO_INLINE LONG PBAudioAvailableDriverCount(void)
{
    return asio_get_num_devices();
}

PB_AUDIO_API PB_AUDIO_INLINE LONG PBAudioGetDriverName(int drvID, char* drvname, int drvnamesize)
{
    return asio_get_driver_name(drvID, drvname, drvnamesize);
}

//PB_AUDIO_API PB_AUDIO_INLINE int PBAudioGetActiveDriverName(char* drvname)
//{
//    return asio_get_current_driver_name(drvname);
//}

//TO DO: load driver id by name instead of list index?
PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioLoadVendorDriver(void** driverRef, PBAudioDriverID currentDriverID, PBAudioDriverID* driverID)
{
    char dname[64]   = "\0";
    char curName[64] = "\0";

    PBAudioDriverID currentID = currentDriverID;

    //get the current driver name in case it needs to be restored on failure
    //TO DO: store name instead of list index ast driverid
    if( currentDriverID > -1) PBAudioGetDriverName(currentDriverID, curName, 32);

    for (long i = 0; i < PBAudioAvailableDriverCount(); i++)
    {
        //if (!asio_get_driver_name(i, dname, 32) && !strcmp(name, dname)) //check driver by name
        if( i == *driverID ) //check driver by list index
        {
            //curName[0] = 0;
            //asio_get_current_driver_name(curName);	// in case we fail...
            
            //asio_remove_current_driver();
            if (*driverID != -1) asio_close_driver(*driverID); *driverID = -1;

            if (!asio_open_driver(i, (void**)driverRef))
            {
                *driverID = i;
                return 0;
            }
            else
            {
                *driverRef = 0;
                if (curName[0] /* && strcmp(dname, curName)*/) PBAudioLoadVendorDriver(driverRef, currentID, &currentID); //asio_load_driver(curName); // try restore
            }
            break;
        }
    }
    return -1;
}


PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioInitVendorDriver(void* driver, void* context)
{
    ASIODriverInfo driverInfo;
	PBAStreamContext* stream = (PBAStreamContext*)context;

    // initialize the driver	
	if (asio_init(driver, &driverInfo) != ASE_OK) return -1;

	//Debug
	fprintf(stdout, "asioVersion:   %d\n"
					"driverVersion: %d\n"
					"Name:          %s\n"
					"ErrorMessage:  %s\n",
					driverInfo.asioVersion, 
					driverInfo.driverVersion,
					driverInfo.name,        
					driverInfo.errorMessage);

	//init channel info and get buffer size, sample rate, and output ready properties needed for buffer creation
	if (init_asio_static_data(stream, &driverInfo) != 0)    return -1;
	if (PBAudioDriverCreateBuffers(stream, &driverInfo) != ASE_OK) return -1;
    return 0;
}


PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverShutdown(void** driverRef, PBAudioDriverID* driverID)
{
	if (asio_shutdown(driverRef, driverID) == ASE_OK) return 0;
	assert(1 == 0);
	return -1;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverStart(void* driver)
{
	if (asio_start(driver) == ASE_OK)
	{
		fprintf(stdout, "\nASIO Driver started successfully.\n\n");
		return 0;
	}
	else assert(1 == 0);

	return -1;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverStop(void* driver)
{
	if (asio_stop(driver) == ASE_OK)
	{
		fprintf(stdout, "\nASIO Driver stopped successfully.\n\n");
		return 0;
	}
	else assert(1 == 0);

	return -1;
}

PB_AUDIO_API PB_AUDIO_INLINE double PBAudioDriverGetSampleRate(void* driver)
{
	double sampleRate = 0.0;
	ASIOError err = asio_get_sample_rate(driver, &sampleRate); assert(err == ASE_OK);
	return sampleRate;
}


PB_AUDIO_API PB_AUDIO_INLINE int PBAudioDriverCanSampleRate(void* driver, double sampRate)
{
	if (asio_can_sample_rate(driver, sampRate) == ASE_OK) return 1;
	return 0;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverSetSampleRate(void* driver, double sampRate, void* context)
{
	ASIODriverInfo driverInfo;
	PBAStreamContext* stream = (PBAStreamContext*)context;

	//IASIO::set_sample_rate
	if (asio_set_sample_rate(driver, sampRate) != ASE_OK) return -1;

	//IASIO::displose_buffers (when would this ever fail?)
	PBAudioDriverDisposeBuffers(driver);								

	//init channel info and get buffer size, sample rate, and output ready properties needed for buffer creation
	if (init_asio_static_data(stream, &driverInfo) != 0) assert(1 == 0); 

	//IASIO::create_buffers
	if (PBAudioDriverCreateBuffers(stream, &driverInfo) != ASE_OK) return -1;

	return 0;
}

PB_AUDIO_API PB_AUDIO_INLINE long PBAudioDriverGetBufferSize(void* driver, long* minSize, long* maxSize, long* preferredSize, long* granularity)
{
	long min  = 0;
	long max  = 0;
	long pref = 0;
	long gran = 0;

	ASIOError err = asio_get_buffer_size(driver, &min, &max, &pref, &gran); assert(err == ASE_OK);

	if (minSize) *minSize = min;
	if (maxSize) *maxSize = max;
	if (preferredSize) *preferredSize = pref;
	if (granularity) *granularity = gran;

	return pref;
}


PB_AUDIO_API PB_AUDIO_INLINE void PBAudioDriverControlPanel(void* driverRef)
{
    asio_control_panel(driverRef);
}



//asio_dispose_buffers(stream->driver);



#pragma mark -- ASIO Callbacks

enum
{
	// number of input and outputs supported by the host application
	// you can change these to higher or lower values
	kASIOMaxInputChannels  = 64,
	kASIOMaxOutputChannels = 64
};

ASIOBufferInfo   bufferInfos[kASIOMaxInputChannels + kASIOMaxOutputChannels]; // buffer info's
ASIOChannelInfo channelInfos[kASIOMaxInputChannels + kASIOMaxOutputChannels]; // channel info's

#include "timeapi.h"

unsigned long get_sys_reference_time()
{	// get the system reference time
#if defined(_WIN32)
	return timeGetTime();
#elif defined(__APPLE__)
	static const double twoRaisedTo32 = 4294967296.;
	UnsignedWide ys;
	Microseconds(&ys);
	double r = ((double)ys.hi * twoRaisedTo32 + (double)ys.lo);
	return (unsigned long)(r / 1000.);
#endif
}

//----------------------------------------------------------------------------------
// conversion from 64 bit ASIOSample/ASIOTimeStamp to double float
#if NATIVE_INT64
#define ASIO64toDouble(a)  (a)
#else
const double twoRaisedTo32 = 4294967296.;
#define ASIO64toDouble(a)  ((a).lo + (a).hi * twoRaisedTo32)
#endif


long PBAudioDriverFormat(long sampleType)
{
	// OK do processing for the outputs only
	switch (sampleType)
	{
		case ASIOSTInt16LSB:
			return 2;
			break;
		case ASIOSTInt24LSB:		// used for 20 bits as well
			return 3;
			break;
		case ASIOSTInt32LSB:
			return 4;
			break;
		case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
			return 4;
			break;
		case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
			return 8;
			break;

			// these are used for 32 bit data buffer, with different alignment of the data inside
			// 32 bit PCI bus systems can more easily used with these
		case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
		case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
			return 4;
			break;

		case ASIOSTInt16MSB:
			return 2;
			break;
		case ASIOSTInt24MSB:		// used for 20 bits as well
			return 3;
			break;
		case ASIOSTInt32MSB:
			return 4;
			break;
		case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
			return 4;
			break;
		case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
			return 8;
			break;

			// these are used for 32 bit data buffer, with different alignment of the data inside
			// 32 bit PCI bus systems can more easily used with these
		case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
		case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
		case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
			return 4;
			break;
	}
}
ASIOTime* PBAudioDriverSwapBufferTimeInfo(ASIOTime* timeInfo, long index, ASIOBool processNow)
{	
	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.
	
	//TO DO:  Steinberg provided no way to pass user context to audio callback?
	PBAStreamContext* stream = &PBAudio.OutputStreams[0];

	static long processedSamples = 0;

	// Information from ASIOGetSamplePosition()
	// data is converted to double floats for easier use, however 64 bit integer can be used, too
	double         nanoSeconds;
	double         samples;
	double         tcSamples;	// time code samples

	// bufferSwitchTimeInfo()
	ASIOTime       tInfo;			// time info state
	unsigned long  sysRefTime;      // system reference time, when bufferSwitch() was called

	// store the timeInfo for later use
	tInfo = *timeInfo;

	// get the time stamp of the buffer, not necessary if no
	// synchronization to other media is required
	if (timeInfo->timeInfo.flags & kSystemTimeValid)	 nanoSeconds = ASIO64toDouble(timeInfo->timeInfo.systemTime);
	else												 nanoSeconds = 0;

	if (timeInfo->timeInfo.flags & kSamplePositionValid) samples     = ASIO64toDouble(timeInfo->timeInfo.samplePosition);
	else												 samples     = 0;

	if (timeInfo->timeCode.flags & kTcValid)			 tcSamples   = ASIO64toDouble(timeInfo->timeCode.timeCodeSamples);
	else												 tcSamples   = 0;

	// get the system reference time
	sysRefTime = get_sys_reference_time();

#if WINDOWS && _DEBUG
	// a few debug messages for the Windows device driver developer
	// tells you the time when driver got its interrupt and the delay until the app receives
	// the event notification.
	static double last_samples = 0;
	
	if (samples - last_samples == 0) return;
	//char tmp[128];
	//sprintf(tmp, "diff: %d / %d ms / %d ms / %d samples                 \n", sysRefTime - (long)(nanoSeconds / 1000000.0), sysRefTime, (long)(nanoSeconds / 1000000.0), (long)(samples - last_samples));
	//OutputDebugStringA(tmp);

	last_samples = samples;
#endif

	// buffer size in samples
	long buffSize = stream->bufferFrameCount;

	// perform the processing
	for (int i = 0; i < stream->nInputChannels + stream->nOutputChannels; i++)
	{
		if (bufferInfos[i].isInput == false)
		{
			// OK do processing for the outputs only
			switch (channelInfos[i].type)
			{
			case ASIOSTInt16LSB:
				memset(bufferInfos[i].buffers[index], 0, buffSize * 2);
				break;
			case ASIOSTInt24LSB:		// used for 20 bits as well
				memset(bufferInfos[i].buffers[index], 0, buffSize * 3);
				break;
			case ASIOSTInt32LSB:
				memset(bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			case ASIOSTFloat32LSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				memset(bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			case ASIOSTFloat64LSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				memset(bufferInfos[i].buffers[index], 0, buffSize * 8);
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
			case ASIOSTInt32LSB16:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32LSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32LSB24:		// 32 bit data with 24 bit alignment
				memset(bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;

			case ASIOSTInt16MSB:
				memset(bufferInfos[i].buffers[index], 0, buffSize * 2);
				break;
			case ASIOSTInt24MSB:		// used for 20 bits as well
				memset(bufferInfos[i].buffers[index], 0, buffSize * 3);
				break;
			case ASIOSTInt32MSB:
				memset(bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			case ASIOSTFloat32MSB:		// IEEE 754 32 bit float, as found on Intel x86 architecture
				memset(bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			case ASIOSTFloat64MSB: 		// IEEE 754 64 bit double float, as found on Intel x86 architecture
				memset(bufferInfos[i].buffers[index], 0, buffSize * 8);
				break;

				// these are used for 32 bit data buffer, with different alignment of the data inside
				// 32 bit PCI bus systems can more easily used with these
			case ASIOSTInt32MSB16:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB18:		// 32 bit data with 18 bit alignment
			case ASIOSTInt32MSB20:		// 32 bit data with 20 bit alignment
			case ASIOSTInt32MSB24:		// 32 bit data with 24 bit alignment
				memset(bufferInfos[i].buffers[index], 0, buffSize * 4);
				break;
			}
		}
	}

	//If driver buffers are valid...
	if (stream->driverID > 0 && bufferInfos[stream->nInputChannels].buffers[index] )
	{
		stream->driverThreadID = GetCurrentThreadId();

		//Configure Output Buffers for Renderpass(es)
		PBABuffer outBufferL = { 1, stream->format.wBitsPerSample / 8, bufferInfos[stream->nInputChannels].buffers[index] };
		PBABuffer outBufferR = { 1, stream->format.wBitsPerSample / 8, bufferInfos[stream->nInputChannels + 1].buffers[index] };
		PBABuffer outBuffers[2] = { outBufferL, outBufferR };
		PBABufferList bufferList = { 2, outBuffers };

		// Clearing the output buffer is critical for DSP routines unless such routines inherently overwrite the buffer
		//PBABufferListSilenceWithFormat(&bufferList, &streamContext->format, 0, streamContext->bufferFrameCount);

		//Execute the client render callback
		stream->outputpass(&bufferList, stream->bufferFrameCount, NULL, stream);

		//memcpy(bufferInfos[64].buffers[index], bufferInfos[19].buffers[index], buffSize * 4);

		// finally if the driver supports the ASIOOutputReady() optimization, do it here, all data are in place
		if (stream->latencyCompensation) asio_output_ready(stream->driver);

	}


	//if (processedSamples >= stream->sampleRate * 20.0) asioDriverInfo.stopped = true;
	//else
	
	processedSamples += buffSize;

	return 0L;
}


//----------------------------------------------------------------------------------
void PBAudioDriverSwapBuffers(long index, ASIOBool processNow)
{	
	// the actual processing callback.
	// Beware that this is normally in a seperate thread, hence be sure that you take care
	// about thread synchronization. This is omitted here for simplicity.

	//TO DO:  Steinberg provided no way to pass user context to audio callback?
	PBAStreamContext* stream = &PBAudio.OutputStreams[0];

	// as this is a "back door" into the bufferSwitchTimeInfo a timeInfo needs to be created
	// though it will only set the timeInfo.samplePosition and timeInfo.systemTime fields and the according flags
	ASIOTime  timeInfo;
	memset (&timeInfo, 0, sizeof (timeInfo));

	// get the time stamp of the buffer, not necessary if no synchronization to other media is required
	if (asio_get_sample_position(stream->driver, &timeInfo.timeInfo.samplePosition, &timeInfo.timeInfo.systemTime) == ASE_OK) timeInfo.timeInfo.flags = kSystemTimeValid | kSamplePositionValid;
	else assert(1==0);

	PBAudioDriverSwapBufferTimeInfo(&timeInfo, index, processNow);
}


//----------------------------------------------------------------------------------
void PBAudioDriverSampleRateChanged(ASIOSampleRate sRate)
{
	// Do whatever you need to do if the sample rate changed -- usually this only happens during external sync.
	// Audio processing is not stopped by the driver, actual sample rate might not have even changed, 
	// or maybe only the sample rate status of an AES/EBU or S/PDIF digital input at the audio device changed.
	// You might have to update time/sample related conversion routines, etc.

	//API Note:  I have not observed this callback in the wild yet.

	assert(1 == 0);
}

//----------------------------------------------------------------------------------
long PBAudioDriverNotification(long selector, long value, void* message, double* opt)
{
	//TO DO:  Steinberg provided no way to pass user context to audio callback?
	PBAStreamContext* stream = &PBAudio.OutputStreams[0];

	// currently the parameters "value", "message" and "opt" are not used.
	long ret = 0;
	switch(selector)
	{
		case kAsioSelectorSupported:
		{
			//register messages to filter those desired to be dispatched by the driver
			if (value == kAsioResetRequest
				|| value == kAsioEngineVersion
				|| value == kAsioResyncRequest
				//|| value == kAsioBufferSizeChange
				|| value == kAsioLatenciesChanged
				// the following three were added for ASIO 2.0, you don't necessarily have to support them
				|| value == kAsioSupportsTimeInfo
				|| value == kAsioSupportsTimeCode
				|| value == kAsioSupportsInputMonitor)
				ret = 1L;
			break;
		}
		case kAsioResetRequest:
		{
			// Request driver reset. if accepted, this will close the driver and re-open it again 
			// some drivers need to reconfigure for instance when the sample rate changes, or some basic
			// changes have been made in ASIO_ControlPanel(). the request is merely passed to the application, 
			//there is no way to determine if it gets accepted at this time (but it usually will be).

			// defer the task and perform the reset of the driver during the next "safe" situation
			// You cannot reset the driver right now, as this code is called from the driver.
			// Reset the driver is done by completely destruct: ie asio_stop(), asio_dispose_buffers(), asio_shutdown()
			// Afterwards you initialize the driver again.

			//API Note:  This callback seems to issue once after all other notifications as a result of the buffer size changing

			//[PbAudio]: Issue one off async call that will reset the driver on a COM single threaded apartment
			//			 Notification of the client isn't strictly necessary because they will receive a property changed notification via AMSNotificationClient callbacks
#ifdef _WIN32
			_beginthreadex(NULL, 0, (_beginthreadex_proc_type)PBAudioStreamReset, stream, 0, NULL);
#else

#endif
			ret = 1L;
			break;
		}
		case kAsioResyncRequest:
		{
			// The driver went out of sync, such that the timestamp is no longer valid. 
			// this is a request to re-start the engine and slave devices (sequencer). 
			// returns 1 for ok, 0 if not supported.

			// This informs the application that the driver encountered some non fatal data loss.
			// It is used for synchronization purposes of different media. Added mainly to work around 
			// the Win16Mutex problems in Windows 95/98 with the Windows Multimedia system, which could 
			// lose data because the Mutex was hold too long by another thread.
			// However a driver can issue it in other situations, too.

			//API Note: This callback seems to issue a lot of times when the buffer size changes

			ret = 1L;
			break;
		}
		case kAsioBufferSizeChange:
		{
			// Not yet supported, will currently always return 0L. For now, use kAsioResetRequest instead.
			// Once implemented, the new buffer size is expected in <value>, and on success returns 1L

			return 0L;
		}
		case kAsioLatenciesChanged:
		{
			// The drivers latencies have changed. The engine will refetch the latencies.

			// This will inform the host application that the driver's latencies were changed.
			// Beware, this does not mean that the buffer sizes have changed!
			// You might need to update internal delay data.

			//API Note: This callback doesn't seem to trigger at all when the buffer size changes

			//long   buffSize = PBAudioDriverGetBufferSize(PBAudio.OutputStreams[0].driver, NULL, NULL, NULL, NULL);
			//assert(stream->bufferFrameCount == buffSize);

			ret = 1L;
			break;
		}
		case kAsioEngineVersion:
		{
			// return the supported ASIO version of the host application
			// If a host applications does not implement this selector, ASIO 1.0 is assumed
			// by the driver
			ret = 2L;
			break;
		}
		case kAsioSupportsTimeInfo:
		{
			// informs the driver wether the asioCallbacks.bufferSwitchTimeInfo() callback is supported.
			// For compatibility with ASIO 1.0 drivers the host application should always support the "old" bufferSwitch method, too.
			ret = 1;
			break;
		}
		case kAsioSupportsTimeCode:
		{
			// informs the driver wether application is interested in time code info.
			// If an application does not need to know about time code, the driver has less work
			// to do.
			ret = 1;
			break;
		}
	}
	return ret;
}


//----------------------------------------------------------------------------------
ASIOError PBAudioDriverCreateBuffers(PBAStreamContext* stream, ASIODriverInfo* driverInfo)
{
    // create buffers for all inputs and outputs of the card with the 
    // preferredSize from ASIOGetBufferSize() as buffer size
    long i;
    ASIOError result;

    // fill the bufferInfos from the start without a gap

   
    ASIOBufferInfo* info = &bufferInfos; //asioDriverInfo->bufferInfos;

    ASIOCallbacks asioCallbacks = { &PBAudioDriverSwapBuffers, &PBAudioDriverSampleRateChanged, &PBAudioDriverNotification, &PBAudioDriverSwapBufferTimeInfo };

    // prepare inputs (Though this is not necessaily required, no opened inputs will work, too
    //if (stream->nInputChannels > kMaxInputChannels) asioDriverInfo->inputBuffers = kMaxInputChannels;
    //else                                            asioDriverInfo->inputBuffers = stream->nInputChannels;

    for (i = 0; i < stream->nInputChannels; i++, info++)
    {
        info->isInput = ASIOTrue;
        info->channelNum = i;
        info->buffers[0] = info->buffers[1] = 0;
    }

    // prepare outputs
    //if (stream->nOutputChannels > kMaxOutputChannels) asioDriverInfo->outputBuffers = kMaxOutputChannels;
    //else                                              asioDriverInfo->outputBuffers = stream->nOutputChannels;

    for (i = 0; i < stream->nOutputChannels; i++, info++)
    {
        info->isInput = ASIOFalse;
        info->channelNum = i;
        info->buffers[0] = info->buffers[1] = 0;
    }

    // create and activate buffers
    result = asio_create_buffers(stream->driver, bufferInfos, stream->nInputChannels + stream->nOutputChannels, stream->bufferFrameCount, &asioCallbacks);
    if (result == ASE_OK)
    {
        // now get all the buffer details, sample word length, name, word clock group and activation
        for (i = 0; i < stream->nInputChannels + stream->nOutputChannels; i++)
        {
            channelInfos[i].channel = bufferInfos[i].channelNum;
            channelInfos[i].isInput = bufferInfos[i].isInput;
            result = asio_get_channel_info(stream->driver, &channelInfos[i]);
            if (result != ASE_OK)
                break;
        }

        if (result == ASE_OK)
        {
            // get the input and output latencies
            // Latencies often are only valid after ASIOCreateBuffers()
            // (input latency is the age of the first sample in the currently returned audio block)
            // (output latency is the time the first sample in the currently returned audio block requires to get to the output)

			long inputLatency, outputLatency;
            result = asio_get_latencies(stream->driver, &inputLatency, &outputLatency);
            if (result == ASE_OK) fprintf(stdout, "ASIOGetLatencies (input: %d, output: %d);\n", inputLatency, outputLatency);

			//TO DO:  convert to seconds?
			stream->inputLatency  = (double)inputLatency;
			stream->outputLatency = (double)outputLatency;

			//populate compatible system format from driver details
			long bytesPerSample			  = PBAudioDriverFormat(channelInfos[stream->nInputChannels].type);

			stream->format.wFormatTag	  = stream->nOutputChannels > 2 ? WAVE_FORMAT_EXTENSIBLE : WAVE_FORMAT_PCM;
			stream->format.nChannels      = stream->nOutputChannels;
			stream->format.nSamplesPerSec = (DWORD)stream->sampleRate;
			stream->format.nBlockAlign	  = 1 * bytesPerSample;
			stream->format.wBitsPerSample = bytesPerSample * 8;

			stream->format.extendedFormat.FormatExt.SubFormat = KSDATAFORMAT_SUBTYPE_PCM;

			stream->target				  = PBAStreamFormatGetType(&stream->format) + 4; //enumerate a sample packing protocol for the given format
			if (stream->target < 4) 		stream->target += 4;						 //add 4 to make the type noninterleaved (ASIO driver types are observed to be noninterleaved)

        }
    }
    return result;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAudioDriverDisposeBuffers(void* driverRef)
{
	if (asio_dispose_buffers(driverRef) == ASE_OK)
	{
		for (int i = 0; i < kASIOMaxInputChannels + kASIOMaxOutputChannels; i++)
		{
			bufferInfos[i].buffers[0] = NULL; //channel front buffer
			bufferInfos[i].buffers[1] = NULL; //channel back  buffer
		}

		return 0;
	}
	assert(1 == 0);
	return -1;

}

//----------------------------------------------------------------------------------
// collect the informational data of the driver
long init_asio_static_data(PBAStreamContext* stream, ASIODriverInfo* driverInfo)
{
    ASIOError result = 0;

    long minSize, maxSize;
    long preferredSize, granularity;


    // get the number of available channels
    result = asio_get_channels(stream->driver, &stream->nInputChannels, &stream->nOutputChannels);
    if (result == ASE_OK)
    {
        printf("ASIOGetChannels (inputs: %d, outputs: %d);\n", stream->nInputChannels, stream->nOutputChannels);

        // get the usable buffer sizes
        if (asio_get_buffer_size(stream->driver, &minSize, &maxSize, &preferredSize, &granularity) == ASE_OK)
        {
            stream->bufferFrameCount = preferredSize;

            fprintf(stdout, "ASIOGetBufferSize (min: %d, max: %d, preferred: %d, granularity: %d);\n", minSize, maxSize, preferredSize, granularity);

            // get the currently selected sample rate
            if (asio_get_sample_rate(stream->driver, &stream->sampleRate) == ASE_OK)
            {
                printf("ASIOGetSampleRate (sampleRate: %f);\n", stream->sampleRate);
                if (stream->sampleRate <= 0.0 || stream->sampleRate > 96000.0)
                {
                    // Driver does not store it's internal sample rate, so set it to a known one.
                    // Usually you should check beforehand, that the selected sample rate is valid with ASIOCanSampleRate().
                    if (asio_set_sample_rate(stream->driver, 44100.0) == ASE_OK)
                    {
                        if (asio_get_sample_rate(stream->driver, &stream->sampleRate) == ASE_OK) printf("ASIOGetSampleRate (sampleRate: %f);\n", stream->sampleRate);
                        else return -6;
                    }
                    else return -5;
                }

                // check wether the driver requires the ASIOOutputReady() optimization
                // (can be used by the driver to reduce output latency by one block)
                if (asio_output_ready(stream->driver) == ASE_OK) stream->latencyCompensation = true;
                else											 stream->latencyCompensation = false;

                printf("ASIOOutputReady(); - %s\n", stream->latencyCompensation ? "Supported" : "Not supported");

                return 0;
            }
            return -3;
        }
        return -2;
    }
    return -1;
}