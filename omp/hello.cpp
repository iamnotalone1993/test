#include <omp.h>
#include <iostream>

int main(int argc, char **argv)
{
	int nthreads, tid;

	#pragma omp parallel private(nthreads, tid)
	{
		nthreads = omp_get_num_threads();
		tid = omp_get_thread_num();

		#pragma omp critical
		std::cout << "Hello world from " << tid << " out of " << nthreads << std::endl;
	}

	return 0;
}
