#include "backtrace.h"
#include "printf.h"



struct frame {
    uintptr_t saved_fp;
    uintptr_t saved_ip;
    uintptr_t saved_lr;
    uintptr_t saved_pc;
} ;

struct frame *advanceFrame(struct frame *currFrame){
    currFrame = ((uintptr_t*)currFrame->saved_fp - 3);
    return currFrame;
}

int backtrace(frame_t f[], int max_frames){
    void *cur_fp;
    __asm__("mov %0, fp" : "=r" (cur_fp));

    struct frame *currFrame = ((uintptr_t*)cur_fp-3);
    struct frame *callFrame = ((uintptr_t*)currFrame->saved_fp - 3);

    currFrame = advanceFrame(currFrame); //Must advance one frame, or the first backtraced function will be "backtrace"
    callFrame = advanceFrame(callFrame);

    int i = 0;
    while(i < max_frames && currFrame->saved_fp){
        uintptr_t function_start = (uintptr_t*)currFrame->saved_pc - 3;// <<function_name + 0>
        uintptr_t *name_header = (uintptr_t*)function_start-1; // 0xff00000c (or something like that)
        if((*(name_header) & 0xff000000) == 0xff000000){ //if the MSB are "ff"
            //name exists
            int nameLength = *name_header & ~0xff000000;
            char *function_name = ((char *)name_header - nameLength);
            f[i].name = function_name;
        } else {
            //name is a mystery
            f[i].name = "???";
        } 
        uintptr_t call_function_start = (uintptr_t*)callFrame->saved_pc - 3;// <<function_name + 0>
        f[i].resume_addr = currFrame->saved_lr;
        f[i].resume_offset = (int)f[i].resume_addr - (int)function_start;
        //f[i].resume_offset = (uintptr_t*)f[i].resume_addr - (uintptr_t*)call_function_start;
        i++;
        currFrame = advanceFrame(currFrame);
        callFrame = advanceFrame(callFrame);
    }

    return i;
    
}

/*
int backtrace(frame_t f[], int max_frames)
{
    void *cur_fp;
    __asm__("mov %0, fp" : "=r" (cur_fp));

    struct frame *currFrame = cur_fp;
    //backtrace
    uintptr_t *saved_pc = *(uintptr_t **)cur_fp;
    uintptr_t *saved_lr = *((uintptr_t **)cur_fp-1);
    uintptr_t *saved_ip = *((uintptr_t **)cur_fp-2);
    uintptr_t *saved_fp = *((uintptr_t **)cur_fp-3);

    cur_fp = saved_fp;
    int i = 0;
    while (i < max_frames && cur_fp){
        //Set Frame Values
        uintptr_t *saved_pc = *(uintptr_t **)cur_fp;
        uintptr_t *saved_lr = *((uintptr_t **)cur_fp-1);
        uintptr_t *saved_ip = *((uintptr_t **)cur_fp-2);
        uintptr_t *saved_fp = *((uintptr_t **)cur_fp-3);
        uintptr_t **cf_fp = (uintptr_t **)saved_fp;
        uintptr_t *cf_saved_pc = *cf_fp;
        uintptr_t *cf_saved_lr = *(cf_fp-1);
       
        //Points to where the name is stored
        uintptr_t *name_header = (*(uintptr_t **)cur_fp-4); 

        if((*(name_header) & 0xff000000) == 0xff000000){ //if the MSB are "ff"
            //name exists
            int nameLength = *name_header & ~0xff000000;
            char *function_name = ((char *)name_header - nameLength);
            f[i].name = function_name;
        } else {
            //name is a mystery
            f[i].name = "???";
        }
        f[i].resume_addr = cf_saved_lr;
        f[i].resume_offset = (uintptr_t*)f[i].resume_addr - (cf_saved_pc - 3);

        //set fp for next loo
        cur_fp = saved_fp;
        //increment number of backtraces done
        i++;
    }
    
    return i;
}
*/



void print_frames (frame_t f[], int n)
{
    for (int i = 0; i < n; i++)
        printf("#%d 0x%x at %s+%d\n", i, f[i].resume_addr, f[i].name, f[i].resume_offset);
}

void print_backtrace (void)
{
    int max = 50;
    frame_t arr[max];

    int n = backtrace(arr, max);
    print_frames(arr+1, n-1);   // print frames starting at this function's caller
}
