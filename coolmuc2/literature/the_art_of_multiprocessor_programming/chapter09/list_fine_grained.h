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
class FineList<T>
{
public:
	FineList();
	bool add(const T& item);
	bool remove(const T& item);
	bool contains(const T& item);

private:
	Node*		head;
	std::mutex	lock;
};

/* Implementation */
template<typename T>
FineList<T>::FineList()
{
	head = new Node(std::numeric_limits<int>::min());
	head->next = new Node(std::numeric_limits<int>::max());
}

template<typename T>
bool FineList<T>::add(const T& item)
{
	head->lock();
	Node* pred = head;
	Node* curr = pred->next;
	curr->lock();
	while (curr->item < item)
	{
		pred.unlock();
		pred = curr;
		curr = curr->next;
		curr->lock();
	}
	if (curr->item == item)
		return false;
	else
	{
		Node* node = new Node(item);
		node->next = curr;
		pred->next = node;
		return true;
	}
	curr->unlock();	
	pred->unlock();
}

template<typename T>
bool FineList<T>::remove(const T& item)
{
	head->lock();
	Node* pred = head;
	node* curr = pred->next;
	curr->lock();
	while (curr->item < item)
	{
		pred->unlock();
		pred = curr;
		curr = curr->next;
		curr->lock();
	}
	if (curr->item == item)
	{
		pred->next = curr->next;
		return true;
	}
	else
		return false;
	curr->unlock();
	pred->unlock();
}

template<typename T>
bool FineList<T>::contains(const T& item)
{
	head->lock();
	Node* pred = head;
	Node* curr = pred->next;
	curr->lock();
	while (curr->item < item)
	{
		pred->unlock();
		pred = curr;
		curr = curr->next;
		curr->lock();
	}
	if (item == curr->item)
		return true;
	else
		return false;
	curr->unlock();
	pred->unlock();
}
