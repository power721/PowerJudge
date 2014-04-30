#include <cstdio>

int main()
{
	int n;
	int a;
	while (~scanf("%d", &n))
	{
		for (a=2; ; ++a)
		{
			if (n % a == 0)
			{
				printf("%d %d\n", n/a, a);
				break;
			}
		}
	}
	return 0;
}