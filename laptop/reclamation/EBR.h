#ifndef EBR_H
#define EBR_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...
#include <limits>       // std::numerics_limits...

class thread_context;

class mem_manager
{
public:	
	mem_manager(const uint64_t& num_threads, const uint64_t& num_hps, const uint64_t& epoch_freq);
	~mem_manager();
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t&	tid,		// num indicates the maximum number of
				const int&	num);		// locations the caller can reserve
	void unregister_thread();	// called once, after the last call to op_end()

	void op_begin();	// indicate the beginning of a concurrent operation
	void op_end();		// indicate the end of a concurrent operation

	bool try_reserve(void* ptr, void* comp);// try to protect a pointer from reclamation
	void unreserve(void* ptr);		// stop protecting a pointer
	void sched_for_reclaim(void* ptr);	// try to reclaim a pointer

private:
	const uint64_t				MAX = std::numeric_limits<uint64_t>::max();

	thread_local static thread_context 	*self;
	std::atomic<uint64_t>			epoch;
	std::atomic<uint64_t>			*reservations;
	uint64_t				epoch_freq;	// freg. of increasing epoch
	// uint64_t				empty_freq;	// freg. of reclaiming retired
};
thread_local thread_context *mem_manager::self = nullptr;

class thread_context
{
public:
	const uint64_t		NUM_THREADS;
	const uint64_t		TID;
	uint64_t		counter;
	uint32_t		curr;
	std::vector<void*>	retired[3];

	thread_context(const uint64_t& num_threads, const uint64_t& tid, mem_manager *m)
		: NUM_THREADS{num_threads}, TID{tid}, counter{0}, curr{0} {}
};

mem_manager::mem_manager(const uint64_t& num_threads, const uint64_t& num_hps, const uint64_t& epoch_freq)
	: epoch{1}, epoch_freq{epoch_freq}
{
	reservations = new std::atomic<uint64_t>[num_threads];
	for (uint64_t i = 0; i < num_threads; ++i)
		reservations[i] = MAX;
}

mem_manager::~mem_manager()
{
	delete[] reservations;
}

void mem_manager::register_thread(const uint64_t&	num_threads,
				const uint64_t&		tid,
				const int&		num)
{
	self = new thread_context(num_threads, tid, this);
}

void mem_manager::unregister_thread()
{
	delete self;
}

void mem_manager::op_begin()
{
	// observe the current global epoch
	uint64_t timestamp = epoch.load();	// one RMA

	if (self->curr != timestamp)
	{
		// synchronize with the current global epoch
		reservations[self->TID] = self->curr = timestamp;

		// reset the local counter
		self->counter = 0;
	}
	else // if (self->curr == timestamp)
	{
		// increment the local counter
		++self->counter;

		/* check if the global epoch has not changed for some pre-determined
		 * number of nonblocking operations (self->counter) */
		if (self->counter % epoch_freq == 0)
		{
			/* determine if all processes currently executing within some
			 * nonblocking operation have seen the current global epoch */
			uint64_t	tmp;
			bool		seen = true;
			for (uint64_t i = 0; i < self->NUM_THREADS; ++i)
			{
				tmp = reservations[i].load();	// one RMA
				if (tmp != MAX && tmp != timestamp)
				{
					seen = false;
					break;
				}
			}

			// if yes
			if (seen)
			{				
				// reclaim the oldest retire list
				uint32_t curr = self->curr % 3;
				while (!self->retired[curr].empty())
				{
					delete (int*)self->retired[curr].back();
					self->retired[curr].pop_back();
				}

				// attempt to increment the global epoch
				uint64_t timestamp_new = timestamp % 3 + 1;
				epoch.compare_exchange_strong(timestamp, timestamp_new);	// one RMA

				// synchronize with the current global epoch
				self->curr = self->curr % 3 + 1;
				reservations[self->TID] = self->curr;
			}
		}
	}
}

void mem_manager::op_end()
{
	reservations[self->TID] = MAX;
}

bool mem_manager::try_reserve(void* ptr, void* comp)
{
	return true;
}

void mem_manager::unreserve(void* ptr)
{
	/* no-op */
}

void mem_manager::sched_for_reclaim(void* ptr)
{
	self->retired[self->curr - 1].push_back(ptr);
}	

#endif // EBR_H
