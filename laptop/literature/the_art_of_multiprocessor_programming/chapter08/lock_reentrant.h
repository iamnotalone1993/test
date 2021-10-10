class lock_reentrant
{
private:
	lock		my_lock;
	condition	my_condition;
	int		owner;
	int		hold_count;

public:
	lock_reentrant()
	{
		lock = new simple_lock();
		condition = lock.new_condition();
		owner = 0;
		hold_count = 0;
	}

	void lock()
	{
		lock.lock();
		if (owner == tid())
		{
			++hold_count;
			return;
		}
		while (hold_count != 0)
			condition.await();
		owner = tid();
		hold_count = 1;
		lock.unlock();
	}

	void unlock()
	{
		lock.lock();
		if (hold_count == 0 || owner != tid())
			throw new illegal_monitor_state_exception();
		--hold_count;
		if (hold_count == 0)
			condition.signal();
		lock.unlock();
	}
};
