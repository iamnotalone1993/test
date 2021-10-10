class counter
{
public:
	counter(const long& c) : value{c} {}	// constructor
	
	long get_and_increment()
	{
		return value++;			// danger zone
		/*
		 * // This expression is in effect an abrreviation of the following, more complex code:
		 * long temp = value;		// temp is a local variable to each thread
		 * value = temp + 1;
		 * return temp;
		 */
	}

private:
	long value;				// is shared among all threads
};
