#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

typedef struct Process{
	int ID;
	struct Thread * threads;
	int numThreads;
} Process;

typedef struct Thread{
	int processID;
	int ID;
	int * CPUburstcycles;
	int * IOburstcycles;
	int numCPU;
	enum {NEW, READY, RUNNING, BLOCKING, TERMINATED} status;
	int arrivaltime;
	int CPUtime;
	int IOtime;
	int CPUIOtime;
	int currenttime;
	int finishtime;
	bool fromBlock;
} Thread;

typedef struct Event{
	enum {ARRIVE, RUN, CPU_BURST, IO_BURST, BLOCK, TERMINATE} type;
	int eventtime;
	struct Thread thread;
	bool fromBlock;
} Event;

// RETURN TRUE IF POSITIVE INTEGER > 0
bool isNumeric(char * str){
	bool numeric = false;
	for (int i = 0; i < strlen(str); ++i)
	{
		if (isdigit(str[i])){
			numeric = true;
		}
		else {
			return false;
		}
	}
	if (numeric && atoi(str) > 0) return true;
	else return false;
}
// READ INPUT FILE
Process * readInput(Process * processes, int * numProcesses, int * totalThreads, int * threadSwitch, int * processSwitch){
	// int numProcesses, threadSwitch, processSwitch;
	scanf("%d %d %d%*[^\n]", numProcesses, threadSwitch, processSwitch);
	// printf("%d %d %d\n", *numProcesses, *threadSwitch, *processSwitch);
	processes = calloc(*numProcesses, sizeof(Process));
	for (int i = 0; i < *numProcesses; ++i)
	{
		int processID, numThreads;
		scanf("%d %d%*[^\n]", &processID, &numThreads);
		// printf("%d %d\n", processID, numThreads);
		processes[i].ID = processID;
		processes[i].threads = calloc(numThreads, sizeof(Thread));
		processes[i].numThreads = numThreads;
		*totalThreads += numThreads;
		for (int j = 0; j < numThreads; ++j)
		{
			int threadID, arrivalTime, numCPU;
			scanf("%d %d %d%*[^\n]", &threadID, &arrivalTime, &numCPU);
			// printf("%d %d %d\n", threadID, arrivalTime, numCPU);
			processes[i].threads[j].processID = processID;
			processes[i].threads[j].ID = threadID;
			processes[i].threads[j].CPUburstcycles = calloc(numCPU, sizeof(int));
			processes[i].threads[j].IOburstcycles = calloc(numCPU-1, sizeof(int));
			processes[i].threads[j].numCPU = numCPU;
			for (int k = 0; k < numCPU; ++k)
			{
				int CPUID, CPUtime, IOtime;
				if (k != numCPU-1){
					scanf("%d %d %d%*[^\n]", &CPUID, &CPUtime, &IOtime);
					// printf("%d %d %d\n", CPUID, CPUtime, IOtime);
					processes[i].threads[j].CPUburstcycles[k] = CPUtime;
					processes[i].threads[j].IOburstcycles[k] = IOtime;
				}
				else {
					scanf("%d %d%*[^\n]", &CPUID, &CPUtime);
					// printf("%d %d\n", CPUID, CPUtime);
					processes[i].threads[j].CPUburstcycles[k] = CPUtime;
				}
			}
			processes[i].threads[j].arrivaltime = arrivalTime;
			processes[i].threads[j].currenttime = arrivalTime;
		}
	}
	return processes;
}

// quicksort algorithm referenced from https://www.codingbot.net/2013/01/quick-sort-algorithm-and-c-code.html
void threadSort(Thread * thread, int low, int high){
	int pivot, j, i;
	Thread temp;
	if (low < high)
	{
		pivot = low;
		i = low;
		j = high;

		while(i < j)
		{
			while((thread[i].currenttime <= thread[pivot].currenttime) && (i < high))
			{
				i++;
			}

			while(thread[j].currenttime > thread[pivot].currenttime)
			{
				j--;
			}

			if(i < j)
			{ 
				temp = thread[i];
				thread[i] = thread[j];
				thread[j] = temp;
			}
		}

		temp = thread[pivot];
		thread[pivot] = thread[j];
		thread[j] = temp;
		threadSort(thread, low, j-1);
		threadSort(thread, j+1, high);
	}
}

// quicksort algorithm referenced from https://www.codingbot.net/2013/01/quick-sort-algorithm-and-c-code.html
void eventSort(Event * events, int low, int high){
	int pivot, j, i;
	Event temp;
	if (low < high)
	{
		pivot = low;
		i = low;
		j = high;

		while(i < j)
		{
			while((events[i].eventtime <= events[pivot].eventtime) && (i < high))
			{
				i++;
			}

			while(events[j].eventtime > events[pivot].eventtime)
			{
				j--;
			}

			if(i < j)
			{ 
				temp = events[i];
				events[i] = events[j];
				events[j] = temp;
			}
		}

		temp = events[pivot];
		events[pivot] = events[j];
		events[j] = temp;
		eventSort(events, low, j-1);
		eventSort(events, j+1, high);
	}
}

Thread * sortThreads(Thread * threads, Process * processes, int numProcesses, int totalThreads){
	threads = calloc(totalThreads, sizeof(Thread));
	int threadcount = 0;
	for (int i = 0; i < numProcesses; ++i)
	{
		for (int j = 0; j < processes[i].numThreads; ++j)
		{
			threads[threadcount] = processes[i].threads[j];
			threadcount++;
		}
	}

	threadSort(threads, 0, totalThreads-1);

	return threads;
}

Event * createEvent(Event * events, int * size){
	*size = *size + 1;
	if (events == NULL) {
		events = calloc(1, sizeof(Event));
	}
	else {
		events = realloc(events, sizeof(Event) * (*size));
	}

	return events;
}

Event * removeEvent(Event * events, int index, int * size){
	*size = *size - 1;

	Event * newEvents;
	newEvents = calloc((*size), sizeof(char *));

	for (int i = 0; i < index; ++i)
	{
		newEvents[i] = events[i];
	}

	for (int i = index; i < (*size); ++i)
	{
		newEvents[i] = events[i+1];
	}

	free(events);

	return newEvents;
}

Event * fcfsEvents(Event * events, int * numEvents, Thread * threads, int numThreads, int threadSwitch, int processSwitch, int * maintime){
	int global_time = 0;
	int lastProcess = 0;
	// SETUP
	for (int i = 0; i < numThreads; ++i)
	{
		// ARRIVE EVENT
		events = createEvent(events, numEvents);
		events[*numEvents-1].eventtime = threads[i].arrivaltime;
		events[*numEvents-1].type = ARRIVE;
		events[*numEvents-1].thread = threads[i];
		events[*numEvents-1].thread.status = NEW;
		eventSort(events, 0, *numEvents-1);
		// CPU AND IO TIME
		int cputime = 0;
		int iotime = 0;
		for (int j = 0; j < threads[i].numCPU; ++j)
		{
			cputime += threads[i].CPUburstcycles[j];
			if (j != threads[i].numCPU-1) iotime += threads[i].IOburstcycles[j];
		}
		threads[i].CPUtime = cputime;
		threads[i].IOtime = iotime;
		threads[i].CPUIOtime = cputime + iotime;
	}

	for (int i = 0; i < numThreads; ++i)
	{
		// SWITCH EVENT
		if (lastProcess == threads[i].processID && i > 0){
			// add thread switch
			global_time += threadSwitch;			
		}
		else{
			// add process switch
			global_time += processSwitch;
		}
		events = createEvent(events, numEvents);
		events[*numEvents-1].eventtime = global_time;
		events[*numEvents-1].type = RUN;
		events[*numEvents-1].thread = threads[i];
		// THREAD EVENT
		global_time += threads[i].CPUIOtime;
		events = createEvent(events, numEvents);
		events[*numEvents-1].eventtime = global_time;
		events[*numEvents-1].type = TERMINATE;
		events[*numEvents-1].thread = threads[i];
		threads[i].status = TERMINATED;
		threads[i].finishtime = global_time;
		// printf("Process: %d Thread: %d ARRIVE: %d\n", threads[i].processID, threads[i].ID, threads[i].arrivaltime);
		lastProcess = threads[i].processID;
	}

	*maintime = global_time;

	eventSort(events, 0, *numEvents-1);

	return events;
}

Event * roundEvents(Event * events, int * numEvents, Thread * threads, int numThreads, int threadSwitch, int processSwitch, int * maintime, int rflag){
	int global_time = 0;
	int lastProcess = 0;
	// SETUP
	for (int i = 0; i < numThreads; ++i)
	{
		// ARRIVE EVENT
		events = createEvent(events, numEvents);
		events[*numEvents-1].eventtime = threads[i].arrivaltime;
		events[*numEvents-1].type = ARRIVE;
		events[*numEvents-1].thread = threads[i];
		events[*numEvents-1].thread.status = NEW;
		eventSort(events, 0, *numEvents-1);
		// CPU AND IO TIME
		int cputime = 0;
		int iotime = 0;
		for (int j = 0; j < threads[i].numCPU; ++j)
		{
			cputime += threads[i].CPUburstcycles[j];
			if (j != threads[i].numCPU-1) iotime += threads[i].IOburstcycles[j];
		}
		threads[i].CPUtime = cputime;
		threads[i].IOtime = iotime;
		threads[i].CPUIOtime = cputime + iotime;
	}

	while (1)
	{
		int i = -1;
		for (int j = 0; j < numThreads; ++j)
		{
			if (threads[j].status != TERMINATED) {
				i = j;
				break;
			}
		}
		if (i < 0) break;

		// SWITCH EVENT
		if (lastProcess == threads[i].processID && i > 0){
			// add thread switch
			global_time += threadSwitch;			
		}
		else{
			// add process switch
			global_time += processSwitch;
		}
		events = createEvent(events, numEvents);
		events[*numEvents-1].eventtime = global_time;
		events[*numEvents-1].type = RUN;
		events[*numEvents-1].thread = threads[i];
		events[*numEvents-1].thread.status = RUNNING;
		if (threads[i].fromBlock)
		{
			events[*numEvents-1].fromBlock = true;
		}
		eventSort(events, 0, *numEvents-1);
		// BLOCKING
		if (threads[i].CPUIOtime > rflag)
		{
			global_time += rflag;
			events = createEvent(events, numEvents);
			events[*numEvents-1].eventtime = global_time;
			events[*numEvents-1].type = BLOCK;
			events[*numEvents-1].thread = threads[i];
			eventSort(events, 0, *numEvents-1);
			threads[i].status = BLOCKING;
			threads[i].CPUIOtime -= rflag;
			threads[i].fromBlock = true;
		}
		else {
			global_time += threads[i].CPUIOtime;
			events = createEvent(events, numEvents);
			events[*numEvents-1].eventtime = global_time;
			events[*numEvents-1].type = TERMINATE;
			events[*numEvents-1].thread = threads[i];
			eventSort(events, 0, *numEvents-1);
			threads[i].status = TERMINATED;
			threads[i].finishtime = global_time;
		}
		threads[i].currenttime = global_time;
		threadSort(threads, 0, numThreads-1);

		lastProcess = threads[i].processID;
	}

	*maintime = global_time;

	eventSort(events, 0, *numEvents-1);

	return events;
}

void defaultprint(Thread * threads, int numThreads, int global_time){
	// CPU UTILIZATION
	int cputime = 0;
	for (int i = 0; i < numThreads; ++i)
	{
		cputime += threads[i].CPUtime;
	}
	float CPUutilization = 0;
	CPUutilization = ((float)cputime / (float)global_time) * 100.0;
	// AVERAGE TURNAROUND
	float avgTurnaround = 0;
	for (int i = 0; i < numThreads; ++i)
	{
		avgTurnaround += (float)threads[i].finishtime - (float)threads[i].arrivaltime;
	}
	avgTurnaround /= (float)numThreads;
	printf("Total Time required is %d units\n", global_time);
	printf("Average Turnaround Time is %.2f time units\n", avgTurnaround);
	printf("CPU Utilization is %.2f%%\n\n", CPUutilization);
}

void detailed(Thread * threads, int numThreads){
	for (int i = 0; i < numThreads; ++i)
	{
		printf("Thread %d of Process %d:\n\n", threads[i].ID, threads[i].processID);
		printf("arrival time: %d\n", threads[i].arrivaltime);
		printf("service time: %d\n", threads[i].CPUtime);
		printf("I/O time: %d\n", threads[i].IOtime);
		printf("turnaround time: %d\n", threads[i].finishtime-threads[i].arrivaltime);
		printf("finish time: %d\n\n", threads[i].finishtime);
	}
}

void verbose(Event * events, int numEvents){
	for (int i = 0; i < numEvents; ++i)
	{
		printf("At time %d: Thread %d of Process %d moves from ", events[i].eventtime, events[i].thread.ID, events[i].thread.processID);
		if (events[i].type == ARRIVE){
			printf("new to ready\n");
		}
		else if (events[i].type == RUN){
			if (events[i].fromBlock == 1) printf("blocked to running\n");
			else printf("ready to running\n");
		}
		else if (events[i].type == TERMINATE){
			printf("running to terminated\n");
		}
		else if (events[i].type == BLOCK){
			printf("running to blocked\n");
		}
		printf("\n");
	}
}

int main(int argc, char* argv[]){
	// VALIDATE ARGUMENTS
	if (argc > 5){
		printf("Error: Too many arguments\n");
		exit(0);
	}
	else if (argc < 1){
		printf("Error: Too few arguments\n");
		exit(0);
	}
	else if (argc == 2){
		if (strcmp(argv[1], "-d") != 0 && strcmp(argv[1], "-v") != 0){
			printf("Error: Invalid flags\n");
			exit(0);
		}
	}
	else if (argc == 3){
		if (strcmp(argv[1], "-r") == 0 || (strcmp(argv[1], "-d") == 0 && strcmp(argv[2], "-v") == 0)){
		}
		else {
			printf("Error: Invalid flags\n");
			exit(0);
		}
	}
	else if (argc == 4){
		if ((strcmp(argv[1], "-d") == 0 || strcmp(argv[1], "-v") == 0) && (strcmp(argv[2], "-r") == 0 && isNumeric(argv[3]))){
		}
		else {
			printf("Error: Invalid flags\n");
			exit(0);
		}
	}
	else if (argc == 5){
		if (strcmp(argv[1], "-d") == 0 && strcmp(argv[2], "-v") == 0 && strcmp(argv[3], "-r") == 0 && isNumeric(argv[4])){
		}
		else {
			printf("Error: Invalid flags\n");
			exit(0);
		}
	}

	bool dflag = false, vflag = false;
	int rflag = 0;

	// READ FLAGS
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "-d") == 0){
			dflag = true;
		}
		else if (strcmp(argv[i], "-v") == 0){
			vflag = true;
		}
		else if (strcmp(argv[i], "-r") == 0){
			rflag = atoi(argv[i+1]);
		}
	}

	int numProcesses = 0;
	int totalThreads = 0;
	int numEvents = 0;
	int threadSwitch = 0;
	int processSwitch = 0;
	int global_time = 0;

	Process * processes = NULL;

	processes = readInput(processes, &numProcesses, &totalThreads, &threadSwitch, &processSwitch);

	Thread * threads = NULL;

	threads = sortThreads(threads, processes, numProcesses, totalThreads);

	Event * events = NULL;

	if (rflag == 0){
		events = fcfsEvents(events, &numEvents, threads, totalThreads, threadSwitch, processSwitch, &global_time);
	}
	else {
		events = roundEvents(events, &numEvents, threads, totalThreads, threadSwitch, processSwitch, &global_time, rflag);
	}

	// OUTPUT

	defaultprint(threads, totalThreads, global_time);

	if (dflag){
		detailed(threads, totalThreads);
	}

	if (vflag){
		verbose(events, numEvents);
	}

	// FREE

	for (int i = 0; i < numProcesses; ++i)
	{
		free(processes[i].threads);
	}

	free(processes);

	for (int i = 0; i < totalThreads; ++i)
	{
		free(threads[i].CPUburstcycles);
		free(threads[i].IOburstcycles);
	}

	free(threads);

	free(events);
	
	return 0;
}