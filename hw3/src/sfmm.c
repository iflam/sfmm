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
        sf_free_header *nextF;
        nextF = seg_free_list[i].head;
        //sets the links
        if(nextF != NULL)
            baseH->next = nextF;
        seg_free_list[i].head = baseH;
        sf_footer *baseF = (sf_footer*)((((char*)get_heap_start())+(sbrkRetVal->block_size<<4))-8);
        baseF->allocated = 0;
        baseF->block_size = PAGE_SZ>>4;
        printf("Header address: %p, Footer address: %p\n",sbrkRetVal,baseF);
        return baseH;
    }
    return (void*)-1;
}

int findProperList(size_t sz){
    if (sz >= LIST_1_MIN && sz <= LIST_1_MAX) return 0;
    else if (sz >= LIST_2_MIN && sz <= LIST_2_MAX) return 1;
    else if (sz >= LIST_3_MIN && sz <= LIST_3_MAX) return 2;
    else return 3;
}

sf_header* makeBlock(sf_free_header *freeH, size_t size, size_t allocSize){
    sf_header *newH;
    sf_footer *newF;
    sf_free_header *remFree;
    sf_free_header *nextF;
    sf_free_header *prevF;
    //freeH must be split
    if((freeH->header.block_size<<4) - allocSize > 32){
        size_t newFreeSize = (freeH->header.block_size<<4) - allocSize;
        sf_footer *newFreeF;
        newH = (sf_header*)freeH;
        newH->allocated = 1;
        newH->block_size = allocSize>>4;
        newF = (sf_footer*)((char*)newH+(allocSize)-8);
        newF->allocated = 1;
        newF->block_size = allocSize>>4;
        newF->requested_size = size;
        //now fix the free block.
        nextF = freeH->next;
        prevF = freeH->prev;
        //sets the links
        if(prevF != NULL)
            prevF->next = nextF;
        if(nextF != NULL)
            nextF->prev = prevF;
        freeH = (sf_free_header*)((char*)newH+(allocSize));
        freeH->header.allocated = 0;
        freeH->header.block_size = newFreeSize>>4;
        newFreeF = (sf_footer*)(((char*)freeH+(newFreeSize))-8);
        newFreeF->allocated = 0;
        newFreeF->block_size = newFreeSize>>4;
        remFree = freeH;
        seg_free_list[findProperList(remFree->header.block_size<<4)].head = remFree;

    }
    //freeH must not be split
    else{
        //we must add this fragment to our allocsize...
        if(freeH->next == NULL && freeH->prev == NULL){
            //We're at the head of the list.
            seg_free_list[findProperList((freeH->header.block_size)<<4)].head = NULL;
        }
        allocSize += ((freeH->header.block_size<<4) - allocSize); //this puts the remaining size into the size.
        newH = (sf_header*)freeH;
        newH->allocated = 1;
        newH->block_size =allocSize;
        newF = (sf_footer*)(((char*)newH+(allocSize<<4))-8);
        newF->allocated = 1;
        newF->block_size = allocSize;
        newF->requested_size = size;
    }
    if(size != allocSize){
        newH->padded = 1;
        newF->padded = 1;
    }
    else{
        newH->padded = 0;
        newF->padded = 0;
    }
    return newH;
}
void *sf_malloc(size_t size) {
    sf_header *newH;
    size_t hfSize = size+16;
    size_t allocSize = (hfSize%16==0?hfSize:hfSize+(16-hfSize%16));
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
    newH = makeBlock(nextFree, size, allocSize);
    }
    sf_blockprint(newH);
    sf_snapshot();
	return newH;
}

int errorTesting(sf_header *ptrH, sf_footer *ptrF){
    if(ptrH == NULL){
        abort();
    }
    if(ptrH < (sf_header*)get_heap_start() || ptrF+8 > (sf_footer*)get_heap_end()){
        abort();
    }
    if(ptrH->allocated == 0 || ptrF->allocated == 0){
        abort();
    }
    size_t hfSize = ptrF->requested_size+16;
    size_t allocSize = (hfSize%16==0?hfSize:hfSize+(16-hfSize%16));
    if(ptrF->padded == 1){
        if(ptrF->requested_size+16 == ptrF->block_size<<4){
            abort();
        }
        if(allocSize != ptrF->block_size<<4){
            abort();
        }
    }
    if(ptrF->padded == 0){
        if(ptrF->requested_size+16 != ptrF->block_size<<4){
            abort();
        }
    }
    if(ptrH->allocated != ptrF->allocated){
        abort();
    }
    if(ptrH->padded != ptrF->padded){
        abort();
    }
    return 0;
}

void *sf_realloc(void *ptr, size_t size) {
	return NULL;
}

void sf_free(void *ptr) {
    sf_header *ptrH = (sf_header*)ptr;
    sf_footer *ptrF = (sf_footer*)(((char*)ptrH)+(ptrH->block_size<<4)-8);
    sf_header *nextVal =(sf_header*)(((char*)ptrF)+8);
    sf_free_header *nextFree = NULL;
    errorTesting(ptrH,ptrF); //check for invalid pointer
    //is valid pointer
    if(nextVal < (sf_header*)get_heap_end()){
        if(nextVal->allocated == 0){
            nextFree = (sf_free_header*)nextVal; //make nextFree the next block in memory so we can coallesce
        }
    }
    if(nextFree != NULL){
        //it is free

        sf_free_header *prevF = nextFree->prev;
        sf_free_header *nextF = nextFree->next;

        //SET THE LINKS
        if(prevF != NULL)
            prevF->next = nextF;
        if(nextF != NULL)
            nextF->prev = prevF;
        if(seg_free_list[findProperList(nextFree->header.block_size<<4)].head == nextFree){
            seg_free_list[findProperList(nextFree->header.block_size<<4)].head = NULL;
        }

        ptrH->block_size = ((ptrH->block_size<<4)+(nextFree->header.block_size<<4))>>4;
        ptrF = (sf_footer*)(((char*)ptrH)+(ptrH->block_size<<4)-8);

        ptrH->allocated = 0;
        ptrH->padded = 0;
        ptrF->block_size = ptrH->block_size;

        //PUT INTO REQUIRED LIST
        sf_free_header *currH = seg_free_list[findProperList(ptrH->block_size<<4)].head;
        sf_free_header *insH = (sf_free_header*)ptrH;
        seg_free_list[findProperList(ptrH->block_size<<4)].head = (sf_free_header*)ptrH;
        insH->next = currH;
        insH->prev = NULL;
        if(currH != NULL){
            currH->prev = insH;
        }
        sf_snapshot();
    }
	return;
}

