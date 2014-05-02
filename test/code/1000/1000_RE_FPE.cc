#include <stdio.h>

int main()
{
    int a, b;
    char *s;
    while (~scanf("%d %d", &a, &b))
    {
      printf("%d\n", a+b);
    }
    printf("%d\n", a/0);
    return 0;
}
