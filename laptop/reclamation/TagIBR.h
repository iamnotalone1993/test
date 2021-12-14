#ifndef TAGIBR_H
#define	TAGIBR_H

#include <vector>	// std::vector...
#include <atomic>	// std::atomic...
#include <cstdint>	// uint64_t...
#include <limits>	// std::numerics_limits...
#include <algorithm>	// std::max...

class thread_context;

struct block
{
	void		*ptr;
	uint32_t	birth_epoch;
	uint32_t	retire_epoch;
};

struct reservation
{
	uint32_t	lower;
	uint32_t	upper;
};

class TPointer
{
public:
	std::atomic<uint32_t>	born_before;	// nonotonically increasing
	std::atomic<block*>	p;

	bool protected_CAS(block *ori, block *new_ptr)
	{
		uint32_t ori_bb;
		do {
			ori_bb = born_before.load();
		} while (new_ptr->birth_epoch <= ori_bb ||
				born_before.compare_exchange_weak(ori_bb, new_ptr->birth_epoch));
		return p.compare_exchange_strong(ori, new_ptr);
	}
	void protected_write(block *ptr)
	{
		uint32_t ori_bb;
		do {
			ori_bb = born_before.load();
		} while (ptr->birth_epoch <= ori_bb ||
				born_before.compare_exchange_weak(ori_bb, ptr->birth_epoch));
		p.store(ptr);
	}
};

class mem_manager
{
public:
	mem_manager(const uint64_t& num_threads, const uint64_t& epoch_freq);
	~mem_manager();
	block* alloc(const uint64_t& size);
	void register_thread(const uint64_t&	num_threads,	// called once, before any call to op_begin()
				const uint64_t&	tid,		// num indicates the maximum number of
				const int& 	num);		// locations the caller can reserve
	void unregister_thread();	// called once, after the last call to op_end()

	void op_begin();	// indicate the beginning of a concurrent operation
	void op_end();		// indicate the end of a concurrent operation

	bool try_reserve(void* ptr);	// try to protect a pointer from reclamation
	void unreserve(void* ptr);	// stop protecting a pointer
	void sched_for_reclaim(block *ptr);	// try to reclaim a pointer

	block* read(TPointer *ptraddr);

private:
	thread_local static thread_context 	*self;
	std::atomic<uint32_t>			epoch;
	std::atomic<reservation>		*reservations;
	uint64_t				epoch_freq;     // freg. of increasing epoch
	uint64_t				empty_freq;     // freg. of reclaiming retired

	void empty();
};
thread_local thread_context *mem_manager::self = nullptr;

class thread_context
{
public:
	uint64_t		num_threads;
	uint64_t		tid;
	uint64_t		counter;
	std::vector<block*>	retired;

	thread_context(const uint64_t& num_threads, const uint64_t& tid, mem_manager *m)
		: num_threads{num_threads}, tid{tid}, counter{0} {}
};

mem_manager::mem_manager(const uint64_t& num_threads, const uint64_t& epoch_freq)
	: epoch{0}, reservations{new std::atomic<reservation>[num_threads]}, epoch_freq{epoch_freq}, empty_freq{1} {}

mem_manager::~mem_manager()
{
	delete[] reservations;
}

void mem_manager::register_thread(const uint64_t&	num_threads,
				const uint64_t&		tid,
				const int& 		num)
{
	self = new thread_context(num_threads, tid, this);
}

void mem_manager::unregister_thread()
{
	delete self;
}

void mem_manager::op_begin()
{
	uint32_t tmp = epoch.load();	// one RMA
	reservations[self->tid].store({tmp, tmp});
}

void mem_manager::op_end()
{
	uint32_t tmp = std::numeric_limits<uint32_t>::max();
	reservations[self->tid].store({tmp, tmp});
}

block* mem_manager::read(TPointer *ptraddr)
{
	block 		*ret;
	reservation 	res;
	uint32_t	tmp;
	while (true)
	{
		ret = ptraddr->p;
		res = reservations[self->tid].load();
		tmp = std::max(res.upper, ptraddr->born_before.load());
		reservations[self->tid].store({res.lower, tmp});
		if (reservations[self->tid].load().upper >= ptraddr->born_before)
			return ret;
	}
}

void write(TPointer *target_ptraddr, block *ptr)
{
	return target_ptraddr->protected_write(ptr);
}

bool CAS(TPointer *target_ptraddr, block *ori, block *new_ptr)
{
	return target_ptraddr->protected_CAS(ori, new_ptr);
}

bool mem_manager::try_reserve(void* ptr)
{
	return false;
}

void mem_manager::unreserve(void* ptr)
{
	/* no-op */
}

void mem_manager::sched_for_reclaim(block *ptr)
{
	self->retired.push_back(ptr);
	ptr->retire_epoch = epoch.load();	// one RMA
	if (self->retired.size() % empty_freq == 0)
		empty();
}	

void mem_manager::empty()
{
	bool conflict;
	reservation res;
	for (int i = 0; i < self->retired.size(); ++i)
	{
		conflict = false;
		for (int j = 0; j < self->num_threads; ++j)	// many RMAs
		{
		/* block protected if some epoch reserved
		   by some thread is in its interval */
			res = reservations[i].load();
			if (self->retired[i]->birth_epoch <= res.upper &
				self->retired[i]->retire_epoch >= res.lower)
				conflict = true;
		}
		if (!conflict)
		{
			delete (block*)self->retired[i];
			self->retired.erase(self->retired.begin() + i);
		}
	}
}

// public interface
block* mem_manager::alloc(const uint64_t& size)
{
	++self->counter;
	if (self->counter % epoch_freq == 0)
		epoch.fetch_add(1);	// one RMA
	block *b = new block;
	b->birth_epoch = epoch.load();	// one RMA
	return b;
}

#endif // TAGIBR_H
