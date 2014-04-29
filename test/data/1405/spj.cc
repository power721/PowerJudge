#include <cstdio>
#include <cstring>

FILE * fp_in;
FILE * fp_out;
FILE * fp;
int main(int argc, char * argv[])
{
	int n;
	int a, b;
	fp_in = fopen(argv[1], "r");
	//fp_out = fopen(argv[2], "r");
	//fp = fopen(argv[3], "r");
	while (~fscanf(fp_in, "%d", &n))
	{
		scanf("%d %d", &a ,&b);
		if (n != a * b)
		{
			puts("WA");
			fclose(fp_in);
			//fclose(fp_out);
			return 0;
		}
	}
	puts("AC");
	fclose(fp_in);
	//fclose(fp_out);
	//fclose(fp);
	return 0;
}