#include <mutex>	// std::mutex...
#include <limits>	// std::numeric_limits...

template<typename T>
struct Node
{
	T	item;
	Node*	next;
	bool	marked;

	Node(const T& item) : *this{item, nullptr, false} {}
	void lock() { lock.lock(); }
	void unlock() { lock.unlock(); }
};

/* Interface */
template<typename T>
class LazyList<T>
{
public:
	LazyList();
	bool add(const T& item);
	bool remove(const T& item);
	bool contains(const T& item);

private:
	Node*		head;
	std::mutex	lock;

	bool validate(const Node*& pred, const Node*& curr);
};

/* Implementation */
template<typename T>
LazyList<T>::LazyList()
{
	head = new Node(std::numeric_limits<int>::min());
	head->next = new Node(std::numeric_limits<int>::max());
}

template<typename T>
bool LazyList<T>::add(const T& item)
{
	while (true)
	{
		Node* pred = head;
		Node* curr = pred->next;
		while (curr->item < item)
		{
			pred = curr;
			curr = curr->next;
		}
		pred->lock();
		curr->lock();
		if (validate(pred, curr))
		{
			if (curr->item == item)
				return false;
			else
			{
				Node* node = new Node(item);
				node->next = curr;
				pred->next = node;
				return true;
			}
		}
		curr->unlock();
		pred->unlock();
	}
}

template<typename T>
bool LazyList<T>::remove(const T& item)
{
	while (true)
	{
		Node* pred = head;
		Node* curr = pred->next;
		while (curr->item < item)
		{
			pred = curr;
			curr = curr->next;
		}
		pred->lock();
		curr->lock();
		if (validate(pred, curr))
		{
			if (curr->item == item)
			{
				curr->marked = true;
				pred->next = curr->next;
				return true;
			}
			else
				return false;
		}
		curr->unlock();
		pred->unlock();
	}
}

template<typename T>
bool LazyList<T>::contains(const T& item)
{
	Node* curr = head;
	while (curr->item < item)
		curr = curr->next;
	return curr->item == item && !curr->marked;
}

template<typename T>
bool LazyList<T>::validate(const Node*& pred, const Node*& curr)
{
	return !pred->marked && !curr->marked && pred->next == curr;
}
