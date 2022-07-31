#include <iostream>	// std::cout...
#include <cstdint>	// uint64_t...
#include <vector>	// std::vector...
#include <cmath>	// exp2l...
#include <mpi.h>

int main()
{
	MPI_Init(NULL, NULL);

	int my_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	MPI_Win		win;
	void*		base_ptr;
	MPI_Win_allocate(1024, 1, MPI_INFO_NULL, MPI_COMM_WORLD, &base_ptr, &win);

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Win_lock_all(0, win);

	int i, j;
	MPI_Aint disp = 0;
	if (my_rank == 1)
	{
		int* tmp_ptr = (int*)base_ptr;
		for (i = 0; i < 10; ++i)
		{
			for (j = 0; j < exp2l(6); ++j)
			{
				MPI_Put(&j, sizeof(int), MPI_CHAR, 0, disp, sizeof(int), MPI_CHAR, win);
				disp += sizeof(int);
			}
			MPI_Put(&j, sizeof(int), MPI_CHAR, 0, disp, sizeof(int), MPI_CHAR, win);
			MPI_Win_flush(0, win);
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Win_unlock_all(win);

	MPI_Win_free(&win);

	MPI_Finalize();

	return 0;
}
