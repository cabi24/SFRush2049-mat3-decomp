/* memsys.c - Heap, memory chunk and master-handle allocation. Status: matched.
   Original: D:\rush_ngc\Rush_NGC\GameSource\src\memsys.c; assembly: build/2049LE/asm/GameSource/src/memsys.s */
#include "rush/rush_protos.h"
#include "rush/dolphin.h"

static void _FillNewMem(void * ptr, unsigned int size);
static void _InitHeap(tHeap *heap, unsigned short numInitialMasters, unsigned short numAdditionalMasters);
static tHeap *_ConvertHeap(void * heap);
static unsigned int _LargestAvailableMemoryChunk(tHeap *heap);
static void check_mem();
static void * _AllocateMemory(tHeap *heap, unsigned int size, void * master, int internal);
static void _ReallocateMemory(void * ptr, unsigned int newSize, int internal);
static void _DeallocateMemory(void * ptr, int internal);
static unsigned int _GetPtrSize(void * ptr, int internal);
static tHeap *_FindHeap(void * ptr);
static tMemoryChunk *_FindChunk(tHeap *heap, void * ptr, int internal);
static void _GetSemaphore(void);
static void _ReleaseSemaphore(void);

tHeap *gMainHeap;
int GMem;
int GHeapSize;

/* 0x8002bd3c, size 0x9c */
void InitMemory(unsigned short numInitialMasters, unsigned short numAdditionalMasters)
{
    unsigned int avail;
    tHeap *heap;
    unsigned int heap_size;
    heap_size = ngcGetHeapSize() - 0x600000;
    avail = heap_size - 0x19000;
    do
    {
        avail -= 64;
        avail &= ~63;
        gMainHeap = (tHeap *)_mem_driver_malloc(avail);
    }
    while (!gMainHeap);
    _mem_driver_free(gMainHeap);
    GHeapSize = avail - 0x1400;
    gMainHeap = (tHeap *)_mem_driver_malloc(avail - 0x1400);
    heap_size = avail + (unsigned int)gMainHeap;
    gMainHeap->endRam = (unsigned char *)((heap_size - 0x1400) & ~31);
    _InitHeap(gMainHeap,numInitialMasters,numAdditionalMasters);
}

/* 0x8002bdd8, size 0x30 */
static void _FillNewMem(void * ptr, unsigned int size)
{
    if (size) memset(ptr,0xcd,size);
}

/* 0x8002be08, size 0xe4 */
static void _InitHeap(tHeap *heap, unsigned short numInitialMasters, unsigned short numAdditionalMasters)
{
    heap->magic = 0xfedcba98;
    heap->next = 0;
    heap->chunks = (tMemoryChunk *)((unsigned char *)heap + 64);
    heap->chunks->magic = 0xfedcba98;
    heap->chunks->next = 0;
    heap->chunks->prev = 0;
    heap->chunks->size = heap->endRam - (unsigned char *)heap->chunks - 32;
    heap->chunks->master = 0;
    heap->chunks->used = 0;
    heap->chunks->internal = 0;
    heap->chunks->locked = 0;
    heap->lastChunk = heap->chunks;
    heap->masters.numMasters = numInitialMasters;
    heap->numAdditionalMasters = numAdditionalMasters;
    heap->masters.nextBlock = 0;
    if (numInitialMasters)
    {
        heap->masters.masters = _AllocateMemory(heap,numInitialMasters*4,0,1);
        memset(heap->masters.masters,0,numInitialMasters*4);
    }
    else heap->masters.masters = 0;
}

/* 0x8002beec, size 0x10 */
static tHeap *_ConvertHeap(void * heap)
{
    if (!heap) return gMainHeap;
    return (tHeap *)heap;
}

/* 0x8002befc, size 0x44 */
unsigned int LargestAvailableMemoryHeapChunk(void * heap)
{
    unsigned int maxChunk;
    _GetSemaphore();
    maxChunk = _LargestAvailableMemoryChunk(_ConvertHeap(heap));
    _ReleaseSemaphore();
    return maxChunk;
}

/* 0x8002bf40, size 0x3c */
static unsigned int _LargestAvailableMemoryChunk(tHeap *heap)
{
    tMemoryChunk *chunk;
    unsigned int maxChunk=0;
    for(chunk=heap->chunks;chunk;chunk=chunk->next) if(!chunk->used && chunk->size>maxChunk) maxChunk=chunk->size;
    return maxChunk & ~63;
}

/* 0x8002bf7c, size 0x4 */
static void check_mem()
{

}

/* 0x8002bf80, size 0x5c */
void * AllocateMemoryHeap(void * heap, unsigned int size)
{
    void *ptr;
    _GetSemaphore();
    ptr = _AllocateMemory(_ConvertHeap(heap),size,0,0);
    _ReleaseSemaphore();
    return ptr;
}

/* 0x8002bfdc, size 0x15c */
static void * _AllocateMemory(tHeap *heap, unsigned int size, void * master, int internal)
{
    tMemoryChunk *chunk;
    tMemoryChunk *newChunk;
    unsigned int freeMem=0,maxFree=0;
    size = (size + 31) & ~31;
    retry:
    for(chunk=heap->chunks;chunk;chunk=chunk->next)
    {
        if(!chunk->used)
        {
            freeMem+=chunk->size;
            if(chunk->size>maxFree)maxFree=chunk->size;
            if(chunk->size>=size)break;
        }
    }
    if(!chunk && heap->emergencyFunc)
    {
        _ReleaseSemaphore();
        if(heap->emergencyFunc(size))
        {
            _GetSemaphore();
            goto retry;
        }
        _GetSemaphore();
    }
    if(chunk->size-size>=64)
    {
        newChunk=(tMemoryChunk *)((unsigned char *)chunk+size+32);
        newChunk->next=chunk->next;
        if(newChunk->next)newChunk->next->prev=newChunk;
        else heap->lastChunk=newChunk;
        newChunk->prev=chunk;
        newChunk->size=chunk->size-size-32;
        newChunk->master=0;
        newChunk->used=0;
        newChunk->internal=0;
        newChunk->locked=0;
        newChunk->magic=0xfedcba98;
        chunk->next=newChunk;
        chunk->size=size;
    }
    chunk->used=1;
    chunk->master=master;
    chunk->internal=internal;
    _FillNewMem((unsigned char *)chunk+32,size);
    check_mem((unsigned char *)chunk+32,size);
    return (unsigned char *)chunk+32;
}

/* 0x8002c138, size 0x17c */
void * AllocateMemoryHeapHigh(void * h, unsigned int size)
{
    tHeap *heap;
    tMemoryChunk *chunk,*newChunk;
    unsigned int freeMem,maxFree;
    _GetSemaphore();
    heap=_ConvertHeap(h);
    size=(size+31)&~31;
    freeMem=0;
    maxFree=0;
    retry:
    for(chunk=heap->lastChunk;chunk;chunk=chunk->prev)
    {
        if(!chunk->used)
        {
            freeMem+=chunk->size;
            if(chunk->size>maxFree)maxFree=chunk->size;
            if(chunk->size>=size)break;
        }
    }
    if(!chunk && heap->emergencyFunc)
    {
        _ReleaseSemaphore();
        if(heap->emergencyFunc(size))
        {
            _GetSemaphore();
            goto retry;
        }
        _GetSemaphore();
    }
    if(chunk->size-size>=64)
    {
        newChunk=(tMemoryChunk *)((unsigned char *)chunk+chunk->size-size);
        newChunk->next=chunk->next;
        if(newChunk->next)newChunk->next->prev=newChunk;
        else heap->lastChunk=newChunk;
        newChunk->prev=chunk;
        newChunk->size=size;
        newChunk->master=0;
        newChunk->used=1;
        newChunk->internal=0;
        newChunk->locked=0;
        newChunk->magic=0xfedcba98;
        chunk->next=newChunk;
        chunk->size-=size+32;
        chunk=newChunk;
    }
    else
    {
        chunk->master=0;
        chunk->used=1;
        chunk->internal=0;
    }
    _ReleaseSemaphore();
    _FillNewMem((unsigned char *)chunk+32,size);
    check_mem((unsigned char *)chunk+32,size);
    return (unsigned char *)chunk+32;
}

/* 0x8002c2b4, size 0x4c */
void ReallocateMemory(void * ptr, unsigned int newSize)
{
    _GetSemaphore();
    _ReallocateMemory(ptr,newSize,0);
    _ReleaseSemaphore();
}

/* 0x8002c300, size 0x220 */
static void _ReallocateMemory(void * ptr, unsigned int newSize, int internal)
{
    tHeap *heap;
    tMemoryChunk *chunk,*nextChunk,*newChunk;
    newSize=(newSize+31)&~31;
    heap=_FindHeap(ptr);
    chunk=_FindChunk(heap,ptr,internal);
    if(newSize<=chunk->size)
    {
        nextChunk=chunk->next;
        if(!nextChunk || nextChunk->used)
        {
            if(chunk->size-newSize>=64)
            {
                newChunk=(tMemoryChunk *)((unsigned char *)chunk+newSize+32);
                newChunk->next=nextChunk;
                if(newChunk->next)newChunk->next->prev=newChunk;
                else heap->lastChunk=newChunk;
                newChunk->prev=chunk;
                newChunk->size=chunk->size-newSize-32;
                newChunk->master=0;
                newChunk->used=0;
                newChunk->internal=0;
                newChunk->locked=0;
                newChunk->magic=0xfedcba98;
                chunk->next=newChunk;
                chunk->size=newSize;
            }
        }
        else if(chunk->size-newSize>=32)
        {
            newChunk=(tMemoryChunk *)((unsigned char *)chunk+newSize+32);
            newChunk->next=nextChunk->next;
            if(newChunk->next)newChunk->next->prev=newChunk;
            else heap->lastChunk=newChunk;
            newChunk->prev=chunk;
            newChunk->size=chunk->size-newSize+nextChunk->size;
            newChunk->master=0;
            newChunk->used=0;
            newChunk->internal=0;
            newChunk->locked=0;
            newChunk->magic=0xfedcba98;
            chunk->next=newChunk;
            chunk->size=newSize;
        }
    }
    else
    {
        nextChunk=chunk->next;
        if(nextChunk->size-(newSize-chunk->size)<32)
        {
            chunk->next=nextChunk->next;
            if(chunk->next)chunk->next->prev=chunk;
            else heap->lastChunk=chunk;
            newSize=chunk->size+nextChunk->size+32;
        }
        else
        {
            newChunk=(tMemoryChunk *)((unsigned char *)chunk+newSize+32);
            newChunk->next=nextChunk->next;
            if(newChunk->next)newChunk->next->prev=newChunk;
            else heap->lastChunk=newChunk;
            newChunk->prev=chunk;
            newChunk->size=chunk->size+nextChunk->size-newSize;
            newChunk->master=0;
            newChunk->used=0;
            newChunk->internal=0;
            newChunk->locked=0;
            newChunk->magic=0xfedcba98;
            chunk->next=newChunk;
        }
        chunk->size=newSize;
    }
}

/* 0x8002c520, size 0x180 */
void * AllocateHandleHeap(void * h, unsigned int size)
{
    tHeap *heap;
    void *master;
    tMasterBlock *block;
    int i;
    _GetSemaphore();
    heap=_ConvertHeap(h);
    block=&heap->masters;
    if(!heap->masters.masters)
    {
        block->numMasters=heap->numAdditionalMasters;
        block->masters=_AllocateMemory(heap,heap->numAdditionalMasters*4,0,1);
        memset(block->masters,0,heap->numAdditionalMasters*4);
        block->nextBlock=0;
    }
    for(;;)
    {
        master=block->masters;
        for(i=0;i<block->numMasters;i++,master=(unsigned char *)master+4)if(!*(void **)master)break;
        if(i<block->numMasters)break;
        if(!block->nextBlock)
        {
            block->nextBlock=_AllocateMemory(heap,sizeof(tMasterBlock),0,1);
            block->nextBlock->numMasters=heap->numAdditionalMasters;
            block->nextBlock->masters=_AllocateMemory(heap,heap->numAdditionalMasters*4,0,1);
            memset(block->nextBlock->masters,0,heap->numAdditionalMasters*4);
            block->nextBlock->nextBlock=0;
        }
        block=block->nextBlock;
    }
    *(void **)master=_AllocateMemory(heap,size,master,0);
    check_mem(*(void **)master,size);
    _ReleaseSemaphore();
    return master;
}

/* 0x8002c6a0, size 0x28 */
void _FillFreeMem(void * ptr, unsigned int size)
{
    memset(ptr,0xef,size);
}

/* 0x8002c6c8, size 0x3c */
void DeallocateMemory(void * ptr)
{
    _GetSemaphore();
    _DeallocateMemory(ptr,0);
    _ReleaseSemaphore();
}

/* 0x8002c704, size 0x134 */
static void _DeallocateMemory(void * ptr, int internal)
{
    tHeap *heap;
    tMemoryChunk *chunk,*nextChunk,*prevChunk;
    heap=_FindHeap(ptr);
    chunk=_FindChunk(heap,ptr,internal);
    _FillFreeMem((unsigned char *)chunk+32,chunk->size);
    if(chunk->master)*(void **)chunk->master=0;
    nextChunk=chunk->next;
    if(nextChunk&&!nextChunk->used)
    {
        chunk->next=nextChunk->next;
        if(chunk->next)chunk->next->prev=chunk;
        else heap->lastChunk=chunk;
        chunk->size+=nextChunk->size+32;
        _FillFreeMem(nextChunk,32);
    }
    chunk->master=0;
    chunk->used=0;
    chunk->internal=0;
    chunk->locked=0;
    prevChunk=chunk->prev;
    if(prevChunk&&!prevChunk->used)
    {
        prevChunk->next=chunk->next;
        if(prevChunk->next)prevChunk->next->prev=prevChunk;
        else heap->lastChunk=prevChunk;
        prevChunk->size+=chunk->size+32;
        _FillFreeMem(chunk,32);
    }
}

/* 0x8002c838, size 0x98 */
void PurgeableMemory(void * ptr, int purgeable)
{
    tHeap *heap;
    tMemoryChunk *chunk;
    _GetSemaphore();
    heap=_FindHeap(ptr);
    chunk=_FindChunk(heap,ptr,0);
    if(chunk->master)
    {
        _ReleaseSemaphore();
        return;
    }
    if(purgeable)
    {
        if(chunk->locked)chunk->locked--;
    }
    else
    {
        if(chunk->locked<255)chunk->locked++;
    }
    _ReleaseSemaphore();
}

/* 0x8002c8d0, size 0x44 */
unsigned int GetPtrSize(void * ptr)
{
    unsigned int size;
    _GetSemaphore();
    size=_GetPtrSize(ptr,0);
    _ReleaseSemaphore();
    return size;
}

/* 0x8002c914, size 0x48 */
static unsigned int _GetPtrSize(void * ptr, int internal)
{
    tHeap *heap;
    tMemoryChunk *chunk;
    heap=_FindHeap(ptr);
    chunk=_FindChunk(heap,ptr,internal);
    return chunk->size;
}

/* 0x8002c95c, size 0x3c */
static tHeap *_FindHeap(void * ptr)
{
    tHeap *heap,*lastHeap=0;
    for(heap=gMainHeap;heap;heap=heap->next)if((unsigned int)ptr>=(unsigned int)heap->chunks&&(unsigned int)ptr<(unsigned int)heap->endRam)lastHeap=heap;
    return lastHeap;
}

/* 0x8002c998, size 0x88 */
static tMemoryChunk *_FindChunk(tHeap *heap, void * ptr, int internal)
{
    tMemoryChunk *chunk;
    tMasterBlock *block;
    for(block=&heap->masters;block;block=block->nextBlock)
    {
        if(!block->masters)break;
        if((unsigned int)ptr>=(unsigned int)block->masters&&(unsigned int)ptr<(unsigned int)block->masters+block->numMasters*4)
        {
            ptr=*(void **)ptr;
            break;
        }
    }
    for(chunk=heap->chunks;chunk;chunk=chunk->next)if(chunk->internal==internal&&(unsigned int)ptr>=(unsigned int)chunk&&(!chunk->next||(unsigned int)ptr<(unsigned int)chunk->next))break;
    return chunk;
}

/* 0x8002ca20, size 0x4 */
static void _GetSemaphore(void)
{

}

/* 0x8002ca24, size 0x4 */
static void _ReleaseSemaphore(void)
{

}

/* 0x8002ca28, size 0x28 */
char *_mem_driver_malloc(int nSize)
{
    return OSAllocFromHeap(__OSCurrHeap,nSize);
}

/* 0x8002ca50, size 0x28 */
void _mem_driver_free(void * pMem)
{
    OSFreeToHeap(__OSCurrHeap,pMem);
}

