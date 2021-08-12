#include <iostream>
#include <GASPI.h>
#include <omp.h>

int main(int argc, char *argv[])
{
	gaspi_proc_init(GASPI_BLOCK);

	gaspi_rank_t iProc;
	gaspi_rank_t nProc;

	gaspi_proc_rank(&iProc);
	gaspi_proc_num(&nProc);

	int nthreads, tid;

	#pragma omp parallel private(nthreads, tid)
	{
		nthreads = omp_get_num_threads();
		tid = omp_get_thread_num();

		//#pragma omp critical
		//std::cout << "Hello world from thread " << tid << " out of " << nthreads
		//	<< " in process " << iProc << " of " << nProc << std::endl;
	}

	gaspi_proc_term(GASPI_BLOCK);

	return 0;
}
