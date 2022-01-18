#ifndef TGEIBR_H
#define	TGEIBR_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...
#include <limits>	// std::numerics_limits...

template<typename T>
class thread_context;

template<typename T>
struct block
{
	T		elem;
	uint32_t	birth_epoch;
	uint32_t	retire_epoch;
};

struct reservation
{
	uint32_t	lower;
	uint32_t	upper;
};

template<typename T>
class mem_manager
{
public:
	std::atomic<reservation>	*reservations;

	mem_manager(const uint64_t&	num_threads,
			const uint32_t&	num_hps,
			const uint64_t& epoch_freq);
	~mem_manager();
	T* malloc();
	void free(T*& ptr);
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t&	tid,		// num indicates the maximum number of
				const uint32_t& num_hps);	// locations the caller can reserve
	void unregister_thread();				// called once, after the last call to op_end()
	void op_begin();					// indicate the beginning of a concurrent operation
	void op_end();						// indicate the end of a concurrent operation
	bool try_reserve(T*&			ptr,		// try to protect a pointer from reclamation
			const std::atomic<T*>&	comp);
	void unreserve(T* ptr);					// stop protecting a pointer
	void sched_for_reclaim(T* ptr);				// try to reclaim a pointer

private:
	thread_local static thread_context<T> 	*self;
	std::atomic<uint32_t>			epoch;
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
		: NUM_THREADS{num_threads},
		TID{tid},
		counter{0} {}
};

template<typename T>
mem_manager<T>::mem_manager(const uint64_t&	num_threads,
			const uint32_t&		num_hps,
			const uint64_t&		epoch_freq)
	: epoch{0},
	reservations{new std::atomic<reservation>[num_threads]},
	epoch_freq{epoch_freq},
	empty_freq{1}
{
	/* No-op */
}

template<typename T>
mem_manager<T>::~mem_manager()
{
	delete[] reservations;
}

template<typename T>
T* mem_manager<T>::malloc()
{
	++self->counter;
	if (self->counter % epoch_freq == 0)
		epoch.fetch_add(1);	// one RMA
	block<T> *b = new block<T>;
	b->birth_epoch = epoch.load();	// one RMA
	return reinterpret_cast<T*>(b);
}

template<typename T>
void mem_manager<T>::free(T*& ptr)
{
	/* No-op */
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
void mem_manager<T>::op_begin()
{
	uint32_t tmp = epoch.load();	// one RMA
	reservations[self->TID] = {tmp, tmp};
}

template<typename T>
void mem_manager<T>::op_end()
{
	uint32_t tmp = std::numeric_limits<uint32_t>::max();
	reservations[self->TID] = {tmp, tmp};
}

template<typename T>
bool mem_manager<T>::try_reserve(T*&			ptr,
				const std::atomic<T*>&	comp)
{
	uint32_t ts, lower, upper;
	while (true)
	{
		ptr = comp.load();	// one RMA
		ts = epoch.load();	// one RMA
		lower = reservations[self->TID].load().lower;
		upper = reservations[self->TID].load().upper;
		if (upper < ts)
			reservations[self->TID] = {lower, ts};
		if (upper == epoch.load())	// one RMA
			return true;
	}
	return false;
}

template<typename T>
void mem_manager<T>::unreserve(T* ptr)
{
	/* No-op */
}

template<typename T>
void mem_manager<T>::sched_for_reclaim(T* ptr)
{
	block<T>* pblock = reinterpret_cast<block<T>*>(ptr);
	self->retired.push_back(pblock);
	pblock->retire_epoch = epoch.load();	// one RMA
	if (self->retired.size() % empty_freq == 0)
		empty();
}	

template<typename T>
void mem_manager<T>::empty()
{
	bool conflict;
	reservation res;
	for (int i = 0; i < self->retired.size(); ++i)
	{
		conflict = false;
		for (int j = 0; j < self->NUM_THREADS; ++j)
		{
		/* block protected if some epoch reserved
		   by some thread is in its interval */
			res = reservations[j].load();	// one RMA
			if (self->retired[i]->birth_epoch <= res.upper &
				self->retired[i]->retire_epoch >= res.lower)
				conflict = true;
		}
		if (!conflict)
		{
			delete self->retired[i];
			self->retired.erase(self->retired.begin() + i);
		}
	}
}

#endif // TGEIBR_H
