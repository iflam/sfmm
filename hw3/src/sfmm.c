/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>

/**
 * You should store the heads of your free lists in these variables.
 * Doing so will make it accessible via the extern statement in sfmm.h
 * which will allow you to pass the address to sf_snapshot in a different file.
 */
free_list seg_free_list[4] = {
    {NULL, LIST_1_MIN, LIST_1_MAX},
    {NULL, LIST_2_MIN, LIST_2_MAX},
    {NULL, LIST_3_MIN, LIST_3_MAX},
    {NULL, LIST_4_MIN, LIST_4_MAX}
};

int sf_errno = 0;


sf_free_header* findFirstFit(size_t size){
    for(int listNum = 0; listNum < 4; listNum++){ //cycle through the lists from the lowest possible list size
        sf_free_header *checkH = seg_free_list[listNum].head;
        while(checkH != NULL){
            if(checkH->header.block_size >= size){ //if the size block is the first one we find bigger than the requested size...
                return checkH; //return this block
            }
            checkH = checkH->next;
        }
        //This means there was no room in this listNum, so we go to the next list...
    }
    return (void*)-1; //return -1, we did not find any block!
}

sf_free_header* initPage(){
    sf_header *sbrkRetVal;
    if((sbrkRetVal = sf_sbrk()) == (void*)-1){
        return (void*)-1;
    } //set the return value for the heap as a header
    printf("Heap start: %p, Heap end: %p, return val: %p\n",(int*)get_heap_start(),(int*)get_heap_end(),sbrkRetVal);
    for(int i = 0; i < FREE_LIST_COUNT; i++){
        if(seg_free_list[i].max < PAGE_SZ){
            continue;
        }
        // sbrkRetVal = header for free header
        //baseH = free header
        //baseF = footer
        sbrkRetVal->allocated = 0;
        sbrkRetVal->block_size = PAGE_SZ>>4;
        sf_free_header *baseH = (sf_free_header*)sbrkRetVal;
        baseH->next = NULL;
        baseH->prev = NULL;
        seg_free_list[i].head = baseH;
        sf_footer *baseF = (sf_footer*)((((char*)get_heap_start())+(sbrkRetVal->block_size<<4))-8);
        baseF->allocated = 0;
        baseF->block_size = PAGE_SZ>>4;
        baseF->requested_size = PAGE_SZ;
        printf("Header address: %p, Footer address: %p\n",sbrkRetVal,baseF);
        return baseH;
    }
    return (void*)-1;
}


sf_header* makeBlock(sf_free_header *freeH, size_t size, size_t allocSize){
    sf_header newH;
    //freeH must be split
    if(freeH->header.block_size - allocSize > 32){

    }
    //freeH must not be split

}
void *sf_malloc(size_t size) {
    size_t hfSize = size+16;
    size_t allocSize = if(hfSize%16==0?hfSize;hfSize+(16-hfSize%16));
    printf("Heap start: %p, Heap end: %p\n",get_heap_start(),get_heap_end());
    //check for if there is room in free lists
    sf_free_header *nextFree;
    if((nextFree = findFirstFit(allocSize))==(void*)-1){
        //NO FIRST FIT
        if((nextFree = initPage()) == (void*)-1){
            //COULD NOT CREATE PAGE
            return NULL;
        }
    //NEXTFREE
    sf_header newH;
    newH = makeBlock(nextFree, size, allocSize);
    }
    sf_snapshot();
	return NULL;
}

void *sf_realloc(void *ptr, size_t size) {
	return NULL;
}

void sf_free(void *ptr) {
	return;
}

int errorTesting(){
    return 0;
}
