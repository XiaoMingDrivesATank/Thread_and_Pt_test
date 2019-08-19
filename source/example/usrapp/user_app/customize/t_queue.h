#ifndef _TEMPLATE_QUEUE_H_
#define _TEMPLATE_QUEUE_H_

/******************************************************************************************/
#define QUEUE_INIT(_NAME)                                                                  \
        T_QUEUE_INIT(_NAME)

#define T_QUEUE_INIT(_NAME)                                                                \
        init_##_NAME##_queue

/******************************************************************************************/
#define IS_QUEUE_EMPTY(_NAME)                                                              \
        T_IS_QUEUE_EMPTY(_NAME)
        
#define T_IS_QUEUE_EMPTY(_NAME)                                                            \
        is_##_NAME##_queue_empty

/******************************************************************************************/
#define IS_QUEUE_FULL(_NAME)                                                              \
        T_IS_QUEUE_FULL(_NAME)
        
#define T_IS_QUEUE_FULL(_NAME)                                                            \
        is_##_NAME##_queue_full
        
/******************************************************************************************/
#define RESET_PEEK(_NAME)                                                                  \
        T_RESET_PEEK(_NAME)
        
#define T_RESET_PEEK(_NAME)                                                                \
        reset_peek_##_NAME
        
/******************************************************************************************/
#define RESUME_PEEK(_NAME)                                                                 \
        T_RESUME_PEEK(_NAME)
        
#define T_RESUME_PEEK(_NAME)                                                               \
        resume_peek_##_NAME
        
/******************************************************************************************/
#define PEEK_QUEUE(_NAME)                                                                  \
        T_PEEK_QUEUE(_NAME)
        
#define T_PEEK_QUEUE(_NAME)                                                                \
        peek_##_NAME##_queue
        
/******************************************************************************************/
#define GET_ALL_PEEKED(_NAME)                                                              \
        T_GET_ALL_PEEKED(_NAME)
        
#define T_GET_ALL_PEEKED(_NAME)                                                            \
        get_all_peekd_##_NAME
        
/******************************************************************************************/
#define ENQUEUE(_NAME)                                                                     \
        T_ENQUEUE(_NAME)
        
#define T_ENQUEUE(_NAME)                                                                   \
        enqueue_##_NAME
        
/******************************************************************************************/
#define DEQUEUE(_NAME)                                                                     \
        T_DEQUEUE(_NAME)
        
#define T_DEQUEUE(_NAME)                                                                   \
        dequeue_##_NAME

/******************************************************************************************/
#define T_EXTERN_QUEUE(_NAME, _METHOD, _TYPE, _CAPACITY)                                   \
declare_class(_NAME##_queue_t)                                                             \
_METHOD##_class(_NAME##_queue_t,                                                           \
                               ,                                                           \
    private_member(                                                                        \
        uint8_t   chQueueVersions;  /*Queue version*/                                      \
        _TYPE     *pchBuffer;       /*Queue buffer space header address*/                  \
        _CAPACITY hwSize;           /*Queue size*/                                         \
        _CAPACITY hwHead;           /*Queue head*/                                         \
        _CAPACITY hwTail;           /*Queue tail*/                                         \
        _CAPACITY hwLength;         /*Number of elements*/                                 \
        _CAPACITY hwPeekHead;       /*Peek shadow queue head*/                             \
        _CAPACITY hwPeekedCount;    /*The number of elements in the shadow queue peek*/    \
    )                                                                                      \
)                                                                                          \
end_##_METHOD##_class(_NAME##_queue_t)                                                   

#define T_EXTERN_CONTEXT(_METHOD, _CAPACITY)                                               \
declare_class(peek_context_t)                                                              \
_METHOD##_class(peek_context_t,                                                            \
                              ,                                                            \
    private_member(                                                                        \
        _CAPACITY hwPeekHeadBackups; /*Peek shadow queue header backup*/                   \
        _CAPACITY hwPeekCountBackups;/*Shadow queue peek out the number of elements backup*/\
        uint8_t   chPeekVersions;    /*Peek version data*/                                 \
    )                                                                                      \
)                                                                                          \
end_##_METHOD##_class(peek_context_t)  

/******************************************************************************************/
#define IMPLEMENT_QUEUE_INIT(_NAME)                                                        \
/*function: Loop queue initialization*/                                                    \
/*ptThis:   Queue pointer*/                                                                \
/*hwSize:   Queue length*/                                                                 \
void init_##_NAME##_queue(_NAME##_queue_t *ptQueue, uint8_t *pchBuffer, uint16_t hwSize)   \
{                                                                                          \
    do {                                                                                   \
        class_internal(ptQueue, ptThis, _NAME##_queue_t);                                  \
        if((NULL == ptThis) || (NULL == pchBuffer)) {                                      \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        ptThis->pchBuffer = pchBuffer;                                                     \
        ptThis->hwSize    = hwSize;                                                        \
    }while(0);                                                                             \
}                                                                                          \
                                                                                           \
/*function: Whether the loop queue is empty or not*/                                       \
/*ptThis:   Queue pointer*/                                                                \
static bool is_##_NAME##_queue_empty(_NAME##_queue_t *ptQueue)                             \
{                                                                                          \
    do {                                                                                   \
        class_internal(ptQueue, ptThis, _NAME##_queue_t);                                  \
        if(NULL == ptThis) {                                                               \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        if(0 == ptThis->hwLength) {                                                        \
            return true;                                                                   \
        }                                                                                  \
    }while(0);                                                                             \
    return false;                                                                          \
}                                                                                          \
                                                                                           \
/*function: Whether the loop queue is full*/                                               \
/*ptThis:   Queue pointer*/                                                                \
static bool is_##_NAME##_queue_full(_NAME##_queue_t *ptQueue)                              \
{                                                                                          \
    do {                                                                                   \
        class_internal(ptQueue, ptThis, _NAME##_queue_t);                                  \
        if(NULL == ptThis) {                                                               \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        if(ptThis->hwSize == ptThis->hwLength) {                                           \
            return true;                                                                   \
        }                                                                                  \
    }while(0);                                                                             \
    return false;                                                                          \
}                                                                                          \
                                                                                           \
/*function:   Reset PEEK pointer*/                                                         \
/*ptThis:     Queue pointer*/                                                              \
/*ptContext:  PEEK context pointer*/                                                       \
bool reset_peek_##_NAME(_NAME##_queue_t *ptQueue, peek_context_t *ptContext)               \
{                                                                                          \
    do {                                                                                   \
        class_internal(ptQueue, ptThis, _NAME##_queue_t);                                  \
        class_internal(ptContext, ptC, peek_context_t);                                    \
        if(NULL == ptThis) {                                                               \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        if(ptC != NULL) {                                                                  \
            if(ptThis->chQueueVersions >= 255) {                                           \
                ptThis->chQueueVersions = 0;                                               \
            }                                                                              \
                                                                                           \
            ptC->hwPeekHeadBackups  = ptThis->hwPeekHead;                                  \
            ptC->hwPeekCountBackups = ptThis->hwPeekedCount;                               \
            ptC->chPeekVersions     = ptThis->chQueueVersions;                             \
        }                                                                                  \
        ptThis->hwPeekHead = ptThis->hwHead;                                               \
        ptThis->hwPeekedCount = 0;                                                         \
        return true;                                                                       \
    }while(0);                                                                             \
    return false;                                                                          \
}                                                                                          \
                                                                                           \
/*function:      Restore PEEK pointer*/                                                    \
/*ptThis:        Queue pointer*/                                                           \
/*ptContext:     PEEK context pointer*/                                                    \
bool resume_peek_##_NAME(_NAME##_queue_t *ptQueue, peek_context_t *ptContext)              \
{                                                                                          \
    do {                                                                                   \
        class_internal(ptQueue, ptThis, _NAME##_queue_t);                                  \
        class_internal(ptContext, ptC, peek_context_t);                                    \
        if((NULL == ptThis) || (NULL == ptC)) {                                            \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        if(ptThis->chQueueVersions != ptC->chPeekVersions) {                               \
           ptThis->chQueueVersions = 0;                                                    \
           break;                                                                          \
        }                                                                                  \
                                                                                           \
        ptThis->hwPeekHead    = ptC->hwPeekHeadBackups;                                    \
        ptThis->hwPeekedCount = ptC->hwPeekCountBackups;                                   \
        return true;                                                                       \
    }while(0);                                                                             \
    return false;                                                                          \
}                                                                                          \
                                                                                           \
/*function:  PEEK an element*/                                                             \
/*ptThis:    Queue pointer*/                                                               \
/*pchAddr:   Receive the variable address of the element*/                                 \
bool peek_##_NAME##_queue(_NAME##_queue_t *ptQueue, uint8_t *pchAddr)                      \
{                                                                                          \
    do {                                                                                   \
        class_internal(ptQueue, ptThis, _NAME##_queue_t);                                  \
        if((NULL == ptThis) || (NULL == pchAddr)) {                                        \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        if(0 == (ptThis->hwLength - ptThis->hwPeekedCount)) {                              \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        *pchAddr = ptThis->pchBuffer[ptThis->hwPeekHead];                                  \
        ptThis->hwPeekHead++;                                                              \
        if(ptThis->hwSize <= ptThis->hwPeekHead) {                                         \
            ptThis->hwPeekHead = 0;                                                        \
        }                                                                                  \
        ptThis->hwPeekedCount++;                                                           \
        return true;                                                                       \
    }while(0);                                                                             \
    return false;                                                                          \
}                                                                                          \
                                                                                           \
/*function:  Delete all elements of PEEK*/                                                 \
/*ptThis:    Queue pointer*/                                                               \
bool get_all_peekd_##_NAME(_NAME##_queue_t *ptQueue)                                       \
{                                                                                          \
    do {                                                                                   \
        class_internal(ptQueue, ptThis, _NAME##_queue_t);                                  \
        if((NULL == ptThis)) {                                                             \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        ptThis->chQueueVersions++;                                                         \
        ptThis->hwHead        = ptThis->hwPeekHead;                                        \
        ptThis->hwLength      = (ptThis->hwLength - ptThis->hwPeekedCount);                \
        ptThis->hwPeekedCount = 0;                                                         \
        return true;                                                                       \
    }while(0);                                                                             \
    return false;                                                                          \
}                                                                                          \
                                                                                           \
/*function:  Add elements to the queue*/                                                   \
/*ptThis:    Queue pointer*/                                                               \
/*chObj:     Element pointer to the queue*/                                                \
bool enqueue_##_NAME(_NAME##_queue_t *ptQueue, uint8_t chObj)                              \
{                                                                                          \
    do {                                                                                   \
        class_internal(ptQueue, ptThis, _NAME##_queue_t);                                  \
        if(NULL == ptThis) {                                                               \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        if(is_byte_queue_full((_NAME##_queue_t *)ptThis)) {                                \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        ptThis->pchBuffer[ptThis->hwTail] = chObj;                                         \
        ptThis->hwTail++;                                                                  \
        if(ptThis->hwSize <= ptThis->hwTail) {                                             \
            ptThis->hwTail = 0;                                                            \
        }                                                                                  \
        ptThis->hwLength++;                                                                \
        return true;                                                                       \
    }while(0);                                                                             \
    return false;                                                                          \
}                                                                                          \
                                                                                           \
/*function:  Queue derailed*/                                                              \
/*ptThis:    Queue pointer*/                                                               \
/*pchAddr:   Receive the variable address of the team element*/                            \
bool dequeue_##_NAME(_NAME##_queue_t *ptQueue, uint8_t *pchAddr)                           \
{                                                                                          \
    do {                                                                                   \
        class_internal(ptQueue, ptThis, _NAME##_queue_t);                                  \
        if((NULL == ptThis) || (NULL == pchAddr)) {                                        \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        if(is_byte_queue_empty((_NAME##_queue_t *)ptThis)) {                               \
            break;                                                                         \
        }                                                                                  \
                                                                                           \
        *pchAddr = ptThis->pchBuffer[ptThis->hwHead];                                      \
        ptThis->hwHead++;                                                                  \
        if(ptThis->hwSize <= ptThis->hwHead) {                                             \
            ptThis->hwHead = 0;                                                            \
        }                                                                                  \
        ptThis->hwLength--;                                                                \
        ptThis->chQueueVersions++;                                                         \
        reset_peek_byte((byte_queue_t *)ptThis, NULL);                                     \
        return true;                                                                       \
    }while(0);                                                                             \
    return false;                                                                          \
}

#endif
