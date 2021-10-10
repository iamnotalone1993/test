class condition()
{
	void await() throw interrupted_exception;
	bool await(const long& time, time_unit unit) throw interrupted_exception;
	bool await_until(const date& deadline) throw interrupted_exception;
	long await_nanos(const long& nanos_timeout) throw interrupted_exception;
	void await_uninterruptibly();
	void signal();		// wake up one waiting thread
	void signal_all();	// wake up all waiting threads
};
