class lock_filter
{
public:
	lock_filter(const int& n)
	{
		level = new int[n];
		victim = new int[n];		// use 1..(n-1)
		for (int i = 0; i < n; ++i)
			level[i] = 0;
	}

	lock()
	{
		int me = tid();
		for (int i = 1; i < n; ++i)	// attemp to enter level i
		{
			level[me] = i;
			victim[i] = me;
			// spin while conflicts exist
			while (exists(k) != me && level[k] >= i && victim[i] == me);
		}
	}

	unlock()
	{
		int me = tid();
		level[me] = 0;
	}

private:
	int*	level;
	int*	victim;
};
