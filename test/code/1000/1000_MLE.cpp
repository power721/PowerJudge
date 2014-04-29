#include <stdio.h>
#include <string.h>
#define N 20000000

int s[N+5];
int main()
{
    int a, b;
    memset(s, 0xFF, sizeof(s));
    while (~scanf("%d %d", &a, &b))
    {
        printf("%d\n", a+b);
        s[a] = b;
    }
    int i;
    for (i=0; i<10000; ++i)
      s[N-i] = N-i;
  
    return 0;
}
