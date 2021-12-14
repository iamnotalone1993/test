#ifndef HPBR_H
#define HPBR_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...

class thread_context;

class mem_manager
{
public:
	std::atomic<thread_context*> head;

	mem_manager(const uint64_t& num_threads, const uint64_t& epoch_freq) {};
	~mem_manager() {};
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t& tid,		// num indicates the maximum number of
				const int&	num);		// locations the caller can reserve
	void unregister_thread();	// called once, after the last call to op_end()

	void op_begin();	// indicate the beginning of a concurrent operation
	void op_end();		// indicate the end of a concurrent operation

	bool try_reserve(void* ptr);	// try to protect a pointer from reclamation
	void unreserve(void* ptr);	// stop protecting a pointer
	void sched_for_reclaim(void* ptr);	// try to reclaim a pointer

private:
	thread_local static thread_context *self;

	void wait_until_unreserved(void* ptr);
};
thread_local thread_context *mem_manager::self = nullptr;

class thread_context
{
public:
	std::vector<void*>	pending_reclaims;
	std::atomic<void*>	*reservations;
	thread_context		*next;
	const int		num;

	thread_context(const int& _num, mem_manager *m) : num{_num}
	{
		reservations = new std::atomic<void*>[num];
		for (int i = 0; i < num; ++i)
			reservations[i] = nullptr;
		do {
			next = m->head.load();
		} while (!m->head.compare_exchange_weak(next, this));
	}
};

void mem_manager::register_thread(const uint64_t&	num_threads,
				const uint64_t&		tid,
				const int&		num)
{
	self = new thread_context(num, this);
}

void mem_manager::unregister_thread()
{
	delete[] self->reservations;
	delete self;
}

void mem_manager::op_begin()
{
	/* no-op */
}

void mem_manager::op_end()
{
	for (int i = 0; i < self->num; ++i)
		self->reservations[i].store(nullptr);
	for (auto p : self->pending_reclaims)
	{
		wait_until_unreserved(p);
		delete (int*)p;
	}
	self->pending_reclaims.clear();
}

bool mem_manager::try_reserve(void* ptr)
{
	for (int i = 0; i < self->num; ++i)
	{
		if (self->reservations[i] == nullptr)
		{
			self->reservations[i].store(ptr);
			return true;
		}	
	}
	return false;
}

void mem_manager::unreserve(void* ptr)
{
	for (int i = 0; i < self->num; ++i)
		if (self->reservations[i] == ptr)
		{
			self->reservations[i].store(nullptr);
			return;
		}
}

void mem_manager::sched_for_reclaim(void* ptr)
{
	self->pending_reclaims.push_back(ptr);
}	

void mem_manager::wait_until_unreserved(void* ptr)
{
	for (auto curr = head.load(); curr != nullptr; curr = curr->next)
		for (int i = 0; i < curr->num; ++i)
			while (curr->reservations[i] == ptr);
}

#endif // HPBR_H
