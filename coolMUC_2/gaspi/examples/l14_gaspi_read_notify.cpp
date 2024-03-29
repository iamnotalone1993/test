// Pipelined read and processing of data
// The pipeline consists of the following two stages
// 1. Read remote data with a predefined number of chunks
// 2. Perform multithreaded waitsome, subsequent processing of
//    the data chunks, and a consecutive read_notify in order to
//    sustain the pipeline

#include <GASPI.h>
#include <success_or_die.h>

extern void process(gaspi_segment_id_t 			segment_id_local,
			gaspi_offset_t			offset_local,
			gaspi_size_t			size,
			gaspi_notification_id_t		id);

// Note: For sake of simplicity we have omitted checking
// 	 the number of used chunks vs. the actually available
// 	 notification resources as well as properly checking the
// 	 queue status. (see e.g. example for gaspi_wait,
// 	 wait_if_queue_full())

void pipelined_read_and_process(int			num_chunks,
				gaspi_segment_id_t	segment_id_local,
				gaspi_offset_t		offset_local,
				gaspi_rank_t		rank,
				gaspi_segment_id_t	segment_id_remote,
				gaspi_offset_t		offset_remote,
				gaspi_size_t		chunk_size,
				gaspi_queue_id_t	queue_id)
{
	const int nthreads = omp_get_max_threads();
	const int num_initial_chunks = nthreads * 4;
	int i;

	// Start GASPI accumulate pipeline
	for (i = 0; i < num_initial_chunks; ++i)
	{
		ASSERT(gaspi_read_notify(segment_id_local,
					(offset_local + i * chunk_size),
					rank,
					segment_id_remote,
					(offset_remote + i * chunk_size),
					chunk_size,
					i,
					queue_id,
					GASPI_BLOCK));
	}

	#pragma omp parallel
	{
		int const tid = omp_get_thread_num();

		// For sake of simplicity we use notifications
		// which are exclusive per thread

		gaspi_notification_id_t id, first = tid;
		gaspi_notification_id_t next = first + num_initial_chunks;

		while (first < num_chunks)
		{
			ASSERT(gaspi_notify_waitsome(segment_id_local, first, i, &id, GASPI_BLOCK));

			gaspi_notification_t val = 0;
			ASSERT(gaspi_notify_reset(segment_id_local, id, &val));

			// process received data chunk
			process(segment_id_local, (offset_local + id * chunk_size), chunk_size, id);

			first += nthreads;
			next += ntrheads;

			if (next < num_chunks)
			{
				// start next read, sustain pipeline.
				ASSERT(gaspi_read_notify(segment_id_local,
							(offset_local + next * chunk_size),
							rank,
							segment_id_remote,
							(offset_remote + next * chunk_size),
							chunk_size,
							next,
							queue_id,
							GASPI_BLOCK));
			}
		}
	}
}
