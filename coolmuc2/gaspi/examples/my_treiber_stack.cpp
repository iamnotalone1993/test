#include <GASPI.h>

template <typename T>
inline void lwrite(const T* src, const GlobalPtr<T>& dst, const size_t& size)
{
	std::memcpy(dst.local(), src, size * sizeof(T));
	// dst.local() can be computed from BCL_BASE_PTR
}

template <typename T>
inline void rwrite_sync(const T* src, const GlobalPtr<T>& dst, const size_t& size)
{
	gaspi_write(BCL_SEGMENT_ID, src - BCL_BASE_PTR,
			dst.rank, BCL_SEGMENT_ID, dst.ptr,
			size * sizeof(T), BCL_QUEUE, GASPI_BLOCK);
}

template <typename T>
inline void rwrite_async(const T* src, const GlobalPtr<T>& dst, const size_t& size)
{
	gaspi_write(BCL_SEGMENT_ID, src - BCL_BASE_PTR,
			dst.rank, BCL_SEGMENT_ID, dst.ptr,
			size * sizeo(T), BCL_QUEUE, GASPI_TEST);
	// BLOCK
	// gaspi_wait(BCL_QUEUE, GASPI_BLOCK);
}

template <typename T>
inline void awrite_sync(const T* src, const GlobalPtr<T>& dst, const size_t& size)
{
	gaspi_atomic_value_t old_val,
			     result;

	do {
		gaspi_atomic_fetch_add(GASPI_SEGMENT_ID, dst.ptr, dst.rank,
					gaspi_atomic_value_t(0), old_val, GASPI_BLOCK);
		gaspi_atomic_compare_swap(BCL_SEGMENT_ID, dst.ptr, dst.rank,
					&old_val, *src, &result, GASPI_BLOCK);
	} while (result != old_val)

}

template <typename T>
inline void awrite_async(const T* src, const GlobalPtr<T>& dst, const size_t& size)
{
	// awrite_sync();
}

template <typename T>
inline void lread(const GlobalPtr<T>& src, T* dst, const size_t& size)
{
        std::memcpy(dst, src.local(), size * sizeof(T));
        // src.local() can be computed from BCL_BASE_PTR
}

template <typename T>
inline void rread_sync(const GlobalPtr<T>& src, T* dst, const size_t& size)
{
	gaspi_read(BCL_SEGMENT_ID, dst - BCL_BASE_PTR,
			src.rank, BCL_SEGMENT_ID, src.ptr,
			size * sizeof(T), BCL_QUEUE, GASPI_BLOCK);
}

template <typename T>
inline void rread_async(const GlobalPtr<T>& src, T* dst, const size_t& size)
{
	gaspi_read(BCL_SEGMENT_ID, dst - BCL_BASE_PTR,
			src.rank, BCL_SEGMENT_ID, src.ptr,
			size * sizeof(T), BCL_QUEUE, GASPI_TEST);
	// BLOCK
	// gaspi_wait(BCL_QUEUE, GASPI_BLOCK);
}

template <typename T>
inline void aread_sync(const GlobalPtr<T>& src, T* dst, const size_t& size)
{
	gaspi_atomic_fetch_add(GASPI_SEGMENT_ID, src.ptr, src.rank, gaspi_atomic_value_t(0), dst, GASPI_BLOCK); 
}

template <typename T>
inline void aread_async(const GlobalPtr<T>& src, T* dst, const size_t& size)
{
        gaspi_atomic_fetch_add(GASPI_SEGMENT_ID, src.ptr, src.rank, gaspi_atomic_value_t(0), dst, GASPI_TEST);
}

template <typename T, typename U>
inline void fetch_and_op_sync(const GlobalPtr<T> &dst, const T* val, const atomic_op<U>& op, T* result)
{
	// Can be implemented using CAS
}

template <typename T>
inline void compare_and_swap_sync(const GlobalPtr<T>& dst, const T* old_val, const T* new_val, T* result)
{
	gaspi_atomic_compare_swap(BCL_SEGMENT_ID, dst.ptr, dst.rank, *old_val, *new_val, result, GASPI_BLOCK);
}

int main(int argc, char* argv[])
{
	// Initialize execution environment
	gaspi_proc_init(GASPI_BLOCK);

	// Retrieve the number of processes
	gaspi_rank_t num_procs;
	gaspi_proc_num(&num_procs);
	
	// Retrieve the rank of the calling process
	gaspi_rank_t my_rank;
	gaspi_proc_rank(&my_rank);

	// Commit GASPI_GROUP_ALL
	gaspi_group_commit(GASPI_GROUP_ALL, GASPI_BLOCK);

	// Create shared segments
	const gaspi_segment_id_t	BCL_SEGMENT_ID 		= 0;
	const gaspi_size_t		BCL_SEGMENT_SIZE 	= 1024 * 1024;
	gaspi_pointer_t			BCL_BASE_PTR;
	gaspi_segment_create(BCL_SEGMENT_ID, BCL_SEGMENT_SIZE,
				GASPI_GROUP_ALL, GASPI_BLOCK, GASPI_ALLOC_DEFAULT);
	gaspi_segment_ptr(BCL_SEGMENT_ID, &BCL_BASE_PTR);

	// Create a communication queue
	gaspi_queue_id_t BCL_QUEUE;
	gaspi_queue_create(&BCL_QUEUE, GASPI_BLOCK);

	// Working phase

	// Delete the communication queue
	gaspi_queue_delete(BCL_QUEUE);

	// Delete the shared segments
	gaspi_segment_delete(BCL_SEGMENT_ID);

	// Terminate execution environment
	gaspi_proc_term(GASPI_BLOCK);

	return 0;
}
