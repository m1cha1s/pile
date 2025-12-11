#ifndef _DRAW_MEM_H
#define _DRAW_MEM_H

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
#define ArrayPush(a,x) (ArrayMaybeGrow((a), 1), (a)[ArrayLen(a)] = (x), ArrayHdr(a)->len++, 0);

#endif