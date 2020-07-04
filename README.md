# [Pb]Audio

[Physics-based] Audio is a cross-platform pure C library for establishing and producing to lowest-possible-latency audio stream buffers intended for Pro Audio mixing/rendering contexts, 3D environment mixing/rendering contexts and pipeline combinations thereof to stereo and multiple output audio hardware devices as well as network endpoints.  [Pb]Audio aims to implement the following non-standard features for all supported platforms:

### Low-Latency Render Pipeline

* Provide a abstract C API for establishing lowest-latency buffered audio ouput streams on all platforms current and future supported by CoreRender where "Lowest-Latency" should be intended to mean "closest-to-kernel" option while still conforming to platform publishing guidelines.
* Develop and extend a cross-platform Lock-Free, Thread-Safe Message Agnostic Buffer Stack implementation that utilizes structured-concurrency via stack switching.
* Suggestions?

### Physics-Modeling

* 3D environment modeling: GPGPU assisted Convolution Reverb, Transmission Loss and Occlusion Networks
* Inherently provide a mechanism for introducing analog circuit non-linearities into the audio summing network
* Consider circuit-modeling feature to aid in deriving and optimizing non-linearity solutions (much more to consider here)
* Electron noise modeling?  Q:  Is PRNG the best a personal computer can do? If so, not really interested in revisting.
* Suggestions?

*[Pb]Audio is the modular audio counterpart to the CoreRender framework.  Together, [Pb]Audio, CoreTransport and CoreRender's modular C libraries embody the foundational layer of 3rdGen's proprietary render engine and cross-platform application framework, Cobalt Rhenium.* 

## Projects

### [Physics-based] DAW

While the [Pb]Audio library exists primarily to supplement applications built within accelerated graphics environments and, as such, feature development will be driven primarily to support audio in the context of 3D rendering and animation the [Pb] Audio project of the same name attempts to demonstrate the feature-set(s) and API usage of the [Pb]Audio library in the context of a cross-platform DAW application, where associated UI and rendering will be handled by integration with CoreRender.

### Status

* 07/03/20 -- The sample project minimally demonstrates establishing an Audio Stream for rendering a 32-bit floating point sine wave buffer to the system's 'Default' audio device via a [32-bit floating point format] buffer received in synchronization with a platform event via a blocking loop on a desired thread.  Loop Elimination/Abstraction, Channel Summing and Structured-Concurrency BufferStack support to come.   
