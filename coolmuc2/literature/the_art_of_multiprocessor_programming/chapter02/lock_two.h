class lock_two
{
public:
	void lock()
	{
		int i = tid();		// tid() returns the calling thread's ID
		victim = i;		// let the other go first
		while (victim == i);	// wait
	}

	void unlock() {}		// do nothing

private:
	int victim;			// is shared by all threads
};
