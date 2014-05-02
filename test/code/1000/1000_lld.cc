#include <iostream>
#include <cstdio>
#include <string>
using namespace std;

int main()
{
    long long a, b;
    while (~scanf("%lld %lld", &a, &b))
    {
        printf("%lld\n", a+b);
    }
    return 0;
}
