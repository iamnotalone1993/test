#include <iostream>			// std::cout...
#include <thread>			// std::thread...
#include <atomic>			// std::atomic...
#include <cassert>			// assert...
#include "EBR.h"			// mem_manager...

/* Shared constants */
const int       NUM_OF_THREADS  = std::thread::hardware_concurrency();
//const int       NUM_OF_THREADS  = 5;
const int       NUM_OF_PAIRS    = 100000; // push & pop pairs

mem_manager	mm(NUM_OF_THREADS, 1);

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
	bool push(const T& value);
	bool pop(T &value);
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
bool stack<T>::push(const T& value)
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
bool stack<T>::pop(T& value)
{
	mm.op_begin();
	node<T> *top_old, *top_new, *hp = nullptr;
	do {
		top_old = top.load();
		if (top_old == nullptr)
		{
			mm.unreserve(hp);
			mm.op_end();
			return false;
		}
		hp = top_old;
		mm.try_reserve(hp);
		top_new = top_old->next;
	} while (!top.compare_exchange_weak(top_old, top_new));
	mm.unreserve(hp);
	value = top_old->value;
	mm.sched_for_reclaim(hp);
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
inline void thread_entry(int tid)
{
	//printf("[%d]Hello World!\n", tid);
	int value;

	mm.register_thread(NUM_OF_THREADS, tid, 1);

	// Sequential Alternating
	for (auto i = 0; i < NUM_OF_PAIRS; ++i)
	{
		my_stack.push(i);
		my_stack.pop(value);
	}

	mm.unregister_thread();
}

/* Main thread's code */
int main()
{
	std::thread 	threads[NUM_OF_THREADS];

	// The main thread forks
	for (auto i = 0; i < NUM_OF_THREADS; ++i)
		threads[i] = std::thread(thread_entry, i);

	// The child threads join
	for (auto i = 0; i < NUM_OF_THREADS; ++i)
		threads[i].join();

	// Print the stack
	my_stack.print();

	// Test the stack
	my_stack.test();

	return 0;
}
