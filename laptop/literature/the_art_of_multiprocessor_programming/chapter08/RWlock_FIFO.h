class lock_read
{
public:
	void lock()
	{
		lock.lock();
		while (writer)
			condition.await();
		++read_acquires;
		lock.unlock();
	}

	void unlock()
	{
		lock.lock();
		++read_releases;
		if (read_acquires == read_releases)
			condition.signal_all();
		lock.unlock();
	}
};

class lock_write
{
public:
	void lock()
	{
		lock.lock();
		while (writer)
			condition.await();
		writer = true;
		while (read_acquires != read_releases)
			condition.await();
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

class RWlock_FIFO
{
private:
	int		read_acquires;
	int		read_releases;
	bool		writer;
	lock		my_lock();
	condition	my_condition;
	lock		lock_read;
	lock		lock_write;

public:
	RWlock_FIFO()
	{
		read_acquires = read_releases = 0;
		writer = false;
		lock = new lock_reentrant();
		condition = lock.new_condition();
		lock_read = new lock_read();
		lock_write = new lock_write();
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
