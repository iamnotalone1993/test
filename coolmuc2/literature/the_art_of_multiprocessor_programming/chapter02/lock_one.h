class lock_one
{
public:
	void lock()
	{
		int i = tid();		// tid() returns the calling thread's ID
		int j = 1 - i;		// j is the other thread's ID
		flag[i] = true;		// I'm interested
		while (flag[j]);	// wait until flag[j] == false
	}

	void unlock()
	{
		int i = tid();
		flag[i] = false;	// I'm no longer interested
	}

private:
	bool[2] flag;
};
