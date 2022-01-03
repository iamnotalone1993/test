#ifndef EBR2_H
#define	EBR2_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...
#include <limits>	// std::numerics_limits...

class thread_context;

struct block
{
	void		*ptr;
	uint64_t	retired_epoch;
};

class mem_manager
{
public:
	mem_manager(const uint64_t& num_threads, const uint64_t& num_hps, const uint64_t& epoch_freq);
	~mem_manager();
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t&	tid,		// num indicates the maximum number of
				const int& 	num);		// locations the caller can reserve
	void unregister_thread();	// called once, after the last call to op_end()

	void op_begin();	// indicate the beginning of a concurrent operation
	void op_end();		// indicate the end of a concurrent operation

	bool try_reserve(void* ptr, void* comp);// try to protect a pointer from reclamation
	void unreserve(void* ptr);		// stop protecting a pointer
	void sched_for_reclaim(void *ptr);	// try to reclaim a pointer

private:
	thread_local static thread_context 	*self;
	std::atomic<uint64_t>			epoch;
	uint64_t				*reservations;
	uint64_t				epoch_freq;     // freg. of increasing epoch
	uint64_t				empty_freq;     // freg. of reclaiming retired

	void empty();
};
thread_local thread_context *mem_manager::self = nullptr;

class thread_context
{
public:
	const uint64_t		NUM_THREADS;
	const uint64_t		TID;
	uint64_t		counter;
	std::vector<block*>	retired;

	thread_context(const uint64_t& num_threads, const uint64_t& tid, mem_manager *m)
		: NUM_THREADS{num_threads}, TID{tid}, counter{0} {}
};

mem_manager::mem_manager(const uint64_t& num_threads, const uint64_t& num_hps, const uint64_t& epoch_freq)
	: epoch{0}, reservations{new uint64_t[num_threads]}, epoch_freq{epoch_freq}, empty_freq{1} {}

mem_manager::~mem_manager()
{
	delete[] reservations;
}

void mem_manager::register_thread(const uint64_t&	num_threads,
				const uint64_t&		tid,
				const int& 		num)
{
	self = new thread_context(num_threads, tid, this);
}

void mem_manager::unregister_thread()
{
	delete self;
}

void mem_manager::op_begin()
{
	reservations[self->TID] = epoch.load();	// one RMA
}

void mem_manager::op_end()
{
	reservations[self->TID] = std::numeric_limits<uint64_t>::max();
}

bool mem_manager::try_reserve(void* ptr, void* comp)
{
	return true;
}

void mem_manager::unreserve(void* ptr)
{
	/* no-op */
}

void mem_manager::sched_for_reclaim(void *ptr)
{
	block *tmp = new block;
	self->retired.push_back(tmp);
	tmp->ptr = ptr;
	tmp->retired_epoch = epoch.load();	// one RMA
	++self->counter;
	if (self->counter % epoch_freq == 0)
		epoch.fetch_add(1);		// one RMA
	if (self->retired.size() % empty_freq == 0)
		empty();
}	

void mem_manager::empty()
{
	uint64_t max_safe_epoch = reservations[0];
	for (int i = 1; i < self->NUM_THREADS; ++i)
		if (reservations[i] < max_safe_epoch)
			max_safe_epoch = reservations[i];	// many RMAs
	for (int i = 0; i < self->retired.size(); ++i)
		/* all blocks retired IN or AFTER
		   max_safe_epoch will be protected */
		if (self->retired[i]->retired_epoch < max_safe_epoch)
		{
			delete (uint64_t*)(self->retired[i]->ptr);
			delete (block*)(self->retired[i]);
			self->retired.erase(self->retired.begin() + i);
		}
}

#endif // EBR2_H
