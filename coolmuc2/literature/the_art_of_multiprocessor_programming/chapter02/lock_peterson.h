class lock_peterson
{
public:
	void lock()
	{
		int i = tid();			// tid() returns the calling thread's ID
		int j = 1 - i;			// j is the other thread's ID
		flag[i] = true;			// I'm interested
		victim = i;			// you go first
		while (flag[j] && victim == i);	// wait
	}

	void unlock()
	{
		int i = tid();
		flag[i] = false;	// I'm no longer interested
	}

private:
	bool[2] flag;
	int	victim;
};
