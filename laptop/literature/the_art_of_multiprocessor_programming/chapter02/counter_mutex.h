class counter
{
public:
	counter(const long& c) : value{c} {}	// constructor
	
	long get_and_increment()
	{
		lock.lock();			// enter critical section
		long temp = value;		// in critical section
		value = temp + 1;		// in critical section
		lock.unlock();			// leave critical section
		return temp;
	}

private:
	long 	value;
	lock_X	lock;				// to protect critical section
};
