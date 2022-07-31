#include <iostream>	// std::cout...
#include <cstdint>	// uint64_t...
#include <vector>	// std::vector...
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

	if (my_rank == 0)
	{
		uint64_t cho = 100;
		MPI_Put(&cho, 1, MPI_BYTE, 0, 23, 1, MPI_BYTE, win);
		MPI_Win_flush(0, win);
	}

	MPI_Barrier(MPI_COMM_WORLD);

	if (my_rank == 1)
	{
		std::vector<uint64_t>	array;
		array.push_back(9);
		array.push_back(19);
		array.push_back(29);

		MPI_Datatype	MPI_REMOTE_LINKED_LIST;
		MPI_Aint	disp[2];
		disp[0]	= 0;
		disp[1] = -10;
		disp[2] = 23;
		MPI_Type_create_hindexed_block(3, 1, disp, MPI_UINT64_T, &MPI_REMOTE_LINKED_LIST);
		MPI_Type_commit(&MPI_REMOTE_LINKED_LIST);

		MPI_Put(array.data(), sizeof(array), MPI_BYTE, 0, 10, 1, MPI_REMOTE_LINKED_LIST, win);
		MPI_Win_flush(0, win);
		MPI_Type_free(&MPI_REMOTE_LINKED_LIST);

		MPI_Barrier(MPI_COMM_WORLD);

		disp[0] = 0;
		disp[1] = 20;
		disp[2] = 40;
		MPI_Type_create_hindexed_block(3, 1, disp, MPI_UINT64_T, &MPI_REMOTE_LINKED_LIST);
		MPI_Type_commit(&MPI_REMOTE_LINKED_LIST);

		MPI_Put(array.data(), sizeof(array), MPI_BYTE, 0, 100, 1, MPI_REMOTE_LINKED_LIST, win);
		MPI_Win_flush(0, win);
		MPI_Type_free(&MPI_REMOTE_LINKED_LIST);

		MPI_Barrier(MPI_COMM_WORLD);
	}

	if (my_rank == 0)
	{
		MPI_Barrier(MPI_COMM_WORLD);

		uint64_t cho;
		MPI_Get(&cho, 1, MPI_UINT64_T, 0, 10, 1, MPI_UINT64_T, win);
		MPI_Win_flush(0, win);
		std::cout << cho << std::endl;
		MPI_Get(&cho, 1, MPI_UINT64_T, 0, 0, 1, MPI_UINT64_T, win);
		MPI_Win_flush(0, win);
		std::cout << cho << std::endl;
		MPI_Get(&cho, 1, MPI_UINT64_T, 0, 23, 1, MPI_UINT64_T, win);
		MPI_Win_flush(0, win);
		std::cout << cho << std::endl;
		MPI_Get(&cho, 1, MPI_UINT64_T, 0, 33, 1, MPI_UINT64_T, win);
		MPI_Win_flush(0, win);
		std::cout << cho << std::endl;

		std::cout << "PHASE II" << std::endl;
		MPI_Barrier(MPI_COMM_WORLD);

		MPI_Get(&cho, 1, MPI_UINT64_T, 0, 100, 1, MPI_UINT64_T, win);
		MPI_Win_flush(0, win);
		std::cout << cho << std::endl;
		MPI_Get(&cho, 1, MPI_UINT64_T, 0, 120, 1, MPI_UINT64_T, win);
		MPI_Win_flush(0, win);
		std::cout << cho << std::endl;
		MPI_Get(&cho, 1, MPI_UINT64_T, 0, 140, 1, MPI_UINT64_T, win);
		MPI_Win_flush(0, win);
		std::cout << cho << std::endl;
	}

	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Win_unlock_all(win);

	MPI_Win_free(&win);

	MPI_Finalize();

	return 0;
}
