#ifndef FL
#define FL __FI##LE__
#endif

#include<algorithm>
#include<iostream>
#include FL
#ifndef _MAIN_
#define _MAIN_
int main()
{
    int a, b;
    while (cin>>a>>b)
    {
        cout<<a+b<<endl;
    }
    return 0;
}
#endif
#include FL

