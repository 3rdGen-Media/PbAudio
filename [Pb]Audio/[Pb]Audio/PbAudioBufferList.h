//
//  pbaudio_buffer.h
//  CRViewer
//
//  Created by Joe Moulton on 6/27/20.
//  Copyright © 2020 Abstract Embedded. All rights reserved.
//

#ifndef pbaudio_bufferlist_h
#define pbaudio_bufferlist_h

typedef AudioBufferList PBABufferList;

//Create AudioBufferList
PB_AUDIO_API PB_AUDIO_INLINE PBABufferList *PBABufferListCreateWithFormat(PBAStreamFormat audioFormat, int frameCount);

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

PB_AUDIO_API PB_AUDIO_INLINE PBABufferList *PBABufferListCreate(int frameCount);



//Schedule to Audio Buffer List


PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilenceWithFormat(const AudioBufferList *bufferList, AudioStreamBasicDescription audioFormat, UInt32 offset, UInt32 length);

//List silence to default engine format
PB_AUDIO_API PB_AUDIO_INLINE void PBABufferListSilence(const AudioBufferList *bufferList, UInt32 offset, UInt32 length);


#endif /* pbaudio_buffer_h */
