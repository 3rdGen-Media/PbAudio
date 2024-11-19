# [Pb]Audio

[Physics-based] Audio is a cross-platform pure C library for establishing and producing to lowest-possible-latency audio stream buffers intended for Pro Audio mixing and rendering contexts, 3D environment mixing and rendering contexts and pipeline combinations thereof to stereo and multiple output audio hardware devices as well as network endpoints.  [Pb]Audio implements the following features for all supported platforms:

### Real-Time Audio Renderpass Pipeline

* <a href="https://github.com/3rdGen-Media/PbAudio/blob/master/%5BPb%5DAudio/%5BPb%5DAudio/PbAudioAPI.h">Xplatform API</a> for establishing buffered HAL client streams
* Lock-Free, Wait-Free <a href="https://github.com/3rdGen-Media/PbAudio/blob/master/%5BPb%5DAudio/%5BPb%5DAudio/pba_event_queue.h">Event Queueing</a> to Audio Threads
* Immediate-Mode <a href="https://github.com/3rdGen-Media/PbAudio/blob/master/%5BPb%5DAudio/%5BPb%5DAudio/PbAudioRenderPass.h">Renderpass</a> Pipeline Abstraction
* <a href="https://github.com/3rdGen-Media/PbAudio/blob/master/%5BPb%5DAudio/%5BPb%5DAudio/PbAudioFileStream.h">File Stream API</a> for Async Resource Loading
* Extendable <a href="https://github.com/3rdGen-Media/PbAudio/blob/master/%5BPb%5DAudio/CMidi/CMidiMessage.h">Control Message Protocol</a> for client-server IPC
* <a href="https://github.com/3rdGen-Media/PbAudio/tree/master/%5BPb%5DAudio/CMidi">MIDI 2.0 Interop</a> for Trigger Event Scheduling
* Plug-in Support for DAW Integration (<s>AAX</s>, <a href="https://github.com/3rdGen-Media/PbAudio/blob/master/Projects/%5BPb%5D%20Audio/Render/PBAudioUnit.m">AudioUnit</a>, <s>CLAP</s>, <s>VST</s>)
* Graphics Pipeline/Vertical Retrace Synchronization

### Spacial Audio System

(In Development)

*[Pb]Audio is the modular audio counterpart to the CoreRender framework.  Together, [Pb]Audio, CoreTransport and CoreRender's modular C libraries embody the foundational layer of 3rdGen's proprietary render engine and cross-platform application framework.* 

## Projects

### Sample App

While the [Pb]Audio library exists primarily to supplement applications built within accelerated graphics environments such that feature development will be driven primarily to support audio in the context of 3D rendering and animation, the '[Pb] Audio' reference project demonstrates the feature-set(s) and API usage of the [Pb]Audio library in the context of a cross-platform application, where associated UI and rendering are minimally implemented sans integration with an accelerated graphics pipeline.

<img align="center" src="https://github.com/3rdGen-Media/PbAudio/blob/master/Projects/%5BPb%5D%20Audio/Assets/Images/MainWindow.png"/>
