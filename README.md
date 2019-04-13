## CPU Scheduling Algorithm

A CPU scheduling algorithm that simulates the work done managing threads by a CPU. Outputs detailed scheduling and usage data to the terminal.

## Setup

1. Clone this repo using `https://github.com/jaredmacintyre/CPU-Scheduling.git`

2. Navigate to the project folder with `cd CPU-Scheduling`

3. Build with `make`

4. Run with `./simcpu [-d] [-v] [-r quantum] < testcase1.txt`

Optional flags are in square brackets

Algorithm Explained: 

This CPU scheduling algorithm begins by loading each thread onto the ready queue upon it's arrival time. The algorithm then simulates overhead in switching to the thread with the highest priority, and the thread simulates running.
- First Come First Serve:
	The thread's CPU and IO cycle bursts are simulated until completion, and the thread is then terminated. The algorithm then simulates switching to the next highest priority thread and again simulates running the thread until completion. This process is continued until all threads have been completed.
- Round Robin
	The thread is simulated for the length of the quantum, and if not completed is placed at the end of the priority queue. The algorithm then simulates switching to the next highest priority thread and again simulates running the thread until completion. This process is continued until all threads have been completed.
