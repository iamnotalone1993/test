class lock_read()
{
public:
	void lock()
	{
		lock.lock();
		while (writer)
			condition.await();
		++readers;
		lock.unlock();
	}

	void unlock()
	{
		lock.lock();
		--readers;
		if (readers == 0)
			condition.signal_all();
		lock.unlock();
	}
};

class lock_write()
{
public:
	void lock()
	{
		lock.lock();
		while (readers > 0 || writer)
			condition.await();
		writer = true;
		lock.unlock();
	}

	void unlock()
	{
		lock.lock();
		writer = false;
		condition.signal_all();
		lock.unlock();
	}
};

class RWlock_simple
{
private:
	int		readers;
	bool		writer;
	lock		my_lock;
	condtion	my_condition;
	lock		lock_read;
	lock		lock_write;

public:
	RWlock_simple()
	{
		writer = false;
		readers = 0;
		lock = new lock_reentrant();
		lock_read = new lock_read();
		lock_write = new lock_write();
		condition = lock.new_condition();
	}

	lock lock_read()
	{
		return lock_read;
	}

	lock lock_write()
	{
		return lock_write;
	}
};
