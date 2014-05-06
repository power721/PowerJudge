#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main()
{
    int a, b;
    while (~scanf("%d %d", &a, &b))
    {
      printf("%d\n", a+b);
    }
    
    if (execl("./Main", "Main", NULL) == -1) {
      fprintf(stderr, "execl: %s\n", strerror(errno));
    }
    return 0;
}
