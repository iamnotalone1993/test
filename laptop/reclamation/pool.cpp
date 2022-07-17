#include <iostream>	// std::cout...
#include <thread>	// std::thread...
#include <atomic>	// std::atomic...

template <typename T>
struct Node 
{
	T		val;
	Node<T>*	next;
};

/* Pool's Interface */
template <typename T>
class Pool
{
public:
	Pool();
	void put(Node<T> *n);
	bool *get(Node<T> *n);

private:
	std::atomic<Node<T>*>	head;
};

/* Pool's Implementation */
template <typename T>
Pool<T>::Pool() : head {nullptr} {}

template <typename T>
void Pool<T>::put(Node<T> *n)
{
	n->next = prev;		// remote, regular
	prev = n;		// local, regular
	head.store(prev);	// remote, atomic
}

template <typename T>
bool *Pool<T>::get(Node<T> *n)
{
	Node<T>* curr = head.load();	// local, atomic
	if (curr == prev)	// local, regular
		return false;	// EMPTY
	n = curr->next;	// local, regular
	curr->next = nullptr;	// local, regular
	prev = curr;	// local, regular
	return true;
}

/* Shared variables */
Pool<int>	MyPool;

/* Private variables */
thread_local Node<int>* prev = nullptr;

