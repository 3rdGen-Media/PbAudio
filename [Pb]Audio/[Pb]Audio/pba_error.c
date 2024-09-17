//
//  PbAudioError.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 7/1/20.
//  Copyright © 2020 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"

PB_AUDIO_API PB_AUDIO_INLINE void PBAError(OSStatus result, const char * _Nonnull operation, const char * _Nonnull file, int line)
{
#ifdef __APPLE__
    if ( PBARateLimit() ) {
        int fourCC = CFSwapInt32HostToBig(result);
        if ( isascii(((char*)&fourCC)[0]) && isascii(((char*)&fourCC)[1]) && isascii(((char*)&fourCC)[2]) ) {
            printf("%s:%d: %s: '%4.4s' (%d)", file, line, operation, (char*)&fourCC, (int)result);
        } else {
            printf("%s:%d: %s: %d", file, line, operation, (int)result);
        }
    }
#endif

}
