#include <typedefs.h>
#include <dfile.h>
#include <bx.h>

int main(int argc, char *argv[])
{
	int n = sizeof(BX) / sizeof(dtype);
	std::vector<dtype> bx(&BX[0], &BX[n]);
	dfile bfp(argv[1], false);
	if (bfp)
		bfp.put(bx);
	else {
		puts("Usage:\n\t createbx bx.dat\n");
	}
	return 0;
}
