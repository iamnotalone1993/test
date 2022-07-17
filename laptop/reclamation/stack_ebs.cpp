#include <cstdint>	// uint64_t...
#include <iostream>	// std::cout...
#include <thread>	// std::thread...
#include <atomic>	// std::atomic...
#include <limits>	// std::numeric_limits...
#include <algorithm>	// std::max...
#include <cassert>	// assert...
#include <iostream>	// std::cout...
#include "HPBR.h"	// mem_manager...

/* Shared constants */
const uint64_t	NUM_OF_THREADS  = std::thread::hardware_concurrency();
const uint64_t	NUM_OF_PAIRS    = 10000; // push & pop pairs

struct adapt_params
{
	uint32_t	count;
	float		factor;
};

template<typename T>
struct node 
{
	node<T>		*next;
	T		value;
};

template<typename T>
struct thread_info
{
	uint64_t	id;
	char		op;
	node<T>*	cell;
	adapt_params*	adapt;
};

/* Stack's Interface */
template<typename T>
class stack
{
public:
	mem_manager<node<T>>	mm{NUM_OF_THREADS, 1, 1};

	stack();
	~stack();
	bool push(const uint64_t& TID, const T& value);
	bool pop(const uint64_t& TID, T &value);
	void print();
	void test();

private:
	const uint64_t			EMPTY	= std::numeric_limits<int>::max();

	std::atomic<node<T>*>		top;
	std::vector<thread_info<T>*> 	location;
	std::atomic<uint64_t> 		collision[NUM_OF_THREADS];

	void stack_op(thread_info* p);
	bool try_perform_stack_op(thread_info* p);
	void less_op(thread_info* p);
	bool try_collision(thread_info* p, thread_info* q);
	void finish_collision(thread_info* p);
	uint64_t get_position(thread_info* p);
	void adapt_width(enum direction);
};

/* Stack's Implementation */
template<typename T>
stack<T>::stack()
{
	top = nullptr;
	for (uint64_t i = 0; i < NUM_OF_THREADS; ++i)
	{
		location.push_back(nullptr);
		collision[i] = EMPTY;
	}
}

template<typename T>
stack<T>::~stack() {}

template<typename T>
bool stack<T>::push(const uint64_t& TID, const T& value)
{
	node<T> *top_old, *top_new;
	top_new = mm.malloc();
	if (top_new == nullptr)
		return false;
	else // if (top_new != nullptr)
		*top_new = {nullptr, value};
	do {
		top_old = top.load();
		top_new->next = top_old;
	} while (!top.compare_exchange_weak(top_old, top_new));
	return true;
}

template<typename T>
bool stack<T>::pop(const uint64_t& TID, T& value)
{
	mm.op_begin();

	node<T> *top_old, *top_old2, *top_new;

	while (true)
	{
		if (!mm.try_reserve(top_old, top))
			std::cout << "ERROR" << std::endl;

		if (top_old == nullptr)
		{
			mm.op_end();

			return false;
		}

		top_new = top_old->next;
		
		top_old2 = top_old;
		if (top.compare_exchange_weak(top_old, top_new))
		{
			mm.unreserve(top_old);

			break;
		}
		else // if (!top.compare_exchange_weak(top_old, top_new))
			mm.unreserve(top_old2);
	}

	value = top_old->value;

	mm.sched_for_reclaim(top_old);
	mm.op_end();

	return true;
}

template<typename T>
void stack<T>::stack_op(thread_info* p)
{
	if (!try_perform_stack_op(p))
		less_op(p);
}

template<typename T>
bool stack<T>::try_perform_stack_op(thread_info* p)
{
	node	*top, *next;

	if (p->op == PUSH)
	{
		top = S.top;
		p->cell->next = top;
		if (S.top.compare_exchange_strong(top, p->cell))
			return true;
		return false;
	}

	if (p->op == POP)
	{
		top = S.top;
		if (top == nullptr)
			return true;
		next = top->next;
		if (S.top.compare_exchange_stron(top, next))
		{
			p->cell = top;
			return true;
		}
		return false;
	}
}

template<typename T>
void stack<T>::less_op(thread_info* p)
{
	while (true)
	{
		location[TID] = p;
		pos = get_position(p);
		do {
			him = collision[pos];
		} while (!collision[pos].compare_exchange_weak(him, TID));
		if (him != EMPTY)
		{
			q = location[him];
			if (q != nullptr && q->id = him && q->op != p->op)
			{
				if (location[TID].compare_exchange_strong(p, nullptr))
				{
					if (try_collision(p,q))
						return;
					goto label;
				}
				else
				{
					finish_collision(p);
					reurn;
				}
			}
		}

		delay(SPIN);
		adapt_width(SHRINK);
		if (!location[TID].compare_exchange_strong(p, nullptr))
		{
			finish_collision(p);
			return;
		}

	label:
		if (try_perform_stack_op(p))
			return;
	}
}

template<typename T>
bool stack<T>::try_collision(thread_info* p, thread_info* q)
{
	if (p->op == PUSH)
	{
		if (location[him].compare_exchange_strong(q, p))
			return true;
		else
		{
			adapt_width(ENLARGE);
			return false;
		}
	}
	if (p->op == POP)
	{
		if (location[him].compare_exchange_strong(q, nullptr))
		{
			p->cell = q->cell;
			location[TID] = nullptr;
			return true;
		}
		else
		{
			adapt_width(ENLARGE);
			return false;
		}
	}
}

template<typename T>
void stack<T>::finish_collision(thread_info* p)
{
	if (p->op == POP)
	{
		p->cell = location[TID]->cell;
		location[TID] = nullptr;
	}
}

template<typename T>
void stack<T>::get_position(thread_info* p)
{
	// TODO
}

template<typename T>
void stack<T>::adapt_width(enum direction)
{
	if (direction == SHRINK)
	{
		if (p->adapt->count > 0)
			p->adapt->count--;
		else
		{
			p->adapt->count = ADAPT_INIT;
			p->adapt->factor = std::max(p->adapt->factor / 2, MIN_FACTOR);
		}
	}
	else if  (p->adapt->count < MAX_COUNT)
		p->adapt->count++;
	else
	{
		p->adapt->count = ADAPT_INIT;
		p->adapt->factor = std::min(2 * p->adapt->factor, MAX_FACTOR);
	}
}

template<typename T>
void stack<T>::print()
{
	std::cout << "Printing the stack..." << std::endl;
	for (node<T> *tmp = top.load(); tmp != nullptr; tmp = tmp->next)
		std::cout << tmp->value << std::endl;
	std::cout << "*********************" << std::endl;
}

template<typename T>
void stack<T>::test()
{
	assert(top.load() == nullptr);
}

/* Shared variables */
stack<int>	my_stack;

/* Private variables */
// thread_local int tid;

/* Child thread's code */
inline void thread_entry(uint64_t tid)
{
	//printf("[%d]Hello World!\n", tid);
	int value;

	my_stack.mm.register_thread(NUM_OF_THREADS, tid, 1);

	// Sequential Alternating
	for (auto i = 0; i < NUM_OF_PAIRS; ++i)
	{
		my_stack.push(tid, i);
		my_stack.pop(tid, value);
	}

	my_stack.mm.unregister_thread();
}

/* Main thread's code */
int main()
{
	std::thread 	threads[NUM_OF_THREADS];

	// The main thread forks
	for (uint64_t tid = 0; tid < NUM_OF_THREADS; ++tid)
		threads[tid] = std::thread(thread_entry, tid);

	// The child threads join
	for (uint64_t tid = 0; tid < NUM_OF_THREADS; ++tid)
		threads[tid].join();

	// Print the stack
	my_stack.print();

	// Test the stack
	my_stack.test();

	return 0;
}
