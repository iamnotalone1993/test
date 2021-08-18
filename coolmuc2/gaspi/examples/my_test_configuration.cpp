#include <iostream>
#include <GASPI.h>

int main(int argc, char *argv[])
{
	// Retrieve the current version of GASPI
	float version;
	gaspi_version(&version);

	// Retrieve the proposed configuration of the execution environment
	gaspi_config_t config;
	gaspi_config_get(&config);

	// Initialize the execution environment
	gaspi_proc_init(GASPI_BLOCK);

	// Retrieve the actual configuration of the execution environment
	gaspi_number_t		group_max,
				segment_max,
				queue_num,
				queue_size_max,
				queue_max,
				notification_num,
				elem_max,
				build_infrastructure;

	gaspi_size_t		transfer_size_max,
				passive_transfer_size_max,
				buf_size;

	gaspi_atomic_value_t	max_value;

	gaspi_network_t		network_type;

	gaspi_group_max(&group_max);
	gaspi_segment_max(&segment_max);
	gaspi_queue_num(&queue_num);
	gaspi_queue_size_max(&queue_size_max);
	gaspi_queue_max(&queue_max);
	gaspi_transfer_size_max(&transfer_size_max);
	gaspi_notification_num(&notification_num);
	gaspi_passive_transfer_size_max(&passive_transfer_size_max);
	gaspi_atomic_max(&max_value);
	gaspi_allreduce_buf_size(&buf_size);
	gaspi_allreduce_elem_max(&elem_max);
	gaspi_network_type(&network_type);
	//gaspi_build_infrastructure(&build_infrastructure);

	// Get the rank of the calling process
	gaspi_rank_t iProc;
	gaspi_proc_rank(&iProc);

	// Get the number of processes
	gaspi_rank_t nProc;
	gaspi_proc_num(&nProc);

	// Print off the current version of GASPI, the proposed and actual configurations
        std::cout << "[" << iProc << "]GASPI_version = " << version << std::endl 
		<< "PROPOSED configuration\n"
                << "group_max = " << config.group_max << std::endl
                << "segment_max = " << config.segment_max << std::endl
                << "queue_num = " << config.queue_num << std::endl
                << "queue_size_max = " << config.queue_size_max << std::endl
                << "transfer_size_max = " << config.transfer_size_max << std::endl
                << "notification_num = " << config.notification_num << std::endl
		<< "passive_queue_size_max = " << config.passive_queue_size_max << std::endl
                << "passive_transfer_size_max = " << config.passive_transfer_size_max << std::endl
                << "allreduce_buf_size = " << config.allreduce_buf_size << std::endl
                << "allreduce_elem_max = " << config.allreduce_elem_max << std::endl
                << "network = " << config.network << std::endl
                << "build_infrastructure = " << config.build_infrastructure << std::endl
		<< "ACTUAL configuration\n"
		<< "group_max = " << group_max << std::endl
		<< "segment_max = " << segment_max << std::endl
		<< "queue_num = " << queue_num << std::endl
		<< "queue_size_max = " << queue_size_max << std::endl
		<< "queue_max = " << queue_max << std::endl
		<< "transfer_size_max = " << transfer_size_max << std::endl
		<< "notification_num = " << notification_num << std::endl
		<< "passive_transfer_size_max = " << passive_transfer_size_max << std::endl
		<< "max_value = " << max_value << std::endl
		<< "buf_size = " << buf_size << std::endl
		<< "elem_max = " << elem_max << std::endl
		<< "network_type = " << network_type << std::endl;
		//<< "build_infrastructure = " << build_infrastructure << std::endl;

	// Terminate the execution environment
	gaspi_proc_term(GASPI_BLOCK);

	return 0;
}
