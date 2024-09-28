//
//  PbAudioFileStream.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 11/6/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

#include <math.h>

#ifdef _WIN32
#include <propvarutil.h>  // PropVariantToInt64
#pragma comment(lib, "propsys.lib")
#endif


PB_AUDIO_API PB_AUDIO_INLINE void xng_load_wav_samples(PBAFileRef wav)
{
    if( !wav) return;

    //load and decode png from file or buffer
    XNZ_WAV_ARCHIVE archive = {0};

    //archive.file.buffer   = img->pixelData; 
    //archive.file.size     = img->size; 
    //xnz_wav_open(&archive,  wav->file.);
    
    //if(img->filepath) xnz_wave_close(&archive);
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamClose(ExtAudioFileRef inputAudioFileRef)
{
    OSStatus err = noErr;

    if (inputAudioFileRef)
    {

#ifndef XNZ_AUDIO
#ifdef __APPLE__
        ExtAudioFileDispose(inputAudioFileRef);
#else
        CALL(Release, inputAudioFileRef);
        //assert(1 == 0);
#endif
#else
        xnz_wav_close(&inputAudioFileRef);
#endif
        if( err )
        {
            fprintf(stderr, "PBAFileStreamClose Error disposing of extended audio file context");
        }

    }
    inputAudioFileRef = NULL;
    
    //if (outputAudioFileRef)
    //    ExtAudioFileDispose(outputAudioFileRef);
    
    return err;
}

#ifdef __APPLE__

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamOpen(const char * fileURL, const char * fileExt, PBAStreamFormatRef converterFormat, PBAFileRef inputAudioFileRef)//PBAStreamContext * streamContext, PBAStreamLatencyReport * report, double renderTime, double bufferDuration);
{
    //for streaming audio files from disk
    //PBAStreamFormat     inputFileFormat;
    //ExtAudioFileRef     inputAudioFileRef = NULL;

    //float *audioFileStream[NUM_CHANNELS] __attribute__ ((aligned (16)));
    //int audioFileStreamSizeInBytes;
    //SInt64 audioFileStreamSizeInSamples;        //must be Sint64 for extAudio api
    //volatile int32_t audioFileStreamFramesReturned;
        
    //int state = playerState;
    //playerState = AAFP_PAUSE;

    //[self clearCircularBuffers];
    //[self closeAudioFileStream];
    
    //[self initAudioFileStreamProperties];

    
    char buffer[1024] = "\0";
    // Open input audio file
    OSStatus                                 err = noErr;
    //AudioStreamBasicDescription            converterFormat;
    uint32_t                                 thePropertySize = sizeof(PBAStreamFormat);
    //ExtAudioFileRef                        outputAudioFileRef = NULL;
    //AudioStreamBasicDescription            outputFileFormat;
    
    
    
    //NSURL *inURL = fileURL;
    //NSURL *outURL = [NSURL fileURLWithPath:outPath];

    //Create CFStringRef from c-style string
    //printf("\nfileURL = %s", fileURL);
    //CFStringRef filePath  = CFStringCreateWithCString(kCFAllocatorDefault, fileURL, kCFStringEncodingUTF8 );
    //assert(filePath);
    //CFShow(filePath);
    //CFURLRef    bundleResourcesURL =        CFURLCreateWithString(kCFAllocatorDefault, filePath, NULL);
    
    /*
    CFBundleRef mainBundle = CFBundleGetMainBundle();
    assert(mainBundle);
    
    // Get a reference to the file's URL
    CFStringRef cfFileString = CFStringCreateWithCString(kCFAllocatorDefault, fileURL, CFStringGetSystemEncoding());
    CFStringRef cfFileExtString = CFStringCreateWithCString(NULL, fileExt, CFStringGetSystemEncoding());
    
    //CFShow(cfFileString);
    //CFShow(cfFileExtString);
    CFURLRef bundleResourcesURL = CFBundleCopyResourceURL(mainBundle, cfFileString, cfFileExtString, NULL);
    assert(bundleResourcesURL);
    
    // Convert the URL reference into a string reference
    CFStringRef filePath = CFURLCopyFileSystemPath(bundleResourcesURL, kCFURLPOSIXPathStyle);
    
    // Get the system encoding method
    CFStringEncoding encodingMethod = CFStringGetSystemEncoding();
    
    // Convert the string reference into a C string
    const char *path = CFStringGetCStringPtr(filePath, encodingMethod);
    
    printf("\nbundlePath = %s\n", path);
    */
    
#ifndef XNZ_AUDIO

    strcat(buffer, fileURL );
    
    
    CFStringRef fileStringRef  = CFStringCreateWithCString(kCFAllocatorDefault, buffer, kCFStringEncodingUTF8 );
    assert(fileStringRef);
    CFShow(fileStringRef);
    
    //FYI, CFURLCreateWithString does NOT work for file URLs!!! Must use the following method to convert file system paths (ie files on disk)
    CFURLRef    bundleResourcesURL2 = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, fileStringRef, kCFURLPOSIXPathStyle, false);

    // Open input audio file
    err = ExtAudioFileOpenURL(bundleResourcesURL2, &(inputAudioFileRef->file));
    if (err)
    {
    
        if (inputAudioFileRef)
            ExtAudioFileDispose((inputAudioFileRef->file));
        
        //if (outputAudioFileRef)
        //    ExtAudioFileDispose(outputAudioFileRef);
        
        printf("PBAFileStreamOpen::ExtAudioFileOpenURL Error opening audio file (%d).", err);
        
        return err;
    }
    
    assert(inputAudioFileRef);
    
    // Get input audio format
    //bzero(&inputFileFormat, sizeof(inputFileFormat)); //deprecated; use memcpy
    memset(&(inputAudioFileRef->sourceFormat), 0, sizeof(PBAStreamFormat) );

    err = ExtAudioFileGetProperty((inputAudioFileRef->file), kExtAudioFileProperty_FileDataFormat,
                                  &thePropertySize, &(inputAudioFileRef->sourceFormat));
    if (err)
    {
        
        if (inputAudioFileRef)
            ExtAudioFileDispose((inputAudioFileRef->file));
        
        //if (outputAudioFileRef)
        //    ExtAudioFileDispose(outputAudioFileRef);
        
        printf("PBAFileStreamOpen::ExtAudioFileGetProperty Error getting input audio file format.");
        
        return err;
    }
    
    // only mono or stereo audio files are supported
    
    if (inputAudioFileRef->sourceFormat.mChannelsPerFrame > 2)
    {
        err = kExtAudioFileError_InvalidDataFormat;
        printf("PBAFileStreamOpen only mono or stereo audio files are currently supported");
        return err;
    }
    
    
    // Enable an audio converter on the input audio data by setting
    // the kExtAudioFileProperty_ClientDataFormat property. Each
    // read from the input file returns data in linear pcm format.
    
    //AudioStreamBasicDescription * description;
    //[self setDefaultAudioFormatFlags:description numChannels:inputFileFormat.mChannelsPerFrame];
    
    
    //[self setDefaultAudioFormatFlags:&converterFormat numChannels:2];
    // now set the client format to what we want on read (LPCM, 32-bit floating point)
    //converterFormat = _audioController.audioDescription;
    
    //Is this bad practice? To modify the input parameter on the stack?
    converterFormat.mChannelsPerFrame = inputAudioFileRef->sourceFormat.mChannelsPerFrame;

    err = ExtAudioFileSetProperty((inputAudioFileRef->file), kExtAudioFileProperty_ClientDataFormat, sizeof(converterFormat), &converterFormat);
    if (err)
    {
        fprintf(stdout, "PBAFileStreamOpen::ExtAudioFileSetProperty Error setting conversion file format.");
        //NSLog(@"%d", err);
    }
    
    memcpy( &(inputAudioFileRef->conversionFormat), &converterFormat, sizeof(PBAStreamFormat) ) ;
    //inputAudioFileRef->conversionFormat.mChannelsPerFrame = inputAudioFileRef->sourceFormat.mChannelsPerFrame;
    
    fprintf(stdout, "\nInput File Format...");
    PBAStreamFormatPrint(&(inputAudioFileRef->sourceFormat));

    fprintf(stdout, "\nConversion Format...");
    PBAStreamFormatPrint(&(inputAudioFileRef->conversionFormat));
    
    // Handle the case of reading from a mono input file and writing to a stereo
    // output file by setting up a channel map. The mono output is duplicated in the left and right channel.
    if (inputAudioFileRef->sourceFormat.mChannelsPerFrame == 1 && converterFormat.mChannelsPerFrame == 2)
    {
        SInt32 channelMap[2] = { 0, 0 };
        
        // Get the underlying AudioConverterRef
        AudioConverterRef convRef = NULL;
        UInt32 size = sizeof(AudioConverterRef);
        
        err = ExtAudioFileGetProperty((inputAudioFileRef->file), kExtAudioFileProperty_AudioConverter, &size, &convRef);
        if (err)
        {
            fprintf(stdout, "\nPBAFileStreamOpen::ExtAudioFileGetProperty Error getting AudioConverterRef.");
            //NSLog(@"%d", err);
        }
    
        assert(convRef);
        
        err = AudioConverterSetProperty(convRef, kAudioConverterChannelMap, sizeof(channelMap), channelMap);
        if (err)
        {
            fprintf(stdout, "\nPBAFileStreamOpen::ExtAudioFileSetProperty Error setting channel map on AudioConverterRef.");
            //NSLog(@"%d", err);
        }
    }
    
    //get size of audio file in samples
    UInt32 size = sizeof(SInt64);
    err = ExtAudioFileGetProperty((inputAudioFileRef->file), kExtAudioFileProperty_FileLengthFrames, &size, &(inputAudioFileRef->numFrames) );
    if( err )
    {
        fprintf(stdout, "\nPBAFileStreamOpen::ExtAudioFileGetProperty Error getting file length in frames.");
        //NSLog(@"%d", err);
    }
    else
        fprintf(stdout, "\nPBAFileStreamOpen::ExtAudioFileGetProperty length of file in frames:   %llu", inputAudioFileRef->numFrames);

    
    //IF YOU WANT TO CONVERT TO DIFFERENT AUDIO FILE FORMAT AND WRITE TO DISK
    
    // Output file is typically a caff file, but the user could emit some other
    // common file types. If a file exists already, it is deleted before writing
    // the new audio file.
/*
    if (mFormatID == kAudioFormatAppleIMA4) {
        [self _setIMA4AudioFormatFlags:&outputFileFormat numChannels:converterFormat.mChannelsPerFrame];
    } else if (mFormatID == kAudioFormatMPEG4AAC) {
        [self _setAACAudioFormatFlags:&outputFileFormat numChannels:converterFormat.mChannelsPerFrame];
    } else if (mFormatID == kAudioFormatAppleLossless) {
        [self _setALACAudioFormatFlags:&outputFileFormat numChannels:converterFormat.mChannelsPerFrame];
    } else if (mFormatID == kAudioFormatLinearPCM) {
        [self _setDefaultAudioFormatFlags:&outputFileFormat numChannels:converterFormat.mChannelsPerFrame];
    } else {
        err = kExtAudioFileError_InvalidDataFormat;
        goto reterr;
    }
    
    UInt32 flags = kAudioFileFlags_EraseFile;
    
    err = ExtAudioFileCreateWithURL((CFURLRef)outURL, audioFileTypeID, &outputFileFormat,
                                    NULL, flags, &outputAudioFileRef);
    if (err)
    {
        // -48 means the file exists already
        goto reterr;
    }
    assert(outputAudioFileRef);
    
    // Enable converter when writing to the output file by setting the client
    // data format to the pcm converter we created earlier.
    
    err = ExtAudioFileSetProperty(outputAudioFileRef, kExtAudioFileProperty_ClientDataFormat,
                                  sizeof(converterFormat), &converterFormat);
    if (err)
    {
        goto reterr;
    }
    
*/


    if( err != noErr ) PBAFileStreamClose((inputAudioFileRef->file));

    //playerState = state;

    //fprintf(stdout, "open file for read complete");
    
    //cleanup
    //CFRelease(fileURLRef);
    //CFRelease(fileURLStringRef);

    CFRelease(bundleResourcesURL2);

    //CFRelease(bundleResourcesURL);
    //CFRelease(bundleResourcesPath);
    //CFRelease(filePath);
    //CFRelease(cfFileExtString);
    //CFRelease(cfFileString);
    
    CFRelease(fileStringRef);
#else
    if (strcmp(fileExt, "aif") == 0 || strcmp(fileExt, "AIF") == 0   ||    //load aiff
        strcmp(fileExt, "aiff") == 0 || strcmp(fileExt, "AIFF") == 0 ||    //load aiff
        strcmp(fileExt, "aifc") == 0 || strcmp(fileExt, "AIFC") == 0)      //load aiff
    {
        //printf("\nLoading PNG:    %s\n", filepath);
        xnz_aif_open(&inputAudioFileRef->aif, fileURL);

        //record source format
        //memcpy(&inputAudioFileRef->sourceFormat, ((char*)inputAudioFileRef->aif.fmt) + sizeof(xnz_wav_chunk), inputAudioFileRef->aif.fmt->chunkSize);
        
        
        double sampleRate = xnz_read_float80(inputAudioFileRef->aif.comm.sampleRate);// xnz_be32toh(*(uint32_t*)inputAudioFileRef->aif.comm.sampleRate);

        inputAudioFileRef->sourceFormat.mFormatID          = kAudioFormatLinearPCM;//inputAudioFileRef->aif.comm.nChannels > 2 ? WAVE_FORMAT_EXTENSIBLE : WAVE_FORMAT_PCM;
        inputAudioFileRef->sourceFormat.mChannelsPerFrame  = inputAudioFileRef->aif.comm.nChannels;
        inputAudioFileRef->sourceFormat.mSampleRate        = (Float64)sampleRate;
        inputAudioFileRef->sourceFormat.mBytesPerFrame     = inputAudioFileRef->aif.comm.nChannels * (inputAudioFileRef->aif.comm.sampleSize / 8);
        inputAudioFileRef->sourceFormat.mBitsPerChannel    = inputAudioFileRef->aif.comm.sampleSize;

        //inputAudioFileRef->sourceFormat.nAvgBytesPerSec = inputAudioFileRef->sourceFormat.nSamplesPerSec * (inputAudioFileRef->aif.comm.sampleSize);// *inputAudioFileRef->sourceFormat.nChannels;

        //copy source format to conversion format
        memcpy(&inputAudioFileRef->conversionFormat, &inputAudioFileRef->sourceFormat, sizeof(PBAStreamFormat));
        
        inputAudioFileRef->type = PBAStreamFormatGetType(&inputAudioFileRef->sourceFormat); //enumerate a sample packing protocol for the given format

        //calculate frame count
        inputAudioFileRef->numFrames = inputAudioFileRef->aif.comm.nSampleFrames;// / inputAudioFileRef->aif.ssnd->nBlockAlign;

        //point at samples buffer
        //inputAudioFileRef->samples[0] = inputAudioFileRef->aif.samples;

        inputAudioFileRef->form = XNG_AUDIO_FORM_AIF;

    }
    else if (strcmp(fileExt, "wav") == 0 || strcmp(fileExt, "WAV") == 0)    //load aiff
    {
        xnz_wav_open(&inputAudioFileRef->wav, fileURL);

        
        //record source format
        //memcpy(&inputAudioFileRef->sourceFormat, ((char*)inputAudioFileRef->wav.fmt) + sizeof(xnz_wav_chunk), inputAudioFileRef->wav.fmt->chunkSize);
        
        inputAudioFileRef->sourceFormat.mFormatID          = kAudioFormatLinearPCM;//inputAudioFileRef->aif.comm.nChannels > 2 ? WAVE_FORMAT_EXTENSIBLE : WAVE_FORMAT_PCM;
        inputAudioFileRef->sourceFormat.mChannelsPerFrame  = inputAudioFileRef->wav.fmt->nChannels;
        inputAudioFileRef->sourceFormat.mSampleRate        = (Float64)inputAudioFileRef->wav.fmt->nSamplesPerSec;
        inputAudioFileRef->sourceFormat.mBytesPerFrame     = inputAudioFileRef->wav.fmt->nChannels * (inputAudioFileRef->wav.fmt->wBitsPerSample / 8);
        inputAudioFileRef->sourceFormat.mBitsPerChannel    = inputAudioFileRef->wav.fmt->wBitsPerSample;
        
        //copy source format to conversion format
        memcpy(&inputAudioFileRef->conversionFormat, &inputAudioFileRef->sourceFormat, sizeof(PBAStreamFormat));

        inputAudioFileRef->type = PBAStreamFormatGetType(&inputAudioFileRef->sourceFormat); //enumerate a sample packing protocol for the given format

        //calculate frame count
        inputAudioFileRef->numFrames = inputAudioFileRef->wav.data->chunkSize / inputAudioFileRef->wav.fmt->nBlockAlign;

        //point at samples buffer
        //inputAudioFileRef->samples[0] = inputAudioFileRef->wav.samples;

        inputAudioFileRef->form = XNG_AUDIO_FORM_WAV;
        
    }
#endif
    
    return err;
}



PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamReadFrames(PBAFileRef audioFileRef, unsigned long long numFramesToRead, void ** sampleBuffers)
{

        OSStatus err = noErr;

#ifdef XNZ_AUDIO
    //if (sampleBuffers[0] != audioFileRef->samples[0])
    //{
        //TO DO:...
        //assert(1 == 0);
    
        switch( audioFileRef->form )
        {
            case (XNG_AUDIO_FORM_AIF):
                xnz_aif_read_samples(&audioFileRef->aif, numFramesToRead, sampleBuffers); break;

            case (XNG_AUDIO_FORM_WAV):
                xnz_wav_read_samples(&audioFileRef->wav, numFramesToRead, sampleBuffers); break;

            default:
                assert(1 == 0);
        }
        
    //}
#else
        int i = 0;

        // BufferList to read from source file and write to dest file
        AudioBufferList conversionBuffer; conversionBuffer.mNumberBuffers = audioFileRef->conversionFormat.mChannelsPerFrame;
        
        // A single AudioBuffer for each L+R Channel, Respectively
        for( i = 0; i< conversionBuffer.mNumberBuffers; i++)
        {
            conversionBuffer.mBuffers[i].mNumberChannels = 1;//audioFileRef->conversionFormat.mChannelsPerFrame;
            conversionBuffer.mBuffers[i].mData = sampleBuffers[i];//buffers[i];
            conversionBuffer.mBuffers[i].mDataByteSize = (uint32_t)numFramesToRead * sizeof(float);
            //bzero(conversionBuffer.mBuffers[i].mData, conversionBuffer.mBuffers[i].mDataByteSize);
        }
        
        //frameCount = (conversionBuffer.mBuffers[0].mDataByteSize / inputFileFormat.mBytesPerFrame);
        UInt32 extAudioFrames = (uint32_t)numFramesToRead;
        
        fprintf(stdout, "\nsource audio file length of file in frames:   %llu", numFramesToRead);
        fprintf(stdout, "\next audio frames requested:   %u", extAudioFrames);
        //if( availableSamples < extAudioFrames ) extAudioFrames = availableSamples;

        // Read a chunk of input as bytes
        err = ExtAudioFileRead(audioFileRef->file, &extAudioFrames, &conversionBuffer);
        
        if (err) fprintf(stderr, "PBAFileStreamReadSamples::Error (%d) reading from InputAudioFileRef.", err);
        else     fprintf(stdout, "ExtAudioFileRead returned %d samples.", (int)extAudioFrames);
        
        // If no frames were returned, conversion is finished
        if (extAudioFrames == 0)
        {
            //End of File
            //break;
        }
        
        // Write pcm data to output file
        //err = ExtAudioFileWrite(outputAudioFileRef, frameCount, &conversionBuffer);
            
        if (err)
        {
        
        }
#endif
        
        return err;//1(unsigned long long)extAudioFrames;
}

#else

IMFAttributes* sourceReaderConfiguration;// Windows Media Foundation Source Reader Configuration

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamOpen(const char* fileURL, const char* fileExt, PBAStreamFormatRef converterFormat, PBAFileRef inputAudioFileRef)//PBAStreamContext * streamContext, PBAStreamLatencyReport * report, double renderTime, double bufferDuration);
{
#ifndef XNZ_AUDIO
    OSStatus hr;
    UINT32 formatLength = 0;
    PBAStreamFormat* fileFormat = NULL;

    //TO DO:  convert char* path to LPCWSTR

    //Media Foundation Initialize
    hr = MFStartup(MF_VERSION, 0UL);
    if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to start the Windows Media Foundation!\n"); assert(1 == 0); }

    // set media foundation reader to low latency
    hr = MFCreateAttributes(&sourceReaderConfiguration, 1);
    if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to create Media Foundation Source Reader configuration!\n"); assert(1 == 0); }

    hr = CALL(SetUINT32, sourceReaderConfiguration, &_IID_MF_LOW_LATENCY, true);
    if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to set Windows Media Foundation configuration!\n"); assert(1 == 0); }

    //Use Media Foundation to load wav file e.g. LPCWSTR fileName = L"../../assets/Audio/WAV/Test/16_48k_PerfectTest.wav";    
    
    wchar_t pathURL[512]; int pathLen = MultiByteToWideChar(CP_UTF8, 0, fileURL, -1, NULL, 0);
    MultiByteToWideChar(CP_UTF8, 0, fileURL, -1, pathURL, pathLen); 
    
    //Source Reader Initialize

    // stream index indicates which data to decode from source file
    DWORD streamIndex = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM; 

    // create the source reader
    hr = MFCreateSourceReaderFromURL(pathURL, sourceReaderConfiguration, &inputAudioFileRef->file);
    if (FAILED(hr)) { wprintf(L"Critical error: Unable to create source reader from URL (%s) !\n", pathURL); assert(1==0); }

    // select the first audio stream, and deselect all other streams
    hr = CALL(SetStreamSelection, inputAudioFileRef->file, (DWORD)MF_SOURCE_READER_ALL_STREAMS, false);
    if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to disable streams!\n"); assert(1==0); }

    hr = CALL(SetStreamSelection, inputAudioFileRef->file, streamIndex, true);
    if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to enable first audio stream!\n"); assert(1 == 0); }


    //Media File

    // query information about the media file
	IMFMediaType* nativeMediaType;
	hr = CALL(GetNativeMediaType, inputAudioFileRef->file, streamIndex, 0, &nativeMediaType);
	if(FAILED(hr)) { fprintf(stderr, "Critical error: Unable to query media information!\n"); assert(1==0); }

	// make sure that this is really an audio file
	GUID majorType = {0};
	hr = CALL(GetGUID, nativeMediaType, &MF_MT_MAJOR_TYPE, &majorType);
	if ( memcmp(&majorType, &MFMediaType_Audio, sizeof(GUID)) != 0 ) { fprintf(stderr, "Critical error: the requested file is not an audio file!"); assert(1==0); }

	// check whether the audio file is compressed or uncompressed
	GUID subType = {0};
	hr = CALL(GetGUID, nativeMediaType, &MF_MT_MAJOR_TYPE, &subType);
	if ( (memcmp(&subType, &MFAudioFormat_Float, sizeof(GUID)) == 0) || (memcmp(&subType, &MFAudioFormat_PCM, sizeof(GUID)) == 0) )
	{
        //LPCM(e.g. not compressed)
	}
	else
	{
		// the audio file is compressed; we have to decompress it first
		IMFMediaType* partialType = NULL;
		hr = MFCreateMediaType(&partialType);
		if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable create media type!"); assert(1==0); }

		// set the media type to "audio"
		hr = CALL(SetGUID, partialType, &MF_MT_MAJOR_TYPE, &MFMediaType_Audio);
		if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to set media type to audio!"); assert(1==0); }
    
		// request uncompressed data
		hr = CALL(SetGUID, partialType, &MF_MT_SUBTYPE, &MFAudioFormat_PCM);
		if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to set guid of media type to uncompressed!"); assert(1==0); }

		//submit request
		hr = CALL(SetCurrentMediaType, inputAudioFileRef->file, streamIndex, NULL, partialType);
		if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to set current media type!"); assert(1==0); }

        // Clean up COM memory
        if (partialType) { CALL(Release, partialType); partialType = NULL; }
	}

	// uncompress the data and load it into an XAudio2 Buffer
	IMFMediaType* uncompressedAudioType = NULL;
	hr = CALL(GetCurrentMediaType, inputAudioFileRef->file, streamIndex, &uncompressedAudioType);
	if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to retrieve the current media type!"); assert(1==0); }

    // get the source format
	hr = MFCreateWaveFormatExFromMFMediaType(uncompressedAudioType, &fileFormat, &formatLength, 0);
	if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to create the wave format!"); assert(1==0); }

    //record source format
    inputAudioFileRef->sourceFormat = *fileFormat;
    inputAudioFileRef->type = PBAStreamFormatGetType(fileFormat); //enumerate a sample packing protocol for the given format

	// ensure the stream is selected
	hr = CALL(SetStreamSelection, inputAudioFileRef->file, streamIndex, true);
	if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to select audio stream!"); assert(1==0); }

    //Get file duration in seconds
    //The function shown here gets the duration in 100 - nanosecond units.Divide by 10,000,000 to get the duration in seconds.

    //HRESULT GetDuration(IMFSourceReader * pReader, LONGLONG * phnsDuration)
    //{
        LONGLONG phnsDuration = 0; PROPVARIANT var = {0};
        hr = CALL(GetPresentationAttribute, inputAudioFileRef->file, MF_SOURCE_READER_MEDIASOURCE, &MF_PD_DURATION, &var); if (FAILED(hr)) assert(1 == 0);
        hr = PropVariantToInt64(&var, &phnsDuration);  if (FAILED(hr)) assert(1 == 0);
        PropVariantClear(&var);
    //}

    //record source frame count
    double dFrames = ((double)phnsDuration / 10000000. * (double)fileFormat->nSamplesPerSec);
    
    double integral;
    double frac = modf(dFrames, &integral);

    inputAudioFileRef->numFrames = (uint64_t)integral + (frac > 0.);

    //record input conversion format
    memcpy(&(inputAudioFileRef->conversionFormat), converterFormat, sizeof(PBAStreamFormat));
    
    // Clean up MFCreateWaveFormat... memory
    // Clean up source rreader configuratio memory
    if (fileFormat)                { CoTaskMemFree(fileFormat); fileFormat = NULL;                               }
    if (sourceReaderConfiguration) { CALL(Release, sourceReaderConfiguration); sourceReaderConfiguration = NULL; }
#else
    
    if (strcmp(fileExt, "aif") == 0 || strcmp(fileExt, "AIF") == 0   ||    //load aiff
        strcmp(fileExt, "aiff") == 0 || strcmp(fileExt, "AIFF") == 0 ||    //load aiff
        strcmp(fileExt, "aifc") == 0 || strcmp(fileExt, "AIFC") == 0)      //load aiff
    {
        //printf("\nLoading PNG:    %s\n", filepath);
        xnz_aif_open(&inputAudioFileRef->aif, fileURL);

        //record source format
        //memcpy(&inputAudioFileRef->sourceFormat, ((char*)inputAudioFileRef->aif.fmt) + sizeof(xnz_wav_chunk), inputAudioFileRef->aif.fmt->chunkSize);
        
        double sampleRate = xnz_read_float80(inputAudioFileRef->aif.comm.sampleRate);// xnz_be32toh(*(uint32_t*)inputAudioFileRef->aif.comm.sampleRate);

        inputAudioFileRef->sourceFormat.wFormatTag      = inputAudioFileRef->aif.comm.nChannels > 2 ? WAVE_FORMAT_EXTENSIBLE : WAVE_FORMAT_PCM;
        inputAudioFileRef->sourceFormat.nChannels       = inputAudioFileRef->aif.comm.nChannels;
        inputAudioFileRef->sourceFormat.nSamplesPerSec  = (DWORD)sampleRate;
        inputAudioFileRef->sourceFormat.nBlockAlign     = inputAudioFileRef->aif.comm.nChannels * (inputAudioFileRef->aif.comm.sampleSize / 8);
        inputAudioFileRef->sourceFormat.wBitsPerSample  = inputAudioFileRef->aif.comm.sampleSize;

        inputAudioFileRef->sourceFormat.nAvgBytesPerSec = inputAudioFileRef->sourceFormat.nSamplesPerSec * (inputAudioFileRef->aif.comm.sampleSize);// *inputAudioFileRef->sourceFormat.nChannels;

        inputAudioFileRef->type = PBAStreamFormatGetType(&inputAudioFileRef->sourceFormat); //enumerate a sample packing protocol for the given format

        //calculate frame count
        inputAudioFileRef->numFrames = inputAudioFileRef->aif.comm.nSampleFrames;// / inputAudioFileRef->aif.ssnd->nBlockAlign;

        //point at samples buffer
        //inputAudioFileRef->samples[0] = inputAudioFileRef->aif.samples;

        inputAudioFileRef->form = XNG_AUDIO_FORM_AIF;

    }
    else if (strcmp(fileExt, "wav") == 0 || strcmp(fileExt, "WAV") == 0)    //load aiff
    {
        xnz_wav_open(&inputAudioFileRef->wav, fileURL);

        //record source format
        memcpy(&inputAudioFileRef->sourceFormat, ((char*)inputAudioFileRef->wav.fmt) + sizeof(xnz_wav_chunk), inputAudioFileRef->wav.fmt->chunkSize);
        inputAudioFileRef->type = PBAStreamFormatGetType(&inputAudioFileRef->sourceFormat); //enumerate a sample packing protocol for the given format

        //calculate frame count
        inputAudioFileRef->numFrames = inputAudioFileRef->wav.data->chunkSize / inputAudioFileRef->wav.fmt->nBlockAlign;

        //point at samples buffer
        //inputAudioFileRef->samples[0] = inputAudioFileRef->wav.samples;

        inputAudioFileRef->form = XNG_AUDIO_FORM_WAV;
    }

#endif

}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamReadFrames(PBAFileRef audioFileRef, unsigned long long numFramesToRead, void** sampleBuffers)
{
    OSStatus hr = 0;

#ifdef XNZ_AUDIO
    //if (sampleBuffers[0] != audioFileRef->samples[0])
    //{
        //TO DO:...
        //assert(1 == 0);
    
        switch( audioFileRef->form )
        {
            case (XNG_AUDIO_FORM_AIF):
                xnz_aif_read_samples(&audioFileRef->aif, numFramesToRead, sampleBuffers); break;

            case (XNG_AUDIO_FORM_WAV):
                xnz_wav_read_samples(&audioFileRef->aif, numFramesToRead, sampleBuffers); break;

            default:
                assert(1 == 0);
        }
        
    //}
#else
    //Ethan Hunt decodes the NOC List
    IMFSample*      sample               = NULL;
    IMFMediaBuffer* buffer               = NULL;

    //Then he copies it to memory
    BYTE*           localAudioData       = NULL;
    DWORD           localAudioDataLength = 0;

    uint64_t        bytesRead            = 0;
    DWORD           streamIndex          = (DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM;

    while (true)
    {
        DWORD flags = 0;
        hr = CALL(ReadSample, audioFileRef->file, streamIndex, 0, NULL, &flags, NULL, &sample);
        if (FAILED(hr)) { printf("Critical error: Unable to read audio sample!"); return; }

        // check whether the data is still valid
        if (flags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED) break;
        // check for eof
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) break;
        // source reader need to pumped again
        if (sample == NULL) continue;

        //Convert a sample with multiple buffers into a sample with a single buffer:
        // 
        // If the sample contains more than one buffer, this method copies the data from the original buffers into a new buffer, 
        // and replaces the original buffer list with the new buffer. The new buffer is returned in the ppBuffer parameter.
        // If the sample contains a single buffer, this method returns a pointer to the original buffer.
        // In typical use, most samples do not contain multiple buffers.
        hr = CALL(ConvertToContiguousBuffer, sample, &buffer); if (FAILED(hr)) { fprintf(stderr, "Critical error: Unable to convert audio sample to contiguous buffer!"); assert(1==0); }

        // lock buffer
        hr = CALL(Lock, buffer, &localAudioData, NULL, &localAudioDataLength); if (FAILED(hr)) { printf("Critical error: Unable to lock the audio buffer!"); return; }

        char* dstBuffer = (char*)sampleBuffers[0]; //audioFileRef->samples[0];
        //copy data to local memory; stereo samples will be interleaved unless unpacked here
        memcpy(&dstBuffer[bytesRead], localAudioData, localAudioDataLength);

        //sum running length of uncompressed source sample buffer in bytes
        bytesRead += localAudioDataLength;

        // unlock buffer
        hr = CALL(Unlock, buffer); localAudioData = NULL; if (FAILED(hr)) { printf("Critical error while unlocking the audio buffer!"); return; }

    }

    //Calculate num frames based on the size of a sample and frame container
    uint64_t framesRead = bytesRead / (audioFileRef->sourceFormat.wBitsPerSample / 8) / audioFileRef->sourceFormat.nChannels;
    assert(framesRead == audioFileRef->numFrames);
#endif

    return hr; //audioFileRef->numFrames;
}

#endif


//Example Asset Bundle Path:
//Users/jmoulton/Library/Developer/Xcode/DerivedData/[Pb]Audio-acgignvyyvqdjjdamniexffodiua/Build/Products/Debug/[Pb] Audio.app/Contents/Resources/Assets/DecadesMix.aif
