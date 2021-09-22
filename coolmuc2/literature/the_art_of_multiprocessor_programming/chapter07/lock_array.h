#include <atomic>	// std::atomic...

class lock_array()
{
public:
	lock_array(const int& capacity)
	{
		slot = -1;
		size = capacity;
		tail = 0;
		flag = new std::atomic<bool>[capacity];
		for (auto& x : flag)
			x = false;
		flag[0] = true;
	}

	void lock()
	{
		slot = tail.fetch_add(1) % size;
		while (!flag[slot]);
	}

	void ALock::unlock()
	{
		flag[slot] = false;
		flag[(slot + 1) % size] = true;
	}

private:
	static thread_local int	slot;
	std::atomic<int>	tail;
	std::atomic<bool>*	flag;
	int			size;
};
