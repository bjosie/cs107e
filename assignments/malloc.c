/*
 * File: malloc.c
 * --------------
 * This is the simple "bump" allocator from lecture.
 * An allocation request is serviced by tacking on the requested
 * space to the end of the heap thus far. 
 * It does not recycle memory (free is a no-op) so when all the
 * space set aside for the heap is consumed, it will not be able
 * to service any further requests.
 *
 * This code is given here just to show the very simplest of
 * approaches to dynamic allocation. You are to replace this code
 * with your own heap allocator implementation.
 */

#include "malloc.h"
#include <stddef.h> // for NULL
#include "strings.h"
#include "assert.h"

extern int __bss_end__;

// Simple macro to round up x to multiple of n.
// The efficient but tricky bitwise approach it uses
// works only if n is a power of two -- why?
#define roundup(x,n) (((x)+((n)-1))&(~((n)-1)))

#define TOTAL_HEAP_SIZE 0x1000000 // 16 MB


/* Global variables for the bump allocator
 *
 * `heap_start` tracks where heap segment begins in memory.
 * It is initialized to point to the address at end of data segment.
 * It uses symbol __bss_end__ from memmap to locate end.
 * `heap_used` tracks the number of bytes allocated thus far.
 * The next available memory begins at heap_start + heap_used.
 * `heap_max` is total number of bytes set aside for heap segment.
 */
static void *heap_start = NULL;
static void *heap_end = NULL;
static int heap_used = 0, heap_max = TOTAL_HEAP_SIZE;


struct header {
    size_t payload_size;
    int status;       // 0 if free, 1 if in use
};




void *findNextHeader(struct header *currHdr){
    return ((int)currHdr + sizeof(struct header) + currHdr->payload_size); //returns the address at the end of the header's payload
}


void *malloc(size_t nbytes) 
{
    if (!heap_start) {
        heap_start = &__bss_end__;
        heap_end = (&heap_start) + heap_max;
    }
    nbytes = roundup(nbytes, 8);
    // if the requested number of Bytes is more than the total bytes we have available (not counting contiguous), immediately returns NULL
    if (heap_used + nbytes > heap_max){ 
        return NULL;
    }

    struct header *nextHeader = heap_start;

    /*  If this is the first malloc request, the heap will be unused, and you     **/ 
    /*  must initialize the available payload_size to everything but the header   **/
    if(!heap_used){
        nextHeader->status = 0;
        nextHeader->payload_size = heap_max - sizeof(struct header);
    }
    
    while(nextHeader->status || nbytes > nextHeader->payload_size){
        nextHeader = (struct header*)findNextHeader(nextHeader);
        if(nextHeader > heap_end - (sizeof(struct header) + 8)){ //if it wouldn't have spece for a header + 8 bytes, don't even try
            return NULL;
        }
    } 
    //If it makes it past the while loop, it means the space following (nextHeader) is able to fulfill the malloc request

    size_t excess = nextHeader->payload_size - nbytes; //How much space is left in the payload after the malloc is returned

    if(excess >= sizeof(struct header) + 8){ //we will only make a new header if it could have a payload of at least 8
                                 //start of header //end of header  //end of payload
        struct header *newHeader = (int)nextHeader + sizeof(struct header) + nbytes;
        newHeader->status = 0;
        newHeader->payload_size = excess - sizeof(struct header);
    }

    nextHeader->status = 1;
    nextHeader->payload_size = nbytes;
    heap_used += (nbytes + sizeof(struct header));
    return (int)nextHeader + sizeof(struct header);
}

void free(void *ptr) 
{
    struct header *freeHeader = ptr - sizeof(struct header);
    freeHeader->status = 0;
    heap_used -= (sizeof(struct header) + freeHeader->payload_size);
}

void *realloc (void *old_ptr, size_t new_size)
{
    //struct header *oldHeader = old_ptr - sizeof(struct header);
    free(old_ptr);
    void *new_ptr = malloc(new_size);
    if (!new_ptr) return NULL;
    // ideally would copy the min of new_size and old_size, but this allocator
    // doesn't know the old_size. Why not? 
    // Why is it "safe" (but not efficient) to always copy new_size?
    memcpy(new_ptr, old_ptr, new_size);
    return new_ptr;
}

void heap_dump () {
    // TODO: fill in your own code here.
}
