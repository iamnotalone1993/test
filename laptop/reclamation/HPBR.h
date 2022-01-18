#ifndef HPBR_H
#define HPBR_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...

template<typename T>
class thread_context;

template<typename T>
class mem_manager
{
public:
	std::atomic<T*> 	**reservations;

	mem_manager(const uint64_t& num_threads, const uint64_t& num_hps, const uint64_t& epoch_freq);
	~mem_manager();
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t& tid,		// num indicates the maximum number of
				const int&	num_hps);	// locations the caller can reserve
	void unregister_thread();				// called once, after the last call to op_end()
	T* malloc();
	void free(T*& ptr);
	void op_begin();					// indicate the beginning of a concurrent operation
	void op_end();						// indicate the end of a concurrent operation
	bool try_reserve(T*&			ptr,		// try to protect a pointer from reclamation
			const std::atomic<T*>&	comp);
	void unreserve(T* ptr);					// stop protecting a pointer
	void sched_for_reclaim(T* ptr);				// try to reclaim a pointer

private:
	const uint64_t				NUM_THREADS;
	thread_local static thread_context<T> 	*self;

	void wait_until_unreserved(T* ptr);
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
	std::vector<T*>		pending_reclaims;
	std::atomic<T*>		*reservations;

	thread_context(const uint64_t& 	num_threads,
			const uint64_t& tid,
			const uint32_t& num_hps,
			mem_manager<T> 	*m)
		: NUM_THREADS{num_threads},
		TID{tid}, 
		NUM_HPS{num_hps},
		reservations{m->reservations[tid]} {}
};

template<typename T>
mem_manager<T>::mem_manager(const uint64_t& 	num_threads,
				const uint64_t& num_hps,
				const uint64_t& epoch_freq)
	: NUM_THREADS{num_threads}
{
	reservations = new std::atomic<T*>*[num_threads];
	for (uint64_t i = 0; i < num_threads; ++i)
	{
		reservations[i] = new std::atomic<T*>[num_hps];
		for (uint32_t j = 0; j < num_hps; ++j)
			reservations[i][j] = nullptr;
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
					const int&	num_hps)
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
	return new T;
}

template<typename T>
void mem_manager<T>::free(T*& ptr)
{
	delete ptr;
	ptr = nullptr;
}

template<typename T>
void mem_manager<T>::op_begin()
{
	/* No-op */
}

template<typename T>
void mem_manager<T>::op_end()
{
	for (auto& ptr : self->pending_reclaims)
	{
		wait_until_unreserved(ptr);
		free(ptr);
	}
	self->pending_reclaims.clear();
}

template<typename T>
bool mem_manager<T>::try_reserve(T*& ptr, const std::atomic<T*>& comp)
{
	for (uint32_t i = 0; i < self->NUM_HPS; ++i)
		if (self->reservations[i] == nullptr)
		{
			T* ptr_curr;
			ptr = comp.load();	// one RMA
			while (true)
			{
				self->reservations[i] = ptr;
				ptr_curr = comp.load();	// one RMA
				if (ptr_curr == ptr)
					return true;
				ptr = ptr_curr;
			}
		}
	return false;
}

template<typename T>
void mem_manager<T>::unreserve(T* ptr)
{
	for (uint32_t i = 0; i < self->NUM_HPS; ++i)
		if (self->reservations[i] == ptr)
		{
			self->reservations[i] = nullptr;
			return;
		}
}

template<typename T>
void mem_manager<T>::sched_for_reclaim(T* ptr)
{
	self->pending_reclaims.push_back(ptr);
}	

template<typename T>
void mem_manager<T>::wait_until_unreserved(T* ptr)
{
	for (uint64_t i = 0; i < self->NUM_THREADS; ++i)
		for (uint32_t j = 0; j < self->NUM_HPS; ++j)
			while (reservations[i][j].load() == ptr);
}

#endif // HPBR_H
