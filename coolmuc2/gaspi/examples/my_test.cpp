#include <iostream>
#include <GASPI.h>
#include <omp.h>

int main(int argc, char *argv[])
{
	// Get the default configuration of the execution environment
	gaspi_config_t config;
	gaspi_config_get(&config);

	// Initialize the execution environment
	gaspi_proc_init(GASPI_BLOCK);

	// Get the rank of the calling process
	gaspi_rank_t iProc;
	gaspi_proc_rank(&iProc);

	// Get the number of processes
	gaspi_rank_t nProc;
	gaspi_proc_num(&nProc);

	/*int nthreads, tid;
	#pragma omp parallel private(nthreads, tid)
	{
		nthreads = omp_get_num_threads();
		tid = omp_get_thread_num();

		#pragma omp critical
		std::cout << "Hello world from thread " << tid << " out of " << nthreads
			<< " in process " << iProc << " out of " << nProc << std::endl;
	}*/

	// Terminate the execution environment
	gaspi_proc_term(GASPI_BLOCK);

	return 0;
}
