#include <iostream>
#include <cstdio>
#include <string>
using namespace std;

int main()
{
    __int64 a, b;
    while (~scanf("%I64d %I64d", &a, &b))
    {
        printf("%I64d\n", a+b);
    }
    return 0;
}
