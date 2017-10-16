/**
 * All functions you make for the assignment must be implemented in this file.
 * Do not submit your assignment with a main function in this file.
 * If you submit with a main function in this file, you will get a zero.
 */
#include "sfmm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

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

int findProperList(size_t sz){
    if (sz >= LIST_1_MIN && sz <= LIST_1_MAX) return 0;
    else if (sz >= LIST_2_MIN && sz <= LIST_2_MAX) return 1;
    else if (sz >= LIST_3_MIN && sz <= LIST_3_MAX) return 2;
    else return 3;
}

sf_free_header* findFirstFit(size_t size){
    for(int listNum = 0; listNum < 4; listNum++){ //cycle through the lists from the lowest possible list size
        sf_free_header *checkH = seg_free_list[listNum].head;
        while(checkH != NULL){
            if(checkH->header.block_size<<4 >= size){ //if the size block is the first one we find bigger than the requested size...
                return checkH; //return this block
            }
            checkH = checkH->next;
        }
        //This means there was no room in this listNum, so we go to the next list...
    }
    return (NULL); //return -1, we did not find any block!
}

sf_free_header* coallesce(sf_header* leftPtr, sf_header* rightPtr, int blockIsFree){
    sf_free_header *nextFree = NULL;
    sf_header *leftH;
    sf_footer *rightF;

    leftH = leftPtr;
    //To coallesce, we will take leftPtr's head and put into it the block_size of leftPtr and rightPtr.
    //We will then change the corresponding header/footer values as required, and remove the proper free one from its list.
    if(blockIsFree == 0){
        //coallescing down (AKA malloc's sbrk)
        nextFree = (sf_free_header*)leftPtr;
    }
    else{
        //coallescing up (AKA free)
        nextFree = (sf_free_header*)rightPtr;
    }
    sf_free_header *prevF = nextFree->prev;
    sf_free_header *nextF = nextFree->next;

    //SET THE LINKS
    if(prevF != NULL)
        prevF->next = nextF;
    if(nextF != NULL)
        nextF->prev = prevF;
    if(seg_free_list[findProperList(nextFree->header.block_size<<4)].head == nextFree){
        if(nextF != NULL)
            seg_free_list[findProperList(nextFree->header.block_size<<4)].head = nextF;
        else
            seg_free_list[findProperList(nextFree->header.block_size<<4)].head = NULL;
    }

    leftH->block_size = ((leftH->block_size<<4)+(rightPtr->block_size<<4))>>4;
    rightF = (sf_footer*)(((char*)leftH)+(leftH->block_size<<4)-8);
    rightF->allocated = 0;
    rightF->padded = 0;
    rightF->block_size = leftH->block_size;
    leftH->allocated = 0;
    leftH->padded = 0;

    //PUT INTO PROPER LIST
    sf_free_header *currH = seg_free_list[findProperList(leftH->block_size<<4)].head;
    sf_free_header *insH = (sf_free_header*)leftH;
    seg_free_list[findProperList(leftH->block_size<<4)].head = (sf_free_header*)leftH;
    insH->next = currH;
    insH->prev = NULL;
    if(currH != NULL){
        currH->prev = insH;
    }
    return nextFree;
}

sf_free_header* initPage(){
    sf_header *sbrkRetVal;
    sf_header *freePutVal;
    if((sbrkRetVal = sf_sbrk()) == (void*)-1){
        return NULL;
    } //set the return value for the heap as a header
    //attempt a coallesce on sbrkRetVal:
    freePutVal = sbrkRetVal;
    sf_footer *prevF= (sf_footer*)(((char*)sbrkRetVal)-8);
    freePutVal->block_size = PAGE_SZ>>4;
    int coll = 0;
    if(prevF > (sf_footer*)get_heap_start()){
        //block does exist
        if(prevF->allocated == 0){
            //not allocated...
            sf_header *temp = (sf_header*)(((char*)prevF)-(prevF->block_size<<4)+8);
            freePutVal = (sf_header*)coallesce(temp,freePutVal,0);
            coll = 1;
        }
    }
    freePutVal->allocated = 0;
    // sbrkRetVal = header for free header
    //baseH = free header
    //baseF = footer
    sf_free_header *baseH = (sf_free_header*)freePutVal;
    sf_free_header *nextF;
    nextF = seg_free_list[findProperList(freePutVal->block_size<<4)].head;
    //sets the links
    if(coll != 1){
        if(nextF != NULL)
            baseH->next = nextF;
        seg_free_list[findProperList(freePutVal->block_size<<4)].head = baseH;
        sf_footer *baseF = (sf_footer*)((((char*)get_heap_start())+(freePutVal->block_size<<4))-8);
        baseF->allocated = 0;
        baseF->block_size = PAGE_SZ>>4;
    }
    return baseH;;
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
        seg_free_list[findProperList(freeH->header.block_size<<4)].head = NULL;
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
        nextF = freeH->next;
        prevF = freeH->prev;
        //sets the links
        if(prevF != NULL)
            prevF->next = nextF;
        if(nextF != NULL)
            nextF->prev = prevF;
        allocSize += ((freeH->header.block_size<<4) - allocSize); //this puts the remaining size into the size.
        newH = (sf_header*)freeH;
        newH->allocated = 1;
        newH->block_size =allocSize>>4;
        newF = (sf_footer*)(((char*)newH+(allocSize))-8);
        newF->allocated = 1;
        newF->block_size = allocSize>>4;
        newF->requested_size = size;
    }
    if(size+16 != allocSize){
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
    /*FOR TESTING//
    */           //
    // initPage();/
    // initPage();/
    /*            /
    //FOR TESTING*/
    if(size > PAGE_SZ*4){
        sf_errno = EINVAL;
        return NULL;
    }
    if(size == 0 || size == PAGE_SZ*4){
        sf_errno = ENOMEM;
        return NULL;

    }
    sf_header *newH;
    size_t hfSize = size+16;
    size_t allocSize = (hfSize%16==0?hfSize:hfSize+(16-hfSize%16));
    //check for if there is room in free lists
    sf_free_header *nextFree;
    while((nextFree = findFirstFit(allocSize)) == NULL){
        //NO FIRST FIT
        if((nextFree = initPage()) == NULL){
            //COULD NOT CREATE PAGE
            sf_errno = ENOMEM;
            return NULL;
        }
    }
    //NEXTFREE
    newH = makeBlock(nextFree, size, allocSize);
	return (char*)newH +8;
}

int errorTesting(sf_header *ptrH, sf_footer *ptrF){
    if(ptrH == NULL){
        abort();
    }
    if(ptrH < (sf_header*)get_heap_start() || (sf_footer*)((char*)ptrF+8) > (sf_footer*)get_heap_end()){
        abort();
    }
    if(ptrH->allocated == 0 || ptrF->allocated == 0){
        abort();
    }
    // size_t hfSize = ptrF->requested_size+16;
    // size_t allocSize = (hfSize%16==0?hfSize:hfSize+(16-hfSize%16));
    if(ptrF->padded == 1){
        if(ptrF->requested_size+16 == ptrF->block_size<<4){
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
    sf_header *ptrH = (sf_header*)((char*)ptr-8);
    sf_footer *ptrF = (sf_footer*)(((char*)ptrH)+(ptrH->block_size<<4)-8);
    sf_footer *returnPtr;
    size_t allocSize = (size%16==0?size:size+(16-size%16));
    size_t avSpace = (ptrH->block_size<<4)-16; //the amount of space in the payload + padding
    sf_free_header *nextFree;
    while((nextFree = findFirstFit(allocSize)) == NULL){
        //NO FIRST FIT
        if((nextFree = initPage()) == NULL){
            //COULD NOT CREATE PAGE
            sf_errno = ENOMEM;
            return NULL;
        }
    }
    errorTesting(ptrH,ptrF);
    //if size is 0, free it.
    if(size == 0){
        sf_free(ptr);
        return NULL;
    }
    //IF LARGER
    if(allocSize >= ptrF->requested_size){

            if(allocSize <= avSpace){
                //WE CAN FIT ALLOC SIZE INTO REQUESTED SIZE!!!
                ptrF->requested_size = size;
                if((ptrF->requested_size)+ 16 != ptrF->block_size<<4){
                    ptrH->padded = 1;
                    ptrF->padded = 1;
                }
                else{
                    ptrF->padded = 0;
                    ptrH->padded = 0;
                }
                return (char*)ptrH +8;
            }

        //MUST CREATE A NEW BLOCK
        sf_header *moveToLoc;
        if((moveToLoc = sf_malloc(size)) == NULL){ //malloc block of size requested
            return NULL;
        }
        moveToLoc = memcpy(((char*)moveToLoc),((char*)ptrH)+8,ptrF->requested_size); //copy memory from payload to payload
        moveToLoc = (sf_header*)(((char*)moveToLoc)-8); //moveToLoc is header
        //change necessary bits
        returnPtr = (sf_footer*)(((char*)moveToLoc)+(moveToLoc->block_size<<4)-8); //returnPtr is footer
        returnPtr->requested_size = size;
        if((returnPtr->requested_size)+16 != returnPtr->block_size<<4){
            moveToLoc->padded = 1;
            returnPtr->padded = 1;
        }
        else{
            moveToLoc->padded = 0;
            returnPtr->padded = 0;
        }
        //NOW FREE THE OTHER BLOCK
        sf_free((char*)ptrH+8);
        return (char*)moveToLoc+8;
    }
    //IF SMALLER
    else{
        //allocSize < ptrF->requested_size
        if(avSpace - allocSize < 32){
            //SPLINTERRRRRRR
            ptrF->requested_size = size;
            if((ptrF->requested_size)+ 16 != ptrF->block_size<<4){
                ptrH->padded = 1;
                ptrF->padded = 1;
            }
            else{
                ptrF->padded = 0;
                ptrH->padded = 0;
            }
            return (char*)ptrH +8;
        }

        else{
            //NOT SPLINTER//
            allocSize += 16;
            sf_footer *allocF = (sf_footer*)(((char*)ptrH)+allocSize-8); //footer of block 1
            //DO ALLOC BLOCK STUFF FIRST
            //HEADER: ptrH
            //FOOTER: allocF
            ptrH->block_size = allocSize>>4;
            allocF->block_size = allocSize>>4;
            allocF->requested_size = size;
            if((allocF->requested_size)+ 16 != allocF->block_size<<4){
                ptrH->padded = 1;
                allocF->padded = 1;
            }
            else{
                allocF->padded = 0;
                ptrH->padded = 0;
            }
            allocF->allocated = 1;

            //DO FREE BLOCK STUFF NOW
            //HEADER: freeH
            //FOOTER: ptrF
            size_t allocSize2 = ((ptrF->block_size<<4)-(ptrH->block_size<<4));
            sf_header *freeH = (sf_header*)(((char*)allocF)+8); //header of block 2
            freeH->allocated = 1;
            freeH->block_size = allocSize2>>4;
            freeH->padded = 1;
            ptrF->block_size = allocSize2>>4;
            ptrF->padded = 1;
            ptrF->requested_size = 0;
            sf_free((char*)freeH+8);
            return (char*)ptrH + 8;
        }


    }

	return NULL;
}

void sf_free(void *ptr) {
    sf_header *ptrH = (sf_header*)((char*)ptr-8);
    sf_footer *ptrF = (sf_footer*)(((char*)ptrH)+(ptrH->block_size<<4)-8);
    sf_header *nextVal =(sf_header*)(((char*)ptrF)+8);
    //sf_free_header *nextFree = NULL;
    // sf_free_header *collPtr = NULL;
    errorTesting(ptrH,ptrF); //check for invalid pointer
    //is valid pointer
    if(nextVal <= (sf_header*)get_heap_end()){
        //IT IS INSIDE THE HEAP
        if(nextVal< (sf_header*)get_heap_end() && nextVal->allocated == 0){
            //IT IS FREE
            //*******nextFree = (sf_free_header*)nextVal; //make nextFree the next block in memory so we can coallesce
            //*******collPtr = coallesce(ptrH,nextVal,1);
            coallesce(ptrH,nextVal,1);
        }
        else{
            //IT IS NOT FREE, JUST SET PTR VALUES AND ADD PTR TO A LIST
            ptrH->allocated = 0;
            ptrH->padded = 0;
            ptrF->allocated = 0;
            ptrF->padded = 0;
            ptrF->requested_size = 0;
            sf_free_header *currH = seg_free_list[findProperList(ptrH->block_size<<4)].head;
            sf_free_header *insH = (sf_free_header*)ptrH;
            seg_free_list[findProperList(ptrH->block_size<<4)].head = (sf_free_header*)ptrH;
            insH->next = currH;
            insH->prev = NULL;
            if(currH != NULL){
                currH->prev = insH;
            }
        }
    }
	return;
}

