/* file: memory.c */
#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
int main (int argc, char **argv)
{
	int i=31415;
	printf ("#i=%d\n", i);
	#pragma omp parallel private(i)
	{
		printf (" i=%d\n", i);
		i = 99;
	}
	printf ("#i=%d\n", i);
	#pragma omp parallel firstprivate(i)
	{
		printf (" i=%d\n", i);
		i = 99;
	}
	printf ("#i=%d\n", i);
	#pragma omp parallel shared(i)
	{
		#pragma omp atomic
		i++;		
		printf (" i=%d\n", i);
	}
	printf ("#i=%d\n", i);
	return EXIT_SUCCESS;
}

