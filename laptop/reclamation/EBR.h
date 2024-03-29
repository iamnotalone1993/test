#ifndef EBR_H
#define EBR_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...
#include <limits>       // std::numerics_limits...

template<typename T>
class thread_context;

template<typename T>
class mem_manager
{
public:
	const uint32_t	MIN = std::numeric_limits<uint32_t>::min();

	mem_manager(const uint64_t&	num_threads,
			const uint32_t& num_hps,
			const uint64_t& epoch_freq);
	~mem_manager();
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t&	tid,		// num indicates the maximum number of
				const uint32_t&	num_hps);	// locations the caller can reserve
	void unregister_thread();				// called once, after the last call to op_end()
	T* malloc();
	void free(T*& ptr);
	void op_begin();					// indicate the beginning of a concurrent operation
	void op_end();						// indicate the end of a concurrent operation
	bool try_reserve(T*& 			ptr,		// try to protect a pointer from reclamation
			const std::atomic<T*>& 	comp);
	void unreserve(T* ptr);					// stop protecting a pointer
	void sched_for_reclaim(T* ptr);				// try to reclaim a pointer

private:
	thread_local static thread_context<T> 	*self;
	std::atomic<uint32_t>			epoch;
	std::atomic<uint32_t>			*reservations;
	uint64_t				epoch_freq;	// freg. of increasing epoch
	// uint64_t				empty_freq;	// freg. of reclaiming retired
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
	uint32_t		curr;
	std::vector<T*>		retired[3];

	thread_context(const uint64_t& num_threads, const uint64_t& tid, mem_manager<T> *m)
		: NUM_THREADS{num_threads}, TID{tid}, counter{0}, curr{m->MIN} {}
};

template<typename T>
mem_manager<T>::mem_manager(const uint64_t& num_threads, const uint32_t& num_hps, const uint64_t& epoch_freq)
	: epoch{1}, epoch_freq{epoch_freq}
{
	reservations = new std::atomic<uint32_t>[num_threads];
	for (uint64_t i = 0; i < num_threads; ++i)
		reservations[i] = MIN;
}

template<typename T>
mem_manager<T>::~mem_manager()
{
	delete[] reservations;
}

template<typename T>
void mem_manager<T>::register_thread(const uint64_t&	num_threads,
				const uint64_t&		tid,
				const uint32_t&		num_hps)
{
	self = new thread_context(num_threads, tid, this);
}

template<typename T>
void mem_manager<T>::unregister_thread()
{
	/* No-op */
}

template<typename T>
void mem_manager<T>::op_begin()
{
	// observe the current global epoch
	uint32_t timestamp = epoch.load();	// one RMA

	if (self->curr != timestamp)
	{
		// reset the local counter
		self->counter = 0;

		// synchronize with the current global epoch
		reservations[self->TID] = self->curr = timestamp;
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
			uint32_t	tmp;
			bool		seen = true;
			for (uint64_t i = 0; i < self->NUM_THREADS; ++i)
			{
				tmp = reservations[i].load();	// one RMA
				if (tmp != MIN && tmp != timestamp)
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
				for (auto& ptr : self->retired[curr])
					free(ptr);
				self->retired[curr].clear();

				// attempt to increment the global epoch
				self->curr = self->curr % 3 + 1;
				epoch.compare_exchange_strong(timestamp, self->curr);	// one RMA
			}
		}
		// synchronize with the current global epoch
		reservations[self->TID] = self->curr;
	}
}

template<typename T>
void mem_manager<T>::op_end()
{
	reservations[self->TID] = MIN;
}

template<typename T>
T* mem_manager<T>::malloc()
{
	return new T;
}

template<typename T>
void mem_manager<T>::free(T*& ptr)
{
	delete ptr;
	ptr = nullptr;
}

template<typename T>
bool mem_manager<T>::try_reserve(T*& ptr, const std::atomic<T*>& comp)
{
	ptr = comp.load();
	return true;
}

template<typename T>
void mem_manager<T>::unreserve(T* ptr)
{
	/* No-op */
}

template<typename T>
void mem_manager<T>::sched_for_reclaim(T* ptr)
{
	self->retired[self->curr - 1].push_back(ptr);
}	

#endif // EBR_H
