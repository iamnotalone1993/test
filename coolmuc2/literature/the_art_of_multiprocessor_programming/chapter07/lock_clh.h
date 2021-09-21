#include <atomic>	// std::atomic...

class node
{
public:
	node() : locked{false} {}

private:
	std::atomic<bool> locked;
	
};

class lock_clh()
{
public:
	lock_clh()
	{
		tail.store(new node());
		my_node = new node();
		my_pred = nullptr;
	}

	void lock()
	{
		my_node->locked = true;
		my_pred = tail.exchange(my_node);
		while (my_pred->locked);	// wait
	}

	void unlock()
	{
		my_node->locked = false;
		my_node = my_pred;
	}

private:
	std::atomic<node*>		tail;
	static thread_local node*	my_pred;
	static thread_local node*	my_node;
};
