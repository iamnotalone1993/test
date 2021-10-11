#include <mpi.h>
#include <iostream>

int main(int argc, char** argv) {
	// Initialize the MPI environment
	MPI_Init(NULL, NULL);

	// Get the rank of the process
	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

        MPI_Comm        nodeComm;
        MPI_Group       group,
                        nodeGroup;
	int     	size;
	int		*table;
        int             *temp;

        MPI_Comm_split_type(MPI_COMM_WORLD, MPI_COMM_TYPE_SHARED, rank, MPI_INFO_NULL, &nodeComm);
        MPI_Comm_size(nodeComm, &size);
        table = new int [size];
        temp = new int [size];
        for (int i = 0; i < size; ++i)
                temp[i] = i;
        MPI_Comm_group(MPI_COMM_WORLD, &group);
        MPI_Comm_group(nodeComm, &nodeGroup);
        MPI_Group_translate_ranks(nodeGroup, size, temp, group, table);
        delete[] temp;

	// Print off a hello world message
	std::cout << "Hello world from process " << rank << " out of " << size << " processes!" << std::endl;

	// Finalize the MPI environment.
	MPI_Finalize();

	return 0;
}

