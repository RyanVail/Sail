(1 + (2 - 2)) / 3

#define COND 0
#if 0
#else
    #if 1
        int abc = 32;
    #else
        int test = 3;
    #endif
#endif

int a = 3;

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
