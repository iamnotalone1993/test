#include <cstdint>	// uint64_t...
#include <iostream>	// std::cout...
#include <thread>	// std::thread...
#include <atomic>	// std::atomic...
#include <cassert>	// assert...
#include "EBR.h"	// mem_manager...

/* Shared constants */
const int       NUM_OF_THREADS  = std::thread::hardware_concurrency();
//const int       NUM_OF_THREADS  = 4;
const int       NUM_OF_PAIRS    = 100000; // push & pop pairs

mem_manager	mm(NUM_OF_THREADS, 1, 1);

template <typename T>
struct node 
{
	T		value;
	node<T>		*next;

	node(const T& value) : value{value}, next{nullptr} {}
};

/* Stack's Interface */
template<typename T>
class stack
{
public:
	stack();
	~stack();
	bool push(const uint64_t& TID, const T& value);
	bool pop(const uint64_t& TID, T &value);
	void print();
	void test();

private:
	std::atomic<node<T>*>	top;
};

/* Treiber stack's Implementation */
template<typename T>
stack<T>::stack() : top{nullptr} {}

template<typename T>
stack<T>::~stack() {}

template<typename T>
bool stack<T>::push(const uint64_t& TID, const T& value)
{
	node<T> *top_old, *top_new;
	top_new = new node<T>(value);
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

	node<T> *top_old, *top_new, *hp;

	while (true)
	{
		hp = top_old = top.load();

		if (top_old == nullptr)
		{
			mm.op_end();

			return false;
		}

		if (!mm.try_reserve(top_old, std::atomic<void*>(top)))
			continue;

		top_new = top_old->next;
		
		if (top.compare_exchange_weak(top_old, top_new))
		{
			mm.unreserve(top_old);

			break;
		}
		else // if (!top.compare_exchange_weak(top_old, top_new))
			mm.unreserve(hp);
	}

	value = top_old->value;

	mm.sched_for_reclaim(top_old);
	mm.op_end();

	return true;
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

	mm.register_thread(NUM_OF_THREADS, tid, 1);

	// Sequential Alternating
	for (auto i = 0; i < NUM_OF_PAIRS; ++i)
	{
		my_stack.push(tid, i);
		my_stack.pop(tid, value);
	}

	mm.unregister_thread();
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
