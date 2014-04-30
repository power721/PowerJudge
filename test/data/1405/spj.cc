#include <cstdio>
#include <cstring>

#define SPJ_AC    0
#define SPJ_PE    1
#define SPJ_WA    2

FILE * fp_in;
FILE * fp_out;
FILE * fp;
int main(int argc, char * argv[])
{
	int n;
	int a, b;
	fp_in = fopen(argv[1], "r");
	//fp_out = fopen(argv[2], "r");
	fp = fopen(argv[3], "r");
	while (~fscanf(fp_in, "%d", &n))
	{
		fscanf(fp, "%d %d", &a ,&b);
		if (n != a * b)
		{
			fclose(fp_in);
			//fclose(fp_out);
			fclose(fp);
			return SPJ_WA;
		}
	}
	fclose(fp_in);
	//fclose(fp_out);
	fclose(fp);
	return SPJ_AC;
}