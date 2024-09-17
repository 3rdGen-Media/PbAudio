//
//  PbAudioFileStream.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 11/6/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"


/*
#if defined( CR_TARGET_OSX)
    char vertexShaderPath[1024] = "/Users/jmoulton/Development/svn/CoreRender/crShaders/crShaders/glsl/cr_mesh_vbo.es2.vert.glsl";
    char fragmentShaderPath[1024] = "/Users/jmoulton/Development/svn/CoreRender/crShaders/crShaders/glsl/cr_mesh_vbo.es2.frag.glsl";
    char gbufferVertexShaderPath[1024] = "/Users/jmoulton/Development/svn/CoreRender/crShaders/crShaders/glsl/cr_gbuffer.es2.vert.glsl";
    char gbufferFragmentShaderPath[1024] =  "/Users/jmoulton/Development/svn/CoreRender/crShaders/crShaders/glsl/cr_gbuffer.es2.frag.glsl";
    const char * terrainPath = "/Users/jmoulton/Development/svn/CoreRender/assets/models/Animate/Bipedal/RedPanda/RedPandaRigged.abc";
    const char * cratePath = "/Users/jmoulton/Development/svn/CoreRender/assets/models/Inanimate/Crates/TexturedCratePBR/TexturedCrate.abc";
    char * animationPath = "/Users/jmoulton/Development/svn/CoreRender/assets/animations/idle.animation";
#elif defined( CR_TARGET_WIN32 )
    char vertexShaderPath[1024] = "C:\\Development\\svn\\CoreRender\\crShaders\\crShaders\\glsl\\cr_mesh_vbo.es2.vert.glsl";
    char fragmentShaderPath[1024] = "C:\\Development\\svn\\CoreRender\\crShaders\\crShaders\\glsl\\cr_mesh_vbo.es2.frag.glsl";
    char gbufferVertexShaderPath[1024] = "C:\\Development\\svn\\CoreRender\\crShaders\\crShaders\\glsl\\cr_gbuffer.es2.vert.glsl";
    char gbufferFragmentShaderPath[1024] =  "C:\\Development\\svn\\CoreRender\\crShaders\\crShaders\\glsl\\cr_gbuffer.es2.frag.glsl";
    const char * terrainPath = "C:\\Development\\svn\\CoreRender\\assets\\models\\Animate\\Bipedal\\RedPanda\\RedPandaRigged.abc";
    char * animationPath = "C:\\Development\\svn\\CoreRender\\assets\\assets\\animations\\idle.animation";
#endif
    
    //Build path to 3d geometry file on disk
#if defined( CR_TARGET_IOS) || defined(CR_TARGET_TVOS)
    //char * terrainPath = cr_file_path_in_documents_dir("/MK2Grenade_OBJ/MetalGrenade.obj");
#else

#ifdef _WIN32
    const char * cratePath = "C:\\Development\\svn\\CoreRender\\assets\\models\\Inanimate\\Crates\\TexturedCratePBR\\TexturedCrate.abc";
#else
    //char * terrainPath = "/Users/jmoulton/Desktop/MK2Grenade_OBJ/MetalGrenade2.obj\0";
    //char * terrainPath = "/Users/jmoulton/Desktop/Backup/terrain/UrbanBlock/urbanBlock.obj\0";
    //const char * terrainPath = "/Users/jmoulton/Development/svn/CoreRender/assets/models/Inanimate/Shapes/UnitCube/UnitCube.abc";

#endif

#endif
*/



PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamClose(ExtAudioFileRef inputAudioFileRef)
{
    OSStatus err = noErr;

    if (inputAudioFileRef)
    {
        ExtAudioFileDispose(inputAudioFileRef);

        if( err )
        {
            printf("PBAFileStreamClose Error disposing of extended audio file context");
        
        }

    }
    inputAudioFileRef = NULL;
    
    //if (outputAudioFileRef)
    //    ExtAudioFileDispose(outputAudioFileRef);
    
    return err;
}

PB_AUDIO_API PB_AUDIO_INLINE OSStatus PBAFileStreamOpen(const char * fileURL, const char * fileExt, PBAStreamFormat converterFormat, PBAFileRef inputAudioFileRef)//PBAStreamContext * streamContext, PBAStreamLatencyReport * report, double renderTime, double bufferDuration);
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
    OSStatus                                err = noErr;
    //AudioStreamBasicDescription             converterFormat;
    UInt32                                  thePropertySize = sizeof(PBAStreamFormat);
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
    // output file by setting up a channel map. The mono output is duplicated
    // in the left and right channel.
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

    return err;
}


PB_AUDIO_API PB_AUDIO_INLINE unsigned long long PBAFileStreamReadFrames(PBAFileRef audioFileRef, unsigned long long numFramesToRead, void ** sampleBuffers)
{

        int i = 0;
        OSStatus err = noErr;

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
        
        return (unsigned long long)extAudioFrames;
}

//Users/jmoulton/Library/Developer/Xcode/DerivedData/[Pb]Audio-acgignvyyvqdjjdamniexffodiua/Build/Products/Debug/[Pb] Audio.app/Contents/Resources/Assets/DecadesMix.aif
//Users/jmoulton/Library/Developer/Xcode/DerivedData/[Pb]Audio-acgignvyyvqdjjdamniexffodiua/Build/Products/Debug/[Pb] Audio.app/Contents/Resources/Assets/DecadesMix.aif
