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
	mem_manager(const uint64_t& num_threads, const uint64_t& epoch_freq);
	~mem_manager();
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t&	tid,		// num indicates the maximum number of
				const int&	num);		// locations the caller can reserve
	void unregister_thread();	// called once, after the last call to op_end()

	void op_begin();	// indicate the beginning of a concurrent operation
	void op_end();		// indicate the end of a concurrent operation

	bool try_reserve(void* ptr);	// try to protect a pointer from reclamation
	void unreserve(void* ptr);	// stop protecting a pointer
	void sched_for_reclaim(void* ptr);	// try to reclaim a pointer

private:
	thread_local static thread_context 	*self;
	std::atomic<uint64_t>			epoch;
	uint64_t				*reservations;
	uint64_t				epoch_freq;	// freg. of increasing epoch
	// uint64_t				empty_freq;	// freg. of reclaiming retired
};
thread_local thread_context *mem_manager::self = nullptr;

class thread_context
{
public:
	uint64_t		num_threads;
	uint64_t		tid;
	uint64_t		counter;
	uint64_t		curr;
	std::vector<void*>	retired[3];

	thread_context(const uint64_t& num_threads, const uint64_t& tid, mem_manager *m)
		: num_threads{num_threads}, tid{tid}, counter{0}, curr{0} {}
};

mem_manager::mem_manager(const uint64_t& num_threads, const uint64_t& epoch_freq)
	: epoch{0}, reservations{new uint64_t[num_threads]}, epoch_freq{epoch_freq} {}

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
	uint64_t old = reservations[self->tid];
	reservations[self->tid] = epoch.load();	// one RMA

	if (old != reservations[self->tid])
	{
		self->curr = (self->curr + 1) % 3;
		self->counter = 0;
	}
	else // if (old == reservations[self->tid])
	{
		++self->counter;
		if (self->counter % epoch_freq == 0)
		{
			uint64_t tmp;
			int last = true;
			for (int i = 0; i < self->num_threads; ++i)
			{
				tmp = reservations[i];	// many RMAs
				if (tmp != std::numeric_limits<uint64_t>::max()
							&& tmp < reservations[self->tid])
				{
					last = false;
					break;
				}
			}
			if (last)
			{
				self->curr = (self->curr + 1) % 3;
				while (!self->retired[self->curr].empty())
				{
					delete (int*)self->retired[self->curr].back();
					self->retired[self->curr].pop_back();
				}
				epoch.fetch_add(1);	// one RMA
				++reservations[self->tid];
			}
		}
	}
}

void mem_manager::op_end()
{
	reservations[self->tid] = std::numeric_limits<uint64_t>::max();
}

bool mem_manager::try_reserve(void* ptr)
{
	return false;
}

void mem_manager::unreserve(void* ptr)
{
	/* no-op */
}

void mem_manager::sched_for_reclaim(void* ptr)
{
	self->retired[self->curr].push_back(ptr);
}	

#endif // EBR_H
