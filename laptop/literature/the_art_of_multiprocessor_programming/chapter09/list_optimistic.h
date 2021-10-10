#include <mutex>	// std::mutex...
#include <limits>	// std::numeric_limits...

template<typename T>
struct Node
{
	T	item;
	Node*	next;

	Node(const T& item) : *this{item, nullptr} {}
	void lock() { lock.lock(); }
	void unlock() { lock.unlock(); }
};

/* Interface */
template<typename T>
class OptimisticList<T>
{
public:
	OptimisticList();
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
OptimisticList<T>::OptimisticList()
{
	head = new Node(std::numeric_limits<int>::min());
	head->next = new Node(std::numeric_limits<int>::max());
}

template<typename T>
bool OptimisticList<T>::add(const T& item)
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
bool OptimisticList<T>::remove(const T& item)
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
bool OptimisticList<T>::contains(const T& item)
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
			return (curr->item == item);
		curr->lock();
		pred->lock();
	}
}

template<typename T>
bool OptimisticList<T>::validate(const Node*& pred, const Node*& curr)
{
	Node *node = head;
	while (node->item <= pred->item)
	{
		if (node == pred)
			return pred->next == curr;
		node = node->next;
	}
	return false;
}
