#ifndef BASE_H
#define BASE_H

#include <stdint.h> // Would like this to be optional
#include <stddef.h>

typedef int8_t   s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t   u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;
typedef ptrdiff_t ssize;

#ifndef true
#define true (1)
#endif

#ifdef false
#define false (0)
#endif

typedef u8   b8;
typedef u16 b16;
typedef u32 b32;
typedef u64 b64;

typedef float f32;
typedef double f64;

typedef struct String {
    u8 *data;
    usize length;
} String;
#define SFMT(s) (int)(s).length, (const char *)(s).data

typedef struct StringParser {
    String src;
    usize current;
    usize base;
} StringParser;

typedef struct StringList {
    String str;
    struct StringList *next, *prev;
} StringList;

#endif //BASE_H
