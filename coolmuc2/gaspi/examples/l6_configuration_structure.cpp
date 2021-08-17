typedef struct
{
	// maximum number of groups
	gaspi_number_t		group_max;

	// maximum number of segments
	gaspi_number_t		segment_max;
	
	// one-sided comm parameter
	gaspi_number_t		queue_num;
	gaspi_number_t		queue_size_max;
	gaspi_size_t		transfer_size_max;

	// notification parameter
	gaspi_number_t		notification_num;

	// pasive comm parameter
	gaspi_number_t		passive_queue_size_max;
	gaspi_size_t		passive_transfer_size_max;

	// collective comm parameter
	gaspi_size_t		allreduce_buf_size;
	gaspi_number_t		allreduce_elem_max;

	// network selection parameter
	gaspi_network_t		network;

	// communication infrastructure build up notification
	gaspi_number_t		build_infrastructure;

	void*			user_defined;
} gaspi_config_t;

