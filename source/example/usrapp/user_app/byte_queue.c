/*============================ INCLUDES ======================================*/
#define __BYTE_QUEUE_CLASS_IMPLEMENT

#include <stdio.h>
#include "./byte_queue.h"    

IMPLEMENT_QUEUE_INIT(byte);

//Defining the queue interface
const i_byte_queue_t BYTE_QUEUE = {
    .Init    = &QUEUE_INIT(byte),
    .Enqueue = &ENQUEUE(byte),
    .Dequeue = &DEQUEUE(byte),
    .Peek    = {
        .Reset        = &RESET_PEEK(byte),
        .Resume       = &RESUME_PEEK(byte),
        .ReadByte     = &PEEK_QUEUE(byte),
        .GetAllPeeked = &GET_ALL_PEEKED(byte)
    }
};