class lock_bakery
{
public:
	lock_bakery(const int& n)
	{
		flag = new bool[n];
		labe = new label[n];
		for (int i = 0; i < n; ++i)
		{
			flag[i] = false;
			labe[i] = 0;
		}
	}

	void lock()
	{
		int i = tid();
		flag[i] = true;
		labe[i] = max(labe[0], ..., labe[n-1]) + 1;
		while (exists(k) != i && flag[k] && (labe[k], k) << (lebe[i],i));
		/* (labe[i],i) << (labe[j],j) if and only if
		 * labe[i] <= labe[j] && i < j
		 */
	}

	void unlock()
	{
		flag[tid()] = false;
	}
private:
	bool*	flag;
	label*	labe;
};
