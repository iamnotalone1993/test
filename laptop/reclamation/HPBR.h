#ifndef HPBR_H
#define HPBR_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...

class thread_context;

class mem_manager
{
public:
	std::atomic<void*> 	**reservations;

	mem_manager(const uint64_t& num_threads, const uint64_t& num_hps, const uint64_t& epoch_freq);
	~mem_manager();
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t& tid,		// num indicates the maximum number of
				const int&	num);		// locations the caller can reserve
	void unregister_thread();	// called once, after the last call to op_end()

	void op_begin();	// indicate the beginning of a concurrent operation
	void op_end();		// indicate the end of a concurrent operation

	bool try_reserve(void*			ptr,	// try to protect a pointer from reclamation
			std::atomic<void*>	comp);
	void unreserve(void* ptr);			// stop protecting a pointer
	void sched_for_reclaim(void* ptr);		// try to reclaim a pointer

private:
	const uint64_t				NUM_THREADS;
	thread_local static thread_context 	*self;

	void wait_until_unreserved(void* ptr);
};
thread_local thread_context *mem_manager::self = nullptr;

class thread_context
{
public:
	const uint64_t		NUM_THREADS;
	const uint64_t		TID;
	const uint32_t		NUM_HPS;
	std::vector<void*>	pending_reclaims;
	std::atomic<void*>	*reservations;
	thread_context		*next;

	thread_context(const uint64_t& num_threads, const uint64_t& tid, const uint32_t& num, mem_manager *m)
		: NUM_THREADS{num_threads}, TID{tid}, NUM_HPS{num}, reservations{m->reservations[tid]} {}
};

mem_manager::mem_manager(const uint64_t& num_threads, const uint64_t& num_hps, const uint64_t& epoch_freq)
	: NUM_THREADS{num_threads}
{
	reservations = new std::atomic<void*>*[num_threads];
	for (uint64_t i = 0; i < num_threads; ++i)
	{
		reservations[i] = new std::atomic<void*>[num_hps];
		for (uint32_t j = 0; j < num_hps; ++j)
			reservations[i][j] = nullptr;
	}
}

mem_manager::~mem_manager()
{
	for (uint64_t i = 0; i < NUM_THREADS; ++i)
		delete[] reservations[i];
	delete[] reservations;
}

void mem_manager::register_thread(const uint64_t&	num_threads,
				const uint64_t&		tid,
				const int&		num_hps)
{
	self = new thread_context(num_threads, tid, num_hps, this);
}

void mem_manager::unregister_thread()
{
	delete self;
}

void mem_manager::op_begin()
{
	/* no-op */
}

void mem_manager::op_end()
{
	for (auto &p : self->pending_reclaims)
	{
		wait_until_unreserved(p);
		delete (int*)p;
	}
	self->pending_reclaims.clear();
}

bool mem_manager::try_reserve(void* ptr, std::atomic<void*> comp)
{
	for (uint32_t i = 0; i < self->NUM_HPS; ++i)
		if (self->reservations[i] == nullptr)
		{
			self->reservations[i] = ptr;
			if (ptr == comp.load())
				return true;
			self->reservations[i] = nullptr;
			return false;
		}
	return false;
}

void mem_manager::unreserve(void* ptr)
{
	for (uint32_t i = 0; i < self->NUM_HPS; ++i)
		if (self->reservations[i] == ptr)
		{
			self->reservations[i] = nullptr;
			return;
		}
}

void mem_manager::sched_for_reclaim(void* ptr)
{
	self->pending_reclaims.push_back(ptr);
}	

void mem_manager::wait_until_unreserved(void* ptr)
{
	for (uint64_t i = 0; i < self->NUM_THREADS; ++i)
		for (uint32_t j = 0; j < self->NUM_HPS; ++j)
			while (reservations[i][j].load() == ptr);
}

#endif // HPBR_H
