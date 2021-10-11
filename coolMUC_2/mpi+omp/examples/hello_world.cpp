#include <mpi.h>
#include <omp.h>
#include <iostream>

int main(int argc, char** argv)
{
	// Initialize the MPI environment
	int provided;
	MPI_Init_thread(NULL, NULL, MPI_THREAD_MULTIPLE, &provided);
	if (provided != MPI_THREAD_MULTIPLE)
	{
		std::cerr << "The MPI implementation does not support MPI_THREAD_MULTIPLE\n";
		return -1;
	}

	// Get the number of processes
	int nprocs;
	MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

	// Get the rank of the process
	int pid;
	MPI_Comm_rank(MPI_COMM_WORLD, &pid);

	// Print off a hello world message
	int nthreads, tid, flag;
	#pragma omp parallel private(nthreads, tid, flag)
	{
		nthreads = omp_get_num_threads();
		tid = omp_get_thread_num();

		//#pragma omp critical
		//std::cout << "Hello world from thread " << tid << " out of " << nthreads
		//	<< " in process " << pid << " out of " << nprocs << std::endl;
	}

	// Finalize the MPI environment
	MPI_Finalize();

	return 0;
}
