#include <stdio.h>
#include <mpi.h>
#include <hwloc.h>

int main(int argc, char* argv[])
{
	int rank, size;
        cpu_set_t mask;
        long num;
        int proc_num(long num);

        hwloc_topology_t topology;
        hwloc_cpuset_t cpuset;
        hwloc_obj_t obj;


        MPI_Init(&argc, &argv);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        MPI_Comm_size(MPI_COMM_WORLD, &size);

        hwloc_topology_init ( &topology);
        hwloc_topology_load ( topology);

        hwloc_bitmap_t set = hwloc_bitmap_alloc();
        hwloc_obj_t pu;
        int err;

        err = hwloc_get_proc_cpubind(topology, getpid(), set, HWLOC_CPUBIND_PROCESS);
        if (err) {
        printf ("Error Cannot find\n"), exit(1);
        }

        // Get the name of the processor
        char processor_name[MPI_MAX_PROCESSOR_NAME];
        int name_len;
        MPI_Get_processor_name(processor_name, &name_len);

        pu = hwloc_get_pu_obj_by_os_index(topology, hwloc_bitmap_first(set));
        printf ("Hello World, Processor=%s, I am %d and pid: %d coreid:%d\n",processor_name,rank,getpid(),hwloc_bitmap_first(set));

        int my_coreid = hwloc_bitmap_first(set);
        int all_coreid[size];
        hwloc_bitmap_free(set);
        hwloc_topology_destroy(topology);
        MPI_Finalize();
        return 0;
}
