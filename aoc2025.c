#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>

#define MEM_IMPLEMENTATION
#include "mem.h"

//#include "yggdrasil.h"

#define DAY 1

uint8_t *LoadFile(char *path, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        if (size) *size = 0;
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    size_t fsize = ftell(f);
    rewind(f);

    uint8_t *contents = malloc(fsize);
    fread(contents, fsize, 1, f);

    fclose(f);

    if (size) {
        *size = fsize;
    }

    return contents;
}

typedef struct String {
    uint8_t *data;
    size_t length;
} String;
#define SFMT(s) (int)(s).length, (const char *)(s).data

typedef struct StringParser {
    String src;
    size_t current, base;
} StringParser;

void SP_Consume(StringParser *sp)
{
    sp->base = sp->current;
}

int IsWhitespace(uint8_t c)
{
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

int IsNum(uint8_t c)
{
    return c >= '0' && c <= '9';
}

String S_TrimLeft(String s)
{
    if (!s.length) return s;

    while (s.length > 0 && IsWhitespace(s.data[0]))
    {
        s.data++;
        s.length--;
    }
    return s;
}

String S_TrimRight(String s)
{
    if (!s.length) return s;

    while (s.length > 0 && IsWhitespace(s.data[s.length-1]))
    {
        s.length--;
    }
    return s;
}

String S_Trim(String s)
{
    return S_TrimLeft(S_TrimRight(s));
}

int SP_EOF(StringParser *sp)
{
    return sp->base >= sp->src.length;
}

uint8_t SP_ParseChar(StringParser *sp)
{
    if (SP_EOF(sp)) return 0;

    sp->current = sp->base;

    uint8_t c = sp->src.data[sp->current++];
    return c;
}

uint64_t SP_ParseU64(StringParser *sp, int *failed)
{
    if (SP_EOF(sp))
    {
        if (failed) *failed = 1;
        return 0;
    }

    sp->current = sp->base;

    uint64_t num = 0;
    while (sp->current < sp->src.length && IsNum(sp->src.data[sp->current]))
    {
        num *= 10;
        num += sp->src.data[sp->current++] - '0';
    }

    if (failed) *failed = 0;
    return num;
}

String SP_ParseLine(StringParser *sp)
{
    sp->current = sp->base;
    while (sp->current < sp->src.length && sp->src.data[sp->current] != '\n') ++sp->current; ++sp->current;
    String res = { .data = &sp->src.data[sp->base], .length = sp->current-sp->base-1 };
    return res;
}

#if DAY == 1

int main()  {
    size_t size;
#ifdef TEST
    uint8_t *data = LoadFile("aoc2025_1_test.txt", &size);
#else
    uint8_t *data = LoadFile("aoc2025_1.txt", &size);
#endif

    StringParser sp = {0};
    sp.src = (String){.data = data, .length = size};

    int zeroCounts = 0;
    int wraps = 0;
    int counter = 50;
    int doubleCount = 0;

    while (!SP_EOF(&sp))
    {
        uint8_t c = SP_ParseChar(&sp);
        SP_Consume(&sp);

        if (c != 'L' && c != 'R') continue;

        int num = (int)SP_ParseU64(&sp, NULL);
        SP_Consume(&sp);

        int startedAtZero = counter == 0;

        if (c == 'L')
        {
            printf("- %d\n", num);
            counter -= num;
        }
        else if (c == 'R')
        {
            printf("+ %d\n", num);
            counter += num;
        }

        int wrapped = 0;

        while (counter < 0)
        {
            printf("wrap neg\n");
            counter += 100;
            if (!startedAtZero) wraps ++;
            wrapped = 1;
        }
        while (counter > 99)
        {
            printf("wrap pos\n");
            counter -= 100;
            wraps ++;
            wrapped = 1;
        }

        if (counter == 0)
        {
            printf("zero!!\n");
            zeroCounts ++;
            if (wrapped)
            {
                printf("double!!!\n");
                doubleCount++;
            }
        }

        printf("Counter: %d\n", counter);
    }

    printf("ZeroCounts: %d\n", zeroCounts);
    printf("Wraps: %d\n", wraps);
    printf("Part2: %d\n", zeroCounts + wraps - doubleCount);
}

#endif