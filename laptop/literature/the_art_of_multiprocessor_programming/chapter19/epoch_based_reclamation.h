#include <vector>	// std::vector...
#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...

class thread_context;

class mem_manager
{
public:
	std::atomic<thread_context*> head;
	
	void register_thread(const int& num);	// called once, before any call to op_begin()
						// num indicates the maximum number of
						// locations the caller can reserve
	void unregister_thread();	// called once, after the last call to op_end()

	void op_begin();	// indicate the beginning of a concurrent operation
	void op_end();		// indicate the end of a concurrent operation

	bool try_reserve(void* ptr);	// try to protect a pointer from reclamation
	void unreserve(void* ptr);	// stop protecting a pointer
	void sched_for_reclaim(void* ptr);	// try to reclaim a pointer

private:
	thread_local static thread_context *self;

	void wait_until_unreserved();
};
thread_local thread_context *mem_manager::self = nullptr;

class thread_context
{
public:
	std::vector<void*>	pending_reclaims;
	std::atomic<uint64_t>	counter;
	thread_context		*next;

	thread_context(mem_manager *m) : counter{0}
	{
		do {
			next = m->head.load();
		} while (!m->head.compare_exchange_weak(next, this));
	}
};

void mem_manager::register_thread(const int& num)
{
	self = new thread_context(this);
}

void mem_manager::unregister_thread()
{
	/* no-op */
}

void mem_manager::op_begin()
{
	++self->counter;
}

void mem_manager::op_end()
{
	++self->counter;
	if (self->pending_reclaims.size() == 0)
		return;
	wait_until_unreserved();
	for (auto p : self->pending_reclaims)
		delete (int*)p;
	self->pending_reclaims.clear();
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
	self->pending_reclaims.push_back(ptr);
}	

void mem_manager::wait_until_unreserved()
{
	uint64_t val;
	for (auto curr = head.load(); curr != nullptr; curr = curr->next)
	{
		val = curr->counter;
		if (val % 2 != 0)
			while (curr->counter.load() == val);
	}
}

