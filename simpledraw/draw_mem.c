#include "draw_mem.h"

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
