#include <atomic>	// std::atomic...

class node
{
public:
	node()
	{
		locked = false;
		next.store(nullptr);
	}

private:
	std::atomic<bool>	locked;
	std::atomic<node*>	next;
};

class lock_mcs()
{
public:
	lock_mcs()
	{
		tail.store(nullptr);
		my_node = new node();
	}

	void lock()
	{
		node* my_pred = tail.exchange(my_node);
		if (my_pred != nullptr)
		{
			my_node->locked = true;
			my_pred->next = my_node;
			// wait until predecessor gives up the lock
			while (my_node->locked);
		}
	}

	void unlock()
	{
		if (my_node->next == nullptr)
		{
			if (tail.compare_exchange_strong(my_node, nullptr)
				return;
			// wait until successor fills in its next field
			while (my_node->next == nullptr);
		}
		my_node->next->locked = false;
		my_node->next = nullptr;
	}

private:
	std::atomic<node*>		tail;
	static thread_local node*	my_node;
};
