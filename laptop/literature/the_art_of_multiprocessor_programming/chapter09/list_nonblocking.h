#include <mutex>	// std::mutex...
#include <limits>	// std::numeric_limits...

template<typename T>
class AtomicMarkableReference
{
public:
	bool compareAndSet(T expectedReference, T newReference, bool expectedMark, bool newMark);
	T get(bool[] marked);
	T getReference();
	bool isMarked();

private:
	// TODO
};

template<typename T>
struct Node
{
	T				item;
	AtomicMarkableReference<Node*>*	next;

	Node(const T& item) : *this{item, nullptr} {}
};

class Window
{
public:
	Node*	pred;
	Node*	curr;
	Window(const Node*& myPred, const Node*& myCurr) : pred{myPred}, curr{myCurr} {};
};

Window* find(const Node*& head, const int& item)
{
	Node*	pred = nullptr;
	Node*	curr = nullptr;
	Node*	succ = nullptr;
	bool[]	marked = {false};
	bool	snip;
retry:
	while (true)
	{
		pred = head;
		curr = pred->next->getReference();
		while (true)
		{
			succ = curr->next->get(marked);
			while (marked[0])
			{
				snip = pred->next->compareAndSet(curr, succ, false, false);
				if (!snip)
					continue retry;
				curr = succ;
				succ = curr->next->get(marked);
			}
			if (curr->item >= item)
				return new Window(pred, curr);
			pred = curr;
			curr = succ;
		}
	}
}

/* Interface */
template<typename T>
class LockFreeList<T>
{
public:
	LockFreeList();
	bool add(const T& item);
	bool remove(const T& item);
	bool contains(const T& item);

private:
	Node* head;

	bool validate(const Node*& pred, const Node*& curr);
};

/* Implementation */
template<typename T>
LockFreeList<T>::LockFreeList()
{
	head = new Node(std::numeric_limits<int>::min());
	head->next = new Node(std::numeric_limits<int>::max());
}

template<typename T>
bool LockFreeList<T>::add(const T& item)
{
	while (true)
	{
		Window* window = find(head, item);

		Node* pred = window->pred;
		Node* curr = window->next;
		if (curr->item == item)
			return false;
		else
		{
			Node* node = new Node(item);
			node->next = new AtomicMarkableReference(curr, false);
			if (pred->next->compareAndSet(curr, node, false, false)
				return true;
		}
	}
}

template<typename T>
bool LockFreeList<T>::remove(const T& item)
{
	bool snip;
	while (true)
	{
		Window* window = find(head, item);
		Node* pred = window->pred;
		Node* curr = window->curr;
		if (curr->item != item)
			return false;
		else
		{
			Node* succ = curr->next->getReference();
			snip = curr->next->compareAndSet(succ, succ, false, true);
			if (!snip)
				continue;
			pred->next->compareAndSet(curr, succ, false, false);
			return true;
		}
	}
}

template<typename T>
bool LockFreeList<T>::contains(const T& item)
{
	Node* curr = head;
	while (curr->item < item)
		curr = curr->next->getReference();
	return curr->item == item && !curr->next->isMarked();
}

