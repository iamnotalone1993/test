#ifndef LHBR_H
#define LHBR_H

#include <vector>		// std::vector...
#include <atomic>		// std::atomic...
#include <cstdint>		// uint64_t...
#include "abstract_tree.h"	// tree_abs...
#include "local_history.h"	// local_history...
#include <mutex>		// std::mutex... // Testing
#include <iostream>		// std::cout... // Testing

template<typename T>
class thread_context;

template<typename T>
class mem_manager
{
public:
	tree_abs tree;

	mem_manager(const uint64_t&	num_threads,
			const uint32_t& num_hps,
			const uint64_t& epoch_freq);
	~mem_manager();
	void register_thread(const uint64_t& 	num_threads,	// called once, before any call to op_begin()
				const uint64_t& tid,		// num indicates the maximum number of
				const uint32_t& num_hps);	// locations the caller can reserve
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
	thread_local static thread_context<T> 	*self;
	uint64_t				epoch_freq;	// freg. of increasing epoch
};

template<typename T>
thread_local thread_context<T> *mem_manager<T>::self = nullptr;

template<typename T>
class thread_context
{
public:
	std::vector<T*>		retired[3];
	local_history<T>	history;
	uint64_t		counter;

	thread_context(const uint64_t& 	num_threads,
			const uint64_t& tid,
			mem_manager<T>* m)
		: counter{0}, history(num_threads, tid, m->tree) {}
};

template<typename T>
mem_manager<T>::mem_manager(const uint64_t& 	num_threads,
				const uint32_t&	num_hps,
				const uint64_t& epoch_freq)
	: epoch_freq{epoch_freq} {}

template<typename T>
mem_manager<T>::~mem_manager()
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
void mem_manager<T>::register_thread(const uint64_t&	num_threads,
					const uint64_t& tid,
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
	++self->counter;
	if (self->counter % epoch_freq == 0)
		self->history.update(self->retired);
}

template<typename T>
void mem_manager<T>::op_end()
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
	self->retired[self->history.count-1].push_back(ptr);
}	

#endif /* LHBR_H */
