#define COND 0
#if COND
int a = 3;
#else
#if 1
int c = 32;
#endif
int b = 3;
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
