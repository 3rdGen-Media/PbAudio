//
//  pbaudio_buffer.h
//  CRViewer
//
//  Created by Joe Moulton on 6/27/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_bufferlist_h
#define pbaudio_bufferlist_h

#include <stdint.h>

#ifdef __APPLE__
typedef AudioBuffer		PBABuffer;
typedef AudioBufferList PBABufferList;
#else

//emulate OSX AudioBufferList on Win32
typedef struct PBABuffer
{
	uint32_t		mNumberChannels;
	uint32_t		mDataByteSize;
	void*			mData;
}PBABuffer;

typedef struct PBABufferList
{
	uint32_t	mNumberBuffers;
	PBABuffer	*mBuffers;//[1]; //this is a variable lenght array of mNumberBuffers Elements
}PBABufferList;
#endif

//Create AudioBufferList
#ifdef __APPLE__
PB_AUDIO_API PB_AUDIO_INLINE PBABufferList *PBABufferListCreateWithFormat(PBAStreamFormat audioFormat, int frameCount);
PB_AUDIO_API PB_AUDIO_INLINE PBABufferList *PBABufferListCreate(int frameCount);

/*
AudioBufferList *AEAudioBufferListCreateWithContentsOfFile(NSString * filePath, AudioStreamBasicDescription audioFormat) {
    AudioStreamBasicDescription fileFormat;
    UInt64 length;
    NSError * error = nil;
    ExtAudioFileRef audioFile = AEExtAudioFileOpen([NSURL fileURLWithPath:filePath], &fileFormat, &length, &error);
    
    if ( !audioFile ) {
        NSLog(@"Unable to open %@ for reading: %@", filePath, error.localizedDescription);
        return NULL;
    }
    
    if ( !audioFormat.mSampleRate ) audioFormat.mSampleRate = fileFormat.mSampleRate;
    
    if ( !AECheckOSStatus(ExtAudioFileSetProperty(audioFile, kExtAudioFileProperty_ClientDataFormat, sizeof(audioFormat), &audioFormat),
                          "ExtAudioFileSetProperty") ) {
        ExtAudioFileDispose(audioFile);
        return NULL;
    }
    
    UInt32 lengthAtTargetRate = (UInt32)floor(((double)length / fileFormat.mSampleRate) * audioFormat.mSampleRate);
    AudioBufferList * output = AEAudioBufferListCreateWithFormat(audioFormat, lengthAtTargetRate);
    
    UInt32 blockSize = 4096;
    UInt32 remaining = lengthAtTargetRate;
    UInt32 readFrames = 0;
    while ( remaining > 0 ) {
        UInt32 block = MIN(blockSize, remaining);
        AEAudioBufferListCopyOnStackWithByteOffset(target, output, (readFrames * audioFormat.mBytesPerFrame));
        AEAudioBufferListSetLength(target, block);
        if ( !AECheckOSStatus(ExtAudioFileRead(audioFile, &block, target), "ExtAudioFileRead") || block == 0 ) {
            break;
        }
        readFrames += block;
        remaining -= block;
    }
    
    ExtAudioFileDispose(audioFile);
    
    AEAudioBufferListSetLength(output, readFrames);
    return output;
}
*/


#endif

//Schedule to Audio Buffer List


PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilenceWithFormat(const PBABufferList *bufferList, PBAStreamFormat audioFormat, uint32_t offset, uint32_t length);

//List silence to default engine format
PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilence(const PBABufferList *bufferList, uint32_t offset, uint32_t length);


#endif /* pbaudio_buffer_h */
