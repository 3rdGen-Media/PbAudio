//
//  pba_event_queue.c
//  [Pb]Audio
//
//  Created by Joe Moulton on 9/9/24.
//  Copyright © 2024 3rdGen Multimedia. All rights reserved.
//

#include "../[Pb]Audio.h"
//#include "pba_event_queue.h"

#pragma mark -- Global Process Handles

//pba_pid_t pba_displaySyncProcess;

#pragma mark -- Init Main Thread/Run Loop References

pba_platform_thread    pba_mainThread      = NULL;
pba_platform_thread_id pba_mainThreadID;// = 0;

pba_platform_thread    pba_eventThread      = NULL;
pba_platform_thread_id pba_eventThreadID;// = 0;


#pragma mark --

#ifdef _WIN32
DWORD                 pba_tlsIndex;
#endif

#pragma mark -- Define Global Kernel Queue Events
pba_kernel_queue_event pba_vBlankNotification = {0};

#pragma mark --Define Global Kernel Event Queues/Pipes

//pba_kernel_queue_id PBAudioEventQueue;
//pba_kernel_queue_id pba_platformEventQueue;
//pba_kernel_queue_id pba_inputEventQueue;          //a global kqueue singleton for distributing user input updates from main thread run loop

//a global read/write pipe pair singleton for sending blob messages to to the pba_displayEventQueue (allows waking pba_displayEventQueue from a separate process)
//int pba_displayEventPipe[2];

//PBAKernelQueue pba_displayEventQueue = { 0 };

//int64_t     pba_mainWindow;
//crgc_view * pba_mainView;            //Main crgc_view reference

#ifndef CR_TARGET_WIN32
PB_AUDIO_API PB_AUDIO_INLINE uintptr_t pba_event_queue_wait_with_timeout(int kqueue, struct kevent * kev, int16_t eventFilter, uintptr_t timeoutEvent, uintptr_t outOfRangeEvent, uintptr_t eventRangeStart, uintptr_t eventRangeEnd, uint32_t timeout)
{
    ////NSLog(@"wait for event with timeout");
    struct timespec _ts;
    struct timespec *ts = NULL;
    if (timeout != UINT_MAX) {
        ts = &_ts;
        
        ts->tv_sec = 0;//(timeout - (timeout % 1000)) / 1000;
        ts->tv_nsec = (timeout /*% 1000*/);// * 1000;
    }
    
    //while (1) {
    //struct kevent kev;
    //int n = kevent(kqueue, NULL, 0, &kev, 1, ts);
    int n = kevent(kqueue, NULL, 0, kev, 1, ts);
    if (n > 0) {
        if (kev->filter == eventFilter && kev->ident >= eventRangeStart && kev->ident <= eventRangeEnd) {
            return kev->ident;
        }
        return outOfRangeEvent;
    }
    //else {
    //    break;
    //}
    // }
    
    return timeoutEvent;
}

#endif

PBAKernelQueue PBAKernelQueueCreate(void)
{
    PBAKernelQueue queue = { 0 };
#ifndef _WIN32
    queue.kq = kqueue(); //{kqueue(), pipe(kq.rxPipe)};
    //pipe(queue.pq);
#else
    queue.kq = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, (ULONG_PTR)NULL, 0);
#endif
    queue.pnInflightCursors = (intptr_t) & (queue.pnInflightCursors); //voodoo!
    return queue;
}

