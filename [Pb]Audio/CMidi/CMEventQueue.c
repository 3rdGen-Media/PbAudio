//
//  CMEventQueue.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/17/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "CMEventQueue.h"

cm_kernel_queue_id CMTriggerEventQueue; //a global kqueue singleton for the main Core Midi Event Loop to IPC with real-time audio threads


CMKernelQueue CMKernelQueueCreate(void)
{
    CMKernelQueue queue = { 0 };
#ifndef _WIN32
    queue.kq = kqueue();//{kqueue(), pipe(kq.rxPipe)};
    pipe(queue.pq);
#else
    queue.kq = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
#endif
    queue.pnInflightCursors = (intptr_t) & (queue.pnInflightCursors); //voodoo!
    return queue;
}