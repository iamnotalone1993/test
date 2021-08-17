let nProc be the number of processes;
let iProc be the unique id of this process;
let src be the data to be distributed;
let dst be an array storing the destination addresses;

foreach process p in [0, nProc)
	write src into dst[p][iProc];
	//
	// remote address if p != iProc
	
wait for the completion of the writes;

barriers;
// the writes of all processes are completed
