#ifndef HEPR_H
#define HEPR_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...

template<typename T>
class thread_context;

template<typename T>
struct block
{
	T		elem;
	uint64_t	era_new;
	uint64_t	era_del;
};

template<typename T>
class mem_manager
{
public:
	std::atomic<uint64_t>	**reservations;

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
	bool try_reserve(T*&				ptr,	// try to protect a pointer from reclamation
			 const std::atomic<T*>&		comp);
	void unreserve(T* ptr);					// stop protecting a pointer
	void sched_for_reclaim(T* ptr);				// try to reclaim a pointer
private:
	const uint64_t				NUM_THREADS;
	const uint64_t				NONE;
	thread_local static thread_context<T>	*self;
	std::atomic<uint64_t>			epoch;
};

template<typename T>
thread_local thread_context<T> *mem_manager<T>::self = nullptr;

template<typename T>
class thread_context
{
public:
	const uint64_t		NUM_THREADS;
	const uint64_t		TID;
	const uint32_t		NUM_HPS;
	const uint64_t		NONE;
	uint32_t		index;
	std::vector<block<T>*>	pending_reclaims;
	std::atomic<uint64_t>	*reservations;

	thread_context(const uint64_t&	num_threads,
			const uint64_t& tid,
			const uint32_t& num_hps,
			mem_manager<T> *m)
		: NUM_THREADS{num_threads},
		TID{tid},
		NUM_HPS{num_hps},
		NONE{0},
		index{0},
		reservations{m->reservations[tid]} {}
};

template<typename T>
mem_manager<T>::mem_manager(const uint64_t&	num_threads,
				const uint32_t& num_hps,
				const uint64_t& epoch_freq)
	: NUM_THREADS{num_threads}, NONE{0}, epoch{1}
{
	reservations = new std::atomic<uint64_t>*[num_threads];
	for (uint64_t i = 0; i < num_threads; ++i)
	{
		reservations[i] = new std::atomic<uint64_t>[num_hps];
		for (uint32_t j = 0; j < num_hps; ++j)
			reservations[i][j] = NONE;
	}
}

template<typename T>
mem_manager<T>::~mem_manager()
{
	for (uint64_t i = 0; i < NUM_THREADS; ++i)
		delete[] reservations[i];
	delete[] reservations;
}

template<typename T>
void mem_manager<T>::register_thread(const uint64_t&	num_threads,
					const uint64_t&	tid,
					const uint32_t&	num_hps)
{
	self = new thread_context(num_threads, tid, num_hps, this);
}

template<typename T>
void mem_manager<T>::unregister_thread()
{
	/* No-op */
}

template<typename T>
T* mem_manager<T>::malloc()
{
	block<T>* ptr = new block<T>;
	ptr->era_new = epoch.load();	// one RMA
	return reinterpret_cast<T*>(ptr);
}

template<typename T>
void mem_manager<T>::free(T*& ptr)
{
	/* No-op */
}

template<typename T>
void mem_manager<T>::op_begin()
{
	/* No-op */
}

template<typename T>
void mem_manager<T>::op_end()
{
	for (uint32_t i = 0; i < self->NUM_HPS; ++i)
		self->reservations[i] = NONE;
	self->index = 0;
}

template<typename T>
bool mem_manager<T>::try_reserve(T*& ptr, const std::atomic<T*>& comp)
{
	uint64_t	era_prev = self->reservations[self->index],
			era_curr;
	if (self->index < self->NUM_HPS)
		while (true)
		{
			ptr = comp.load();	// one RMA	
			era_curr = epoch.load();// one RMA
			if (era_curr == era_prev)
			{
				++self->index;
				return true;
			}
			self->reservations[self->index] = era_curr;
			era_prev = era_curr;	
		}
	return false;
}

template<typename T>
void mem_manager<T>::unreserve(T* ptr)
{
	self->reservations[self->index] = NONE;
	--self->index;
}

template<typename T>
void mem_manager<T>::sched_for_reclaim(T* ptr)
{
	block<T>* temp = reinterpret_cast<block<T>*>(ptr);
	uint64_t era_curr = epoch.load();	// one RMA
	temp->era_del = era_curr;
	self->pending_reclaims.push_back(temp);
	epoch.compare_exchange_strong(era_curr, 1);	// one RMA
	
	std::vector<uint64_t>      list_hes;
	for (uint64_t i = 0; i < self->NUM_THREADS; ++i)
		for (uint32_t j = 0; j < self->NUM_HPS; ++j)
		{
			era_curr = reservations[i][j];	// one RMA
			if (era_curr != NONE)
				list_hes.push_back(era_curr);
		}

	bool test;
	for (uint64_t i = 0; i < self->pending_reclaims.size(); ++i)
	{
		test = true;
		for (uint64_t j = 0; j < list_hes.size(); ++j)
			if (self->pending_reclaims[i]->era_new <= list_hes[j] &&
					list_hes[j] <= self->pending_reclaims[i]->era_del)
			{
				test = false;
				break;
			}

		if (test)
		{
			delete self->pending_reclaims[i];
			self->pending_reclaims.erase(self->pending_reclaims.begin() + i);
		}
	}	
}

#endif // HEPR_H
