#ifndef EBR2_H
#define	EBR2_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...
#include <limits>	// std::numerics_limits...

template<typename T>
class thread_context;

template<typename T>
struct block
{
	T		*ptr;
	uint64_t	retired_epoch;
};

template<typename T>
class mem_manager
{
public:
	mem_manager(const uint64_t&	num_threads,
			const uint32_t& num_hps,
			const uint64_t& epoch_freq);
	~mem_manager();
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t&	tid,		// num indicates the maximum number of
				const uint32_t& num_hps);	// locations the caller can reserve
	void unregister_thread();				// called once, after the last call to op_end()
	T* malloc();
	void free(T*& ptr);
	void op_begin();					// indicate the beginning of a concurrent operation
	void op_end();						// indicate the end of a concurrent operation

	bool try_reserve(T*&			ptr,		// try to protect a pointer from reclamation
			const std::atomic<T*>& 	comp);
	void unreserve(T* ptr);					// stop protecting a pointer
	void sched_for_reclaim(T *ptr);				// try to reclaim a pointer

private:
	thread_local static thread_context<T> 	*self;
	std::atomic<uint64_t>			epoch;
	uint64_t				*reservations;
	uint64_t				epoch_freq;     // freg. of increasing epoch
	uint64_t				empty_freq;     // freg. of reclaiming retired

	void empty();
};

template<typename T>
thread_local thread_context<T> *mem_manager<T>::self = nullptr;

template<typename T>
class thread_context
{
public:
	const uint64_t		NUM_THREADS;
	const uint64_t		TID;
	uint64_t		counter;
	std::vector<block<T>*>	retired;

	thread_context(const uint64_t&	num_threads,
			const uint64_t& tid,
			mem_manager<T> 	*m)
		: NUM_THREADS{num_threads}, TID{tid}, counter{0} {}
};

template<typename T>
mem_manager<T>::mem_manager(const uint64_t& 	num_threads,
				const uint32_t& num_hps,
				const uint64_t& epoch_freq)
	: epoch{0},
	reservations{new uint64_t[num_threads]},
	epoch_freq{epoch_freq},
	empty_freq{1} {}

template<typename T>
mem_manager<T>::~mem_manager()
{
	delete[] reservations;
}

template<typename T>
void mem_manager<T>::register_thread(const uint64_t&	num_threads,
					const uint64_t&	tid,
					const uint32_t& num_hps)
{
	self = new thread_context(num_threads, tid, this);
}

template<typename T>
void mem_manager<T>::unregister_thread()
{
	/* No-op */
}

template<typename T>
T* mem_manager<T>::malloc()
{
	return new T;
}

template<typename T>
void mem_manager<T>::free(T*& ptr)
{
	/* No-op */
}

template<typename T>
void mem_manager<T>::op_begin()
{
	reservations[self->TID] = epoch.load();	// one RMA
}

template<typename T>
void mem_manager<T>::op_end()
{
	reservations[self->TID] = std::numeric_limits<uint64_t>::max();
}

template<typename T>
bool mem_manager<T>::try_reserve(T*& ptr, const std::atomic<T*>& comp)
{
	return true;
}

template<typename T>
void mem_manager<T>::unreserve(T* ptr)
{
	/* no-op */
}

template<typename T>
void mem_manager<T>::sched_for_reclaim(T* ptr)
{
	block<T> *tmp = new block<T>;
	self->retired.push_back(tmp);
	tmp->ptr = ptr;
	tmp->retired_epoch = epoch.load();	// one RMA
	++self->counter;
	if (self->counter % epoch_freq == 0)
		epoch.fetch_add(1);		// one RMA
	if (self->retired.size() % empty_freq == 0)
		empty();
}	

template<typename T>
void mem_manager<T>::empty()
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
			delete self->retired[i]->ptr;
			delete self->retired[i];
			self->retired.erase(self->retired.begin() + i);
		}
}

#endif // EBR2_H
