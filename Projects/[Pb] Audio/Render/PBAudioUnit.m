/*
 Copyright (C) 2016 Apple Inc. All Rights Reserved.
 See LICENSE.txt for this sample’s licensing information
 
 Abstract:
 An AUAudioUnit subclass implementing a low-pass filter with resonance. Illustrates parameter management and rendering, including in-place processing and buffer management.
 */

#import "PBAudioUnit.h"
#import <AVFoundation/AVFoundation.h>

//[Pb]Audio + CMidi
#include "PbAudioAppInterface.h"

//[Pb]Audio Renderpass(es)
#include "ToneGenerator.h"
#include "SamplePlayer.h"

#pragma mark -- [Pb]AudioUnit Stream Renderpass(es)

SamplePlayer  AUSamplePlayer  = {0};
ToneGenerator AUToneGenerator = {0};

#pragma mark -- [Pb]AudioUnit Stream OutputPass Callbacks

PBAStreamOutputPass PBAudioUnitTestOutputPass = ^(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp, struct PBAStreamContext* stream)
{
    ToneGeneratorRenderPass(ioData, frames, timestamp, stream->target, &AUToneGenerator, NULL, 0);
};

PBAStreamOutputPass PBAudioUnitOutputPass = ^(AudioBufferList * _Nonnull ioData, UInt32 frames, const AudioTimeStamp * _Nonnull timestamp, struct PBAStreamContext* stream)
{
    pba_platform_event_msg msg = {0}; //this is like kev

    //local trigger event iterator/cache
    int nTriggerEvents = 0;
    CMTriggerMessage* triggerEvent = NULL;
    CMTriggerMessage  triggerEvents[MAX_TRIGGER_EVENTS];
    
    //Thread Queue Message Processing

    struct kevent kev[MAX_TRIGGER_EVENTS];
        
    //wait without timeout for trigger input kevent from our kqueue
    struct timespec _ts; struct timespec *ts = NULL;
    ts = &_ts; ts->tv_sec = 0; ts->tv_nsec = 0;
    
    //read axis control events + button control events from the control queue
    nTriggerEvents = kevent(CMTriggerEventQueue, NULL, 0, &kev[0], MAX_TRIGGER_EVENTS, ts);
    //while (nTriggerEvents > 0)
    {
        int i; for(i=0;i<nTriggerEvents;i++)
        {
            if (kev[i].filter == EVFILT_USER)
            {
                //cache trigger event values we just pulled from the queue
                triggerEvent = (CMTriggerMessage*)(kev[i].udata);
                
                //TO DO: cache the event in the note:articulation map
                //trigger_event_frame.events[kev[i].ident] = *controlEvent; //cache previous events of type
                
                //TO DO: count # of trigger events for each articulation
                
                //Unpack/Process the Triggered Note
                uint8_t note     = CMNoteNumberFromEventWord(triggerEvent->word);
                uint8_t velocity = CMNoteVelocityFromEventWord(triggerEvent->word);
                fprintf(stdout, "\nTrigger Event (%lu) Note = %u (%u)\n", kev[i].ident, note, velocity);
                
            }
        }
        
        //Note: renderpasses must follow this loop if they reschedule events
        //nTriggerEvents = kevent(CMTriggerEventQueue, NULL, 0, &kev[0], MAX_TRIGGER_EVENTS, ts);
        //assert(nTriggerEvents == 0); //ensure we aren't overwriting trigger events wrt the current frame buffer size
    }
    
    //Outputpass pipeline can be configured w/ custom buffer routing schemes by modifying renderpass input/output here...
    
    //A renderpass has a source resource format (SRV) and a target format (RTV)
    //The source format/buffer and target format/buffer can be the same or different
    //Resolve happens internally to the renderpass with a lookup into pb_audio_transforms

    SamplePlayerRenderPass(ioData, frames, timestamp, stream->target, &AUSamplePlayer, triggerEvent, 1);};

#pragma mark SampleAudioUnit (Presets)

/*
static const UInt8     kNumberOfPresets      = 3;
static const NSInteger kDefaultFactoryPreset = 0;

typedef struct FactoryPresetParameters
{
    AUValue cutoffValue;
    AUValue resonanceValue;

 } FactoryPresetParameters;

static const FactoryPresetParameters presetParameters[kNumberOfPresets] =
{
    // preset 0
    {
        400.0f,//FilterParamCutoff,
        -5.0f,//FilterParamResonance
    },
    
    // preset 1
    {
        6000.0f,//FilterParamCutoff,
        15.0f,//FilterParamResonance
    },
    
    // preset 2
    {
        1000.0f,//FilterParamCutoff,
        5.0f,//FilterParamResonance
    }
};

static AUAudioUnitPreset* NewAUPreset(NSInteger number, NSString *name)
{
    AUAudioUnitPreset *aPreset = [AUAudioUnitPreset new];
    aPreset.number = number;
    aPreset.name = name;
    return aPreset;
}
*/

#pragma mark - SampleAudioUnit : AUAudioUnit

@interface PBAudioUnit ()

@property AUAudioUnitBus      *outputBus;
@property AUAudioUnitBusArray *inputBusArray;
@property AUAudioUnitBusArray *outputBusArray;

@end

@implementation PBAudioUnit
{
    //AudioUnits don't create full-fledge device output streams
    //but rather use a stream context to maintain Format/OutputPass State
    PBAStreamContext          _streamContext;

    // MARK: - Member Variables

    // C++ members need to be ivars; they would be copied on access if they were properties.
    //SampleInstrumentDSPKernel  kernel;
    //std::vector<float*>        mOutputBuffers;
    //BufferedInputBus _inputBus;

    //double mSampleRate = 44100.0;
    //double mGain = 1.0;
    //double mNoteEnvelope = 0.0;
    bool mBypassed;
    
    AUAudioFrameCount             mMaxFramesToRender;   //What is meaning of TooManyFrames?
    AUHostMusicalContextBlock     mMusicalContextBlock; //TO DO
    MIDIProtocolID                mMidiProtocol;        //Trivial

    //These are expected by the AudioUnit Class Overrides for processing audio
    AUAudioUnitBus*               _outputBus;
    AUAudioUnitBusArray*          _outputBusArray;
    AVAudioFormat*                _format;
    

    //TO DO: Preset configuration
    AUAudioUnitPreset*            _currentPreset;
    NSInteger                     _currentFactoryPresetIndex;
    NSArray<AUAudioUnitPreset *> *_presets;
}

//@synthesize parameterTree  = _parameterTree;
//@synthesize factoryPresets = _presets;

- (instancetype)initWithComponentDescription:(AudioComponentDescription)componentDescription options:(AudioComponentInstantiationOptions)options error:(NSError **)outError
{
    self = [super initWithComponentDescription:componentDescription options:options error:outError];
    if (self != nil)
    {
        NSError * error = NULL;
        
        mMidiProtocol      = kMIDIProtocol_1_0;
        mBypassed          = false;
        mMaxFramesToRender = 512;
        
        // Initialize with a meaningless requested format for the busses
        // Note:  This format on the busses will be ovewritten with the host format when allocateRenderResourcesAndReturnError is called
        _format = [[AVAudioFormat alloc] initStandardFormatWithSampleRate:96000. channels:2];
        //format = [[AVAudioFormat alloc] initWithStreamDescription:&asbd];

        //Create output buss with format
        _outputBus = [[AUAudioUnitBus alloc] initWithFormat:_format error:&error]; if( error ) assert(1==0);
        _outputBus.maximumChannelCount = _format.channelCount;
        
        //Depending on the AudioUnit type, configure the input/output bus arrays
        _outputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeOutput busses:@[_outputBus]];
                
        // Maintain a PBAStream context
        memcpy(&_streamContext.format, _format.streamDescription, sizeof(AudioStreamBasicDescription));
        _streamContext.sampleRate = _outputBus.format.sampleRate;
        _streamContext.target = SampleType32BitFloat;
        
        //Set the active [Pb]Audio OutputPass + Renderpasses
        _streamContext.outputpass = PBAudioUnitOutputPass;
        
        /*
         // Create a DSP kernel to handle the signal processing.
         _kernel.init(defaultFormat.channelCount, defaultFormat.sampleRate);
         
         // Create a parameter object for the cutoff frequency.
         AUParameter *cutoffParam = [AUParameterTree createParameterWithIdentifier:@"cutoff" name:@"Cutoff"
         address:FilterParamCutoff
         min:12.0 max:20000.0 unit:kAudioUnitParameterUnit_Hertz unitName:nil
         flags: kAudioUnitParameterFlag_IsReadable |
         kAudioUnitParameterFlag_IsWritable |
         kAudioUnitParameterFlag_CanRamp
         valueStrings:nil dependentParameters:nil];
         
         // Create a parameter object for the filter resonance.
         AUParameter *resonanceParam = [AUParameterTree createParameterWithIdentifier:@"resonance" name:@"Resonance"
         address:FilterParamResonance
         min:-20.0 max:20.0 unit:kAudioUnitParameterUnit_Decibels unitName:nil
         flags: kAudioUnitParameterFlag_IsReadable |
         kAudioUnitParameterFlag_IsWritable |
         kAudioUnitParameterFlag_CanRamp
         valueStrings:nil dependentParameters:nil];
         
         // Initialize default parameter values.
         cutoffParam.value = 20000.0;
         resonanceParam.value = 0.0;
         _kernel.setParameter(FilterParamCutoff, cutoffParam.value);
         _kernel.setParameter(FilterParamResonance, resonanceParam.value);
         
         // Create factory preset array.
         _currentFactoryPresetIndex = kDefaultFactoryPreset;
         _presets = @[NewAUPreset(0, @"First Preset"),
         NewAUPreset(1, @"Second Preset"),
         NewAUPreset(2, @"Third Preset")];
         
         // Create the parameter tree.
         _parameterTree = [AUParameterTree createTreeWithChildren:@[cutoffParam, resonanceParam]];
         
         // Create the input and output busses.
         _inputBus.init(defaultFormat, 8);
         _outputBus = [[AUAudioUnitBus alloc] initWithFormat:defaultFormat error:nil];
         
         // Create the input and output bus arrays.
         _inputBusArray  = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeInput busses: @[_inputBus.bus]];
         _outputBusArray = [[AUAudioUnitBusArray alloc] initWithAudioUnit:self busType:AUAudioUnitBusTypeOutput busses: @[_outputBus]];
         
         // Make a local pointer to the kernel to avoid capturing self.
         __block FilterDSPKernel *filterKernel = &_kernel;
         
         // implementorValueObserver is called when a parameter changes value.
         _parameterTree.implementorValueObserver = ^(AUParameter *param, AUValue value) {
         filterKernel->setParameter(param.address, value);
         };
         
         // implementorValueProvider is called when the value needs to be refreshed.
         _parameterTree.implementorValueProvider = ^(AUParameter *param) {
         return filterKernel->getParameter(param.address);
         };
         
         // A function to provide string representations of parameter values.
         _parameterTree.implementorStringFromValueCallback = ^(AUParameter *param, const AUValue *__nullable valuePtr) {
         AUValue value = valuePtr == nil ? param.value : *valuePtr;
         
         switch (param.address) {
         case FilterParamCutoff:
         return [NSString stringWithFormat:@"%.f", value];
         
         case FilterParamResonance:
         return [NSString stringWithFormat:@"%.2f", value];
         
         default:
         return @"?";
         }
         };
         
         self.maximumFramesToRender = 512;
         
         // set default preset as current
         self.currentPreset = _presets.firstObject;
         */
    }
    return self;
}

-(void)dealloc
{
    _presets = nil;
}

// MARK: - Rendering

// Expresses whether an audio unit can process in place.
// In-place processing is the ability for an audio unit to transform an input signal to an
// output signal in-place in the input buffer, without requiring a separate output buffer.
// A host can express its desire to process in place by using null mData pointers in the output
// buffer list. The audio unit may process in-place in the input buffers.
// See the discussion of renderBlock.
// Partially bridged to the v2 property kAudioUnitProperty_InPlaceProcessing, the v3 property is not settable.
- (BOOL)canProcessInPlace
{
    return YES;
}

- (AUInternalRenderBlock)internalRenderBlock
{
    // Specify captured objects are mutable.
    __block uint32_t      maxFramesToRender = mMaxFramesToRender;
    __block PBAStreamContext *streamContext = &_streamContext;

    //__block SampleInstrumentDSPKernel *mKernel  = &kernel;
    //__block BufferedInputBus *input = &_inputBus;
    
    return ^OSStatus (AudioUnitRenderActionFlags *actionFlags, const AudioTimeStamp       *inTimestamp, AVAudioFrameCount           inNumberFrames,
                              NSInteger outputBusNumber, AudioBufferList *ioData, const AURenderEvent *realtimeEventListHead, AURenderPullInputBlock pullInputBlock)
    {
        //Get current timestamp
        AudioTimeStamp timestamp = *inTimestamp;

        //perform latency compensation (iOS) only
    #if TARGET_OS_IPHONE
        if ( streamContext->latencyCompensation ) timestamp.mHostTime += PBAHostTicksFromSeconds(streamContext->outputLatency);
    #endif
       
        //fprintf(stderr, "\ninBusNumber: %lu, inNumberFrames: %lu", inBusNumber, inNumberFrames );
        
    #ifdef PBA_DEBUG
        uint64_t start = PBACurrentTimeInHostTicks();
    #endif
              
        //Use the render context callback to allow the client to fill the buffer with renderpasses
        __unsafe_unretained PBAStreamOutputPass outputpass = streamContext->outputpass;
        
        // Clearing the output buffer is critical for DSP routines unless such routines inherently overwrite the buffer at all times
        // WARNING:  Errant format changes upon stream reconfiguration (eg changing buffer size) can crash by writing incorrect # of bytes
        //PBABufferListSilenceWithFormat(ioData, &streamContext->format, 0, inNumberFrames);
        
        //Run Renderpass Pipeline
        outputpass(ioData, inNumberFrames, &timestamp, streamContext);
        
         
         /*
          Important: If the caller passed non-null output pointers (outputData->mBuffers[x].mData), use those.
          
          If the caller passed null output buffer pointers, process in memory owned by the Audio Unit
          and modify the (outputData->mBuffers[x].mData) pointers to point to this owned memory.
          
          The Audio Unit is responsible for preserving the validity of this memory until the next call to render,
          or deallocateRenderResources is called.
          
          If your algorithm cannot process in-place, you will need to preallocate an output buffer and use it here.
          
          See the description of the canProcessInPlace property.
          */
         
        /*

         if (frameCount > maxFramesToRender) return kAudioUnitErr_TooManyFramesToProcess;

         //processWithEvents(&kernel, outputData, timestamp, frameCount, realtimeEventListHead);
         
         AUEventSampleTime now             = AUEventSampleTime(timestamp->mSampleTime);
         AUAudioFrameCount framesRemaining = frameCount;
         AURenderEvent const *nextEvent    = realtimeEventListHead; // events is a linked list, at the beginning, the nextEvent is the first event
         
         while (framesRemaining > 0)
         {
             // If there are no more events, we can process the entire remaining segment and exit.
             if (nextEvent == nullptr)
             {
                 AUAudioFrameCount const frameOffset = frameCount - framesRemaining;
                 
                 //callProcess(outBufferList, now, framesRemaining, frameOffset);
                 
                 //for (int channel = 0; channel < mOutputBuffers.size(); ++channel) mOutputBuffers[channel] = (float*)ioData->mBuffers[channel].mData + frameOffset;
                 //mKernel->process(mOutputBuffers, now, frameCount);
                 
                 ToneGeneratorRenderPass(ioData, frameCount, timestamp, SampleType32BitFloat, &AUToneGenerator, NULL, 0);

                 return noErr;
             }
             
             // **** start late events late.
             AUEventSampleTime timeZero          = AUEventSampleTime(0);
             AUEventSampleTime headEventTime     = nextEvent->head.eventSampleTime;
             AUAudioFrameCount framesThisSegment = AUAudioFrameCount(std::max(timeZero, headEventTime - now));
             
             // Compute everything before the next event.
             if (framesThisSegment > 0)
             {
                 AUAudioFrameCount const frameOffset = frameCount - framesRemaining;
                 
                 //callProcess(outBufferList, now, framesThisSegment, frameOffset);
                 
                 //for (int channel = 0; channel < mOutputBuffers.size(); ++channel) mOutputBuffers[channel] = (float*)ioData->mBuffers[channel].mData + frameOffset;
                 //mKernel->process(mOutputBuffers, now, frameCount);
    
                 ToneGeneratorRenderPass(ioData, framesThisSegment, timestamp, SampleType32BitFloat, &AUToneGenerator, NULL, 0);

                 // Advance frames.
                 framesRemaining -= framesThisSegment;
                 
                 // Advance time.
                 now += AUEventSampleTime(framesThisSegment);
             }
             
             //nextEvent = performAllSimultaneousEvents(now, nextEvent);
             do
             {
                 switch (nextEvent->head.eventType)
                 {
                     case AURenderEventParameter:
                     {
                         //handleParameterEvent(now, event->parameter);
                         break;
                     }
                         
                     case AURenderEventMIDI:
                     {
                         //handleMIDIEventList(now, &event->MIDIEventsList);
                         break;
                     }
                         
                     case AURenderEventMIDIEventList:
                     {
                         //handleMIDIEventList(now, &event->MIDIEventsList);
                         break;
                     }
                         
                     default:
                         break;
                 }
                 
                 // Go to next event.
                 nextEvent = nextEvent->head.next;
                 
                 // While event is not null and is simultaneous (or late).
             } while (nextEvent && nextEvent->head.eventSampleTime <= now);

         }
         */

        
        return noErr;
        
    };
}

// Allocate resources required to render.
// Subclassers should call the superclass implementation.
- (BOOL)allocateRenderResourcesAndReturnError:(NSError **)outError
{
    //update the format
    _format = self.outputBusses[0].format;
    assert(_format.channelCount <=2 ); //only stereo busses are currently handled
    
    memcpy(&_streamContext.format, _format.streamDescription, sizeof(AudioStreamBasicDescription));
    _streamContext.sampleRate = _outputBus.format.sampleRate;
    
    //Get Paths to sample files...

    const char* home = RESOURCE_HOME ? getenv(RESOURCE_HOME) : "\0";
    char   audioFileURL[256] = "\0";

#if TARGET_OS_OSX
    //TO DO: create an app documents directory suitable for sandboxed app and plugin access
    //const char* home = "/Users/jmoulton"; //TO DO: plugins get a sandboxed documents dir similar to ios
    static const char* TEST_FILE = MUSIC_DIR "/AudioAssets/UTest/FLAC/subset/28 - high resolution audio, default settings.flac"; //96k
#else
    static const char* TEST_FILE = RESOURCE_DIR "/AudioAssets/Unknown Artist/Unknown Album/Print#45.aif"; //96k
#endif
    //static const char* TEST_FILE = RESOURCE_DIR "/AudioAssets/Test/FLAC/subset/28 - high resolution audio, default settings.flac";
    //const char* audioFileURL = "/Users/jmoulton/Music/iTunes/iTunes Media/Music/Unknown Artist/Unknown Album/Print#45.aif";//"../../assets/Audio/AIF/Print#45.aif";
    //const char * audioFileURL = "/Users/jmoulton/Music/iTunes/iTunes Media/Music/ArticulationLayers/65 Drum Samples/56442_Surfjira_Snare_HeadShot_Hard.wav";

    const char* audioFileExt = "flac\0";
    //const char * audioFileExt = "aif\0";
    //const char * audioFileExt = "wav\0";

    //mtl->textures[pbrTextureIndex]->name = (char*)malloc(strlen(texturePaths[pbrTextureIndex]) + strlen(home) + 1);
    //mtl->textures[pbrTextureIndex]->name[0] = '\0';
    strcpy(audioFileURL, home);
    strcat(audioFileURL, TEST_FILE);

    //Now that the host stream format is known Renderpasses (ie kernels) can be initialized
    ToneGeneratorInit(&AUToneGenerator, 440.f, _streamContext.sampleRate);                  //Initialize a 32-bit floating point sine wave buffer
    SamplePlayerInit(&AUSamplePlayer, audioFileURL, /*audioFileExt,*/ &_streamContext.format);  //Read an audio file from disk to formatted buffer for playback
    
    //kernel.setMusicalContextBlock(self.musicalContextBlock);
    //kernel.initialize(UInt32(outputChannelCount), _outputBus.format.sampleRate);
    
    //size any intermediate buffers
    //mInputBuffers.resize(0);
    //mOutputBuffers.resize(self.outputBusses[0].format.channelCount);
    
    if (![super allocateRenderResourcesAndReturnError:outError]) return NO;
    
    return YES;
}

// Deallocate resources allocated in allocateRenderResourcesAndReturnError:
// Subclassers should call the superclass implementation.
- (void)deallocateRenderResources
{
    // Deallocate your resources.
    //kernel.deInitialize();
    
   [super deallocateRenderResources];
}

// MARK: - MIDI
-(MIDIProtocolID)audioUnitMIDIProtocol
{
    return mMidiProtocol;
}

#pragma mark - AUAudioUnit (Overrides)

- (AUAudioUnitBusArray *)inputBusses
{
    return _inputBusArray;
}

- (AUAudioUnitBusArray *)outputBusses
{
    return _outputBusArray;
}

-(AUAudioFrameCount)maximumFramesToRender
{
    return mMaxFramesToRender;
}

-(void)setMaximumFramesToRender:(AVAudioFrameCount) newValue
{
    mMaxFramesToRender = newValue;
}

-(BOOL) shouldBypassEffect
{
    return mBypassed;
}

-(void)setShouldBypassEffect:(BOOL)newValue
{
    mBypassed = newValue;
}



/*
 public func setupParameterTree(_ parameterTree: AUParameterTree) {
     self.parameterTree = parameterTree

     // Set the Parameter default values before setting up the parameter callbacks
     for param in parameterTree.allParameters {
         kernel.setParameter(param.address, param.value)
     }

     setupParameterCallbacks()
 }

 private func setupParameterCallbacks() {
     // implementorValueObserver is called when a parameter changes value.
     parameterTree?.implementorValueObserver = { [weak self] param, value -> Void in
         self?.kernel.setParameter(param.address, value)
     }

     // implementorValueProvider is called when the value needs to be refreshed.
     parameterTree?.implementorValueProvider = { [weak self] param in
         return self!.kernel.getParameter(param.address)
     }

     // A function to provide string representations of parameter values.
     parameterTree?.implementorStringFromValueCallback = { param, valuePtr in
         guard let value = valuePtr?.pointee else {
             return "-"
         }
         return NSString.localizedStringWithFormat("%.f", value) as String
     }
 }
 */

@end
