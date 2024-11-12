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
PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListFree(PBABufferList *bufferList);


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

/*!
 * Create an audio buffer list on the stack, using the default audio format.
 *
 *  This is useful for creating buffers for temporary use, without needing to perform any
 *  memory allocations. It will create a local AudioBufferList* variable on the stack, with
 *  a name given by the first argument, and initialise the buffer according to the given
 *  audio format.
 *
 *  The created buffer will have NULL mData pointers and 0 mDataByteSize: you will need to
 *  assign these to point to a memory buffer.
 *
 * @param name Name of the variable to create on the stack
 */
#define PBABufferListCreateOnStack(name) \
    AEAudioBufferListCreateOnStackWithFormat(name, AEAudioDescription)

/*!
 * Create an audio buffer list on the stack, with a custom audio format.
 *
 *  This is useful for creating buffers for temporary use, without needing to perform any
 *  memory allocations. It will create a local AudioBufferList* variable on the stack, with
 *  a name given by the first argument, and initialise the buffer according to the given
 *  audio format.
 *
 *  The created buffer will have NULL mData pointers and 0 mDataByteSize: you will need to
 *  assign these to point to a memory buffer.
 *
 * @param name Name of the variable to create on the stack
 * @param audioFormat The audio format to use
 */
#define PBABufferListCreateOnStackWithFormat(name, audioFormat, channels) \
    int name ## _numberBuffers = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved \
                                    ? channels : 1; \
    char name ## _bytes[sizeof(AudioBufferList)+(sizeof(AudioBuffer)*(name ## _numberBuffers-1))]; \
    memset(&name ## _bytes, 0, sizeof(name ## _bytes)); \
    AudioBufferList * name = (AudioBufferList*)name ## _bytes; \
    name->mNumberBuffers = name ## _numberBuffers; \
    for ( int i=0; i<name->mNumberBuffers; i++ ) { \
        name->mBuffers[i].mNumberChannels \
            = audioFormat.mFormatFlags & kAudioFormatFlagIsNonInterleaved ? 1 : channels; \
    }

/*!
 * Create a stack copy of the given audio buffer list and offset mData pointers
 *
 *  This is useful for creating buffers that point to an offset into the original buffer,
 *  to fill later regions of the buffer. It will create a local AudioBufferList* variable
 *  on the stack, with a name given by the first argument, copy the original AudioBufferList
 *  structure values, and offset the mData and mDataByteSize variables.
 *
 *  Note that only the AudioBufferList structure itself will be copied, not the data to
 *  which it points.
 *
 * @param name Name of the variable to create on the stack
 * @param sourceBufferList The original buffer list to copy
 * @param offsetFrames Number of frames of noninterleaved float to offset mData/mDataByteSize members
 */
#define PBABufferListCopyOnStack(name, sourceBufferList, offsetFrames) \
    PBABufferListCopyOnStackWithByteOffset(name, sourceBufferList, offsetFrames * AEAudioDescription.mBytesPerFrame)

/*!
 * Create a stack copy of the given audio buffer list and offset mData pointers, with offset in bytes
 *
 *  This is useful for creating buffers that point to an offset into the original buffer,
 *  to fill later regions of the buffer. It will create a local AudioBufferList* variable
 *  on the stack, with a name given by the first argument, copy the original AudioBufferList
 *  structure values, and offset the mData and mDataByteSize variables.
 *
 *  Note that only the AudioBufferList structure itself will be copied, not the data to
 *  which it points.
 *
 * @param name Name of the variable to create on the stack
 * @param sourceBufferList The original buffer list to copy
 * @param offsetBytes Number of bytes to offset mData/mDataByteSize members
 */
#define PBABufferListCopyOnStackWithByteOffset(name, sourceBufferList, offsetBytes) \
    const AudioBufferList * name ## _sourceBuffer = (sourceBufferList); \
    const UInt32 name ## _offsetBytes = (UInt32)(offsetBytes); \
    char name ## _bytes[sizeof(AudioBufferList)+(sizeof(AudioBuffer)*(name ## _sourceBuffer->mNumberBuffers-1))]; \
    memcpy(name ## _bytes, name ## _sourceBuffer, sizeof(name ## _bytes)); \
    AudioBufferList * name = (AudioBufferList*)name ## _bytes; \
    for ( int i=0; i<name->mNumberBuffers; i++ ) { \
        name->mBuffers[i].mData = (char*)name->mBuffers[i].mData + name ## _offsetBytes; \
        name->mBuffers[i].mDataByteSize -= name ## _offsetBytes; \
    }

/*!
 * Create a stack copy of an audio buffer list that points to a subset of its channels
 *
 * @param name Name of the variable to create on the stack
 * @param sourceBufferList The original buffer list to copy
 * @param channelSet The subset of channels
 */
#define PBABufferListCopyOnStackWithChannelSubset(name, sourceBufferList, channelSet) \
    int name ## _bufferCount = MIN(sourceBufferList->mNumberBuffers-1, channelSet.lastChannel) - \
                               MIN(sourceBufferList->mNumberBuffers-1, channelSet.firstChannel) + 1; \
    char name ## _bytes[sizeof(AudioBufferList)+(sizeof(AudioBuffer)*(name ## _bufferCount-1))]; \
    AudioBufferList * name = (AudioBufferList*)name ## _bytes; \
    name->mNumberBuffers = name ## _bufferCount; \
    memcpy(name->mBuffers, &sourceBufferList->mBuffers[MIN(sourceBufferList->mNumberBuffers-1, channelSet.firstChannel)], \
        sizeof(AudioBuffer) * name ## _bufferCount);

/*!
 * Create a copy of an audio buffer list
 *
 *  Note: Do not use this utility from within the Core Audio thread (such as inside a render
 *  callback). It may cause the thread to block, inducing audio stutters.
 *
 * @param original The original AudioBufferList to copy
 * @return The new, copied audio buffer list
 */
//AudioBufferList *AEAudioBufferListCopy(const AudioBufferList *original);

#endif

//Schedule to Audio Buffer List


PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilenceWithFormat(const PBABufferList *bufferList, PBAStreamFormat* audioFormat, uint32_t offset, uint32_t length);

//List silence to default engine format
//PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilence(const PBABufferList *bufferList, uint32_t offset, uint32_t length);


#endif /* pbaudio_buffer_h */
