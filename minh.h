struct test
{
	intptr_t		tgt_entry_ptr;
	int32_t			idx_image;
	ptrdiff_t		entry_image_offset;
	//TYPE_TASK_ID		task_id;
	int32_t			arg_num;
	std::vector<void *>	arg_hst_pointers;
	std::vector<int64_t>	arg_sizes;
	std::vector<int64_t>	arg_types;
	std::vector<void *>	arg_tgt_pointers;
	std::vector<ptrdiff_t>	arg_tgt_offsets;
	int32_t			is_remote_task;
	int32_t			is_manual_task;
	int32_t			is_replicated_task;
	int32_t			is_migrated_task;
	int32_t			is_canceled;
};

struct dang
{
	int a;
	float b;
};
