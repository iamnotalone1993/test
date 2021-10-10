class semaphore
{
private:
	const int	capacity;
	int		state;
	Lock		lock;
	Condition	condition;

public:
	semaphore(const int& c)
	{
		capacity = c;
		state = 0;
		lock = new reentrant_lock();
		condition = lock.new_condition();
	}

	void acquire()
	{
		lock.lock();
		while (state == capacity)
			condition.await();
		++state;
		lock.unlock();
	}

	void release()
	{
		lock.lock();
		--state;
		condition.signal_all();
		lock.unlock();
	}
};
