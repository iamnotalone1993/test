#include <iostream>	// std::cout...
#include <thread>	// std::thread...
#include <atomic>	// std::atomic...
#include <cassert>	// assert...

/* Shared constants */
const int       NUM_OF_THREADS  = std::thread::hardware_concurrency();
const int       NUM_OF_PAIRS    = 10; // push & pop pairs

template <typename T>
struct Node
{
	std::atomic<Node<T>*>	next;
	T			val;	
};

/* Stack's Interface */
template <typename T>
class Stack
{
public:
	Stack();
	void push(Node<T> *n);
	Node<T> *pop();
	void print();
	void test();

private:
	std::atomic<Node<T>*>	top;
};

/* Stack's Implementation */
template <typename T>
Stack<T>::Stack() : top {nullptr} {}

template <typename T>
void Stack<T>::push(Node<T> *newtop)
{
	Node<T>	*oldtop;

	do {
		oldtop = top.load();
		newtop->next = oldtop;
	} while (!top.compare_exchange_weak(oldtop, newtop));
}

template <typename T>
Node<T> *Stack<T>::pop()
{
	Node<T> *oldtop, *newtop, *last, *returnval;

	do {
		oldtop = top.load();
		if (oldtop == nullptr)
			return nullptr;
	} while (!top.compare_exchange_weak(oldtop, nullptr));

	returnval = oldtop;
	newtop = oldtop->next.load();
	for (last = newtop; last != nullptr; last = last->next)
	{
		if (last->next.load() == nullptr)
		{
			do {
				oldtop = top.load();
				last->next = oldtop
			} while (!top.compare_exchange_weak(oldtop, newtop));
			break;
		}
	}

	return returnval;
}

template <typename T>
void Stack<T>::print()
{
	std::cout << "Printing the stack..." << std::endl;
	for (Node<T> *tmp = top.load(); tmp != nullptr; tmp = tmp->next)
		std::cout << tmp->val << std::endl;
	std::cout << "*********************" << std::endl;
}

template <typename T>
void Stack<T>::test()
{
	assert(top.load() == nullptr);
}

/* Shared variables */
Stack<int>	MyStack;

/* Private variables */
// thread_local ...

/* Child thread's code */
inline void thread_entry()
{
	Node<int>	*n;

	// Sequential Alternating
	for (auto i = 0; i < NUM_OF_PAIRS; ++i)
	{
		n = new Node<int>;
		n->val = i;
		MyStack.push(n);
		n = MyStack.pop();
		delete n;
	}
}

/* Main thread's code */
int main()
{
	std::thread 	threads[NUM_OF_THREADS];

	// The main thread forks
	for (auto i = 0; i < NUM_OF_THREADS; ++i)
		threads[i] = std::thread(thread_entry);

	// The child threads join
	for (auto i = 0; i < NUM_OF_THREADS; ++i)
		threads[i].join();

	// Print the stack
	MyStack.print();

	// Test the stack
	MyStack.test();

	return 0;
}
