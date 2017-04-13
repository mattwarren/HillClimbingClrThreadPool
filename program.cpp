#include<stdio.h>
#include <windows.h>
#include "hillclimbing.h"

static HillClimbing HillClimbingInstance;

int main()
{
	printf("Running Hill Climbing algorithm\n");

	LARGE_INTEGER startTime, endTime;
	QueryPerformanceCounter(&startTime);
	Sleep(1000);
	QueryPerformanceCounter(&endTime);

	static LARGE_INTEGER freq;
	if (freq.QuadPart == 0)
	  QueryPerformanceFrequency(&freq);
	// this calculates 'elapsed' in seconds!!
	double elapsed = (double)(endTime.QuadPart - startTime.QuadPart) / freq.QuadPart; 
	printf("startTime = %lld\n", startTime.QuadPart);
	printf("endTime   = %lld\n", endTime.QuadPart);
	printf("freq      = %lld\n", freq.QuadPart);
	printf("Elapsed   = %.3lf secs (%.3lf msecs)\n\n", elapsed, elapsed * 1000.0);

	HillClimbingInstance.Initialize();
	int newMax = 0, threadAdjustmentInterval = 0;
	long totalCompletions = 0, priorCompletionCount = 0;
	int timer = 0, lastSampleTimer = 0;

	int currentThreadCount = ThreadpoolMgr::MinLimitTotalWorkerThreads;
	HillClimbingInstance.ForceChange(currentThreadCount, Initializing);	
	
	CLRRandom randomGenerator;
	randomGenerator.Init(((int)1 << 16) ^ (int)GetCurrentProcessId());
	
	bool randomWorkloadJumps = false;
	//bool randomWorkloadJumps = true;

	FILE *fp;	
	fp = fopen(randomWorkloadJumps ? "results-random.csv" : "results-smooth.csv", "w+");	
	fprintf(fp, "Time,Throughput,Threads\n");
	for (int mode = 1; mode <= 5; mode++)
	{
		int currentWorkLoad = 0;
		switch (mode)
		{
			case 1: case 5:
				currentWorkLoad = 3;
				break;
			case 2: case 4:
				currentWorkLoad = 7;
				break;
			case 3:
				currentWorkLoad = 10;
				break;
			default:
				currentWorkLoad = 1;
		}		

		bool reportedMsgInWorkload = false;
		int workLoadForSection = currentWorkLoad * 500;
		while (workLoadForSection > 0)
		{
			if (randomWorkloadJumps)
			{
				int randomValue = randomGenerator.Next(21); // 0 to 20
				if (randomValue >= 19)
				{
					int randomChange = randomGenerator.Next(-2, 3); // i.e. -2, -1, 0, 1, 2 (not 3)
					if (randomChange != 0)
					{
						printf("Changing workload from %i -> %i\n", currentWorkLoad, currentWorkLoad + randomChange);
						currentWorkLoad += randomChange;
					}
				}
			}
			timer += 1; //tick-tock, each iteration of the loop is 1 second
			totalCompletions += currentThreadCount;
			workLoadForSection -= currentThreadCount;
			//fprintf(fp, "%d,%d\n", min(currentWorkLoad, currentThreadCount), currentThreadCount);
			double randomNoise = randomGenerator.NextDouble() / 100.0 * 5; // [0..1) -> [0..0.01) -> [0..0.05)
			fprintf(fp, "%i,%.3lf,%d\n", timer, (double)min(currentWorkLoad, currentThreadCount) * (0.95 + randomNoise), currentThreadCount);
			// Calling HillClimbingInstance.Update(..) should ONLY happen when we need more threads, not all the time!!
			if (currentThreadCount <= currentWorkLoad)
			{
				// We nievely assume that each work items takes 1 second (which is also our loop/timer length)
				// So in every loop we complete 'currentThreadCount' pieces of work
				int numCompletions = currentThreadCount;
				
				// In win32threadpool.cpp it does the following check before calling Update(..)
				// if (elapsed*1000.0 >= (ThreadAdjustmentInterval/2)) // 
				// Also 'ThreadAdjustmentInterval' is initially set like so ('INTERNAL_HillClimbing_SampleIntervalLow' = 10):
				// ThreadAdjustmentInterval = CLRConfig::GetConfigValue(CLRConfig::INTERNAL_HillClimbing_SampleIntervalLow);
				double sampleDuration = (double)(timer - lastSampleTimer);
				if (sampleDuration*1000.0 >= (threadAdjustmentInterval / 2))
				{					
					newMax = HillClimbingInstance.Update(currentThreadCount, sampleDuration, numCompletions, &threadAdjustmentInterval);
					printf("Mode = %i, Num Completions = %i (%ld), New Max = %i (Old = %i), threadAdjustmentInterval = %i\n",
						mode, numCompletions, totalCompletions, newMax, currentThreadCount, threadAdjustmentInterval);

					// don't increase more that we need to, i.e don't add more threads than are needed for our current workload
					// TODO we never decrease the thread count, what't the best way to do this?
					// the CLR threadpool ONLY uses the Hill-Climbing value for adding threads, never removing them
					// It appears to just age out old threads when they've been dormant for a while (validate this!!!)
					// **Seems** to be related to ThreadpoolMgr::ShouldWorkerKeepRunning()
					int newThreadCount = min(newMax, currentWorkLoad); 
					if (newThreadCount > currentThreadCount && newThreadCount != 0)
					{						
						printf("*** %s thread count, from %i -> %i ***\n",
								newThreadCount > currentThreadCount ? "Increasing" : "Decreasing", 
								currentThreadCount, newThreadCount);
						currentThreadCount = newThreadCount;
					}

					priorCompletionCount = totalCompletions;
					lastSampleTimer = timer;
				}
				else
				{
					printf("Sample Duration is too small, current = %.3lf, needed = %.3lf (threadAdjustmentInterval = %i)\n",
							sampleDuration, (threadAdjustmentInterval / 2) / 1000.0, threadAdjustmentInterval);
				}
			}
			else
			{
				if (reportedMsgInWorkload == false)
				{
					printf("Enough threads to carry out current workload, currentThreadCount = %i, currentWorkLoad= %i\n",
							currentThreadCount, currentWorkLoad);
				}

				reportedMsgInWorkload = true;
			}
		}

		fflush(fp);
	}

	fclose(fp);

	/*printf("Press <ENTER> to exit\n");
	getchar();*/
}