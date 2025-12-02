#ifndef PILE_MEM_H
#define PILE_MEM_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    size_t *mem;
    size_t end;
} Arena;

typedef struct {
    size_t cap;
    size_t len;
} ArrayHeader;

void *ArrayGrowFunc(void *arr, size_t elemSize, size_t addLen, size_t minCap);

#define ArrayHdr(a) ((ArrayHeader*)(a)-1)
#define ArrayLen(a) ((a) ? ArrayHdr(a)->len : 0)
#define ArrayCap(a) ((a) ? ArrayHdr(a)->cap : 0)
#define ArrayGrow(a,n,c) ((a)=ArrayGrowFunc((a), sizeof(*(a)), (n), (c)))
#define ArrayMaybeGrow(a,n) (!(a) || ((ArrayLen(a) + (n)) > ArrayCap(a)) ? ArrayGrow((a),(n),0) : 0)
#define ArrayPush(a,x) (ArrayMaybeGrow((a), 1), (a)[ArrayLen(a)] = (x), ArrayHdr(a)->len++, 0)

#endif //PILE_MEM_H

#ifdef MEM_IMPLEMENTATION
#undef MEM_IMPLEMENTATION

#include <stdlib.h>

void *ArrayGrowFunc(void *arr, size_t elemSize, size_t addLen, size_t minCap)
{
    size_t newCap, newLen;
    void *newArr;

    newCap = ArrayCap(arr);
    newLen = ArrayLen(arr);

    if (newCap < minCap)
    {
        newCap = minCap;
    }

    if (newCap < 8)
    {
        newCap = 8;
    }

    while (newLen + addLen > newCap)
    {
        newCap *= 2;
    }

    newArr = realloc(arr ? ArrayHdr(arr) : NULL, elemSize*newCap+sizeof(ArrayHeader));

    newArr = (ArrayHeader*)newArr+1;

    ArrayHdr(newArr)->len = newLen;
    ArrayHdr(newArr)->cap = newCap;

    return newArr;
}

#endif
