#include <mutex>	// std::mutex...
#include <limits>	// std::numeric_limits...

template<typename T>
struct Node
{
	T	item;
	Node*	next;

	Node(const T& item) : *this{item, nullptr} {}
};

/* Interface */
template<typename T>
class CoarseList<T>
{
public:
	CoarseList();
	bool add(const T& item);
	bool remove(const T& item);
	bool contains(const T& item);

private:
	Node*		head;
	std::mutex	lock;
};

/* Implementation */
template<typename T>
CoarseList<T>::CoarseList()
{
	head = new Node(std::numeric_limits<int>::min());
	head->next = new Node(std::numeric_limits<int>::max());
}

template<typename T>
bool CoarseList<T>::add(const T& item)
{
	Node*	pred,
		curr;

	lock.lock();
	for (pred = head, curr = pred->next; curr->item < item; pred = curr, curr = curr->next);
	if (item == curr->item)
		return false;
	else
	{
		Node* node = new Node(item);
		node.next = curr;
		pred.next = node;
		return true;
	}	
	lock.unlock();
}

template<typename T>
bool CoarseList<T>::remove(const T& item)
{
	Node*	pred,
		curr;

	lock.lock();
	for (pred = head, curr = pred->next; curr->item < item; pred = curr, curr = curr->next);
	if (item == curr->item)
	{
		pred->next = curr->next;
		return true;
	}
	else
		return false;
	lock.unlock();
}

template<typename T>
bool CoarseList<T>::contains(const T& item)
{
	Node*	pred,
		curr;

	lock.lock();
	for (pred = head, curr = pred->next; curr->item < item; pred = curr, curr = curr->next);
	if (item == curr->item)
		return true;
	else
		return false;
	lock.unlock();
}
