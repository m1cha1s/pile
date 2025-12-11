#include <stdio.h>

int main() 
{
    for (char a = 'A'; a <= 'Z'; ++a)
    {
        printf("KEY_%c,\n", a);
    }
    for (char a = 'A'; a <= 'Z'; ++a)
    {
        printf("case kVK_ANSI_%c: return KEY_%c;\n", a, a);
    }
}