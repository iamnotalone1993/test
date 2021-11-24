#ifndef LHBR_H
#define LHBR_H

#include <vector>		// std::vector...
#include <atomic>		// std::atomic...
#include <cstdint>		// uint64_t...
#include "abstract_tree.h"	// tree_abs...
#include "local_history.h"	// local_history...
#include <mutex>		// std::mutex... // Testing
#include <iostream>		// std::cout... // Testing

class thread_context;

class mem_manager
{
public:
	tree_abs tree;

	mem_manager(const uint64_t& num_threads, const uint64_t& epoch_freq);
	~mem_manager();
	void register_thread(const uint64_t& 	num_threads,	// called once, before any call to op_begin()
				const uint64_t& tid,		// num indicates the maximum number of
				const int& 	num);		// locations the caller can reserve
	void unregister_thread();	// called once, after the last call to op_end()

	void op_begin();	// indicate the beginning of a concurrent operation
	void op_end();		// indicate the end of a concurrent operation

	bool try_reserve(void* ptr);	// try to protect a pointer from reclamation
	void unreserve(void* ptr);	// stop protecting a pointer
	void sched_for_reclaim(void* ptr);	// try to reclaim a pointer

private:
	thread_local static thread_context 	*self;
	uint64_t				epoch_freq;	// freg. of increasing epoch
};
thread_local thread_context *mem_manager::self = nullptr;

class thread_context
{
public:
	std::vector<void*>	retired[3];
	local_history		history;
	uint64_t		counter;

	thread_context(const uint64_t& num_threads, const uint64_t& tid, mem_manager* m)
		: counter{0}, history(num_threads, tid, m->tree) {}
};

mem_manager::mem_manager(const uint64_t& num_threads, const uint64_t& epoch_freq)
	: epoch_freq{epoch_freq} {}

mem_manager::~mem_manager()
{
	/* no-op */
}

void mem_manager::register_thread(const uint64_t& num_threads, const uint64_t& tid, const int& num)
{
	self = new thread_context(num_threads, tid, this);
}

void mem_manager::unregister_thread()
{
	delete self;
}

void mem_manager::op_begin()
{
	++self->counter;
	if (self->counter % epoch_freq == 0)
		self->history.update(self->retired);
}

void mem_manager::op_end()
{
	/* no-op */
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
	self->retired[self->history.count-1].push_back(ptr);
}	

#endif /* LHBR_H */
