#ifndef EBR3_H
#define EBR3_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...

template<typename T>
class thread_context;

template<typename T>
class mem_manager
{
public:
	std::atomic<thread_context<T>*> head;
	
	mem_manager(const uint64_t&	num_threads,
			const uint32_t& num_hps,
			const uint64_t& epoch_freq) {};
	~mem_manager() {};
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t& tid,		// num indicates the maximum number of
				const uint32_t&	num_hps);	// locations the caller can reserve
	void unregister_thread();				// called once, after the last call to op_end()
	T* malloc();
	void free(T*& ptr);
	void op_begin();					// indicate the beginning of a concurrent operation
	void op_end();						// indicate the end of a concurrent operation
	bool try_reserve(T*& 			ptr,		// try to protect a pointer from reclamation
			const std::atomic<T*>&	comp);
	void unreserve(T* ptr);					// stop protecting a pointer
	void sched_for_reclaim(T* ptr);				// try to reclaim a pointer

private:
	thread_local static thread_context<T> *self;

	void wait_until_unreserved();
};

template<typename T>
thread_local thread_context<T> *mem_manager<T>::self = nullptr;

template<typename T>
class thread_context
{
public:
	const uint64_t		NUM_THREADS;
	const uint64_t		TID;
	std::vector<T*>		pending_reclaims;
	std::atomic<uint64_t>	counter;
	thread_context<T>	*next;

	thread_context(const uint64_t& 	num_threads,
			const uint64_t& tid,
			mem_manager<T> 	*m)
		: NUM_THREADS{num_threads}, TID{tid}, counter{0}
	{
		do {
			next = m->head.load();
		} while (!m->head.compare_exchange_weak(next, this));
	}
};

template<typename T>
void mem_manager<T>::register_thread(const uint64_t&	num_threads,
					const uint64_t&	tid,
					const uint32_t&	num_hps)
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
	++self->counter;
}

template<typename T>
void mem_manager<T>::op_end()
{
	++self->counter;
	if (self->pending_reclaims.size() == 0)
		return;
	wait_until_unreserved();
	for (auto& ptr : self->pending_reclaims)
		delete ptr;
	self->pending_reclaims.clear();
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
bool mem_manager<T>::try_reserve(T*&			ptr,
				const std::atomic<T*>& 	comp)
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
	self->pending_reclaims.push_back(ptr);
}	

template<typename T>
void mem_manager<T>::wait_until_unreserved()
{
	uint64_t val;
	for (auto curr = head.load(); curr != nullptr; curr = curr->next)
	{
		val = curr->counter;
		if (val % 2 != 0)
			while (curr->counter.load() == val);
	}
}

#endif // EBR3_H
