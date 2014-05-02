#include <stdio.h>
#include </dev/random>

// compilation will block until judge timeout
int main()
{
    int a, b;
    while (~scanf("%d %d", &a, &b))
    {
        printf("%d\n", a+b);
    }
    return 0;
}
