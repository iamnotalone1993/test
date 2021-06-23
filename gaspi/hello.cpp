#include <iostream>
#include <GASPI.h>

int main(int argc, char *argv[])
{
	gaspi_proc_init(GASPI_BLOCK);

	gaspi_rank_t iProc;
	gaspi_rank_t nProc;

	gaspi_proc_rank(&iProc);
	gaspi_proc_num(&nProc);

	std::cout << "Hello world from rank " << iProc << " of " << nProc << std::endl;

	gaspi_proc_term(GASPI_BLOCK);

	return 0;
}
