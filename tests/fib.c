#define COND 0
#if 0
    int foo = 3;
#else
    #if 0
        int abc = 32;
    #else
        int test = 3;
    #endif
#endif

//#include<stdio.h>
#define MAX_DEPTH 16 - 2 * 2

int fib(int a, int b, int depth);

int main()
{
    fib(1, 1, 0);
}

int fib(int a, int b, int depth)
{
    //printf("%u\n", a);
    if (depth >= MAX_DEPTH)
        return a;
    int c = a;
    a += b;
    b = c;
    fib(a, b, depth+1);
}
