#include<stdio.h>
#include "hillclimbing.h"

static HillClimbing HillClimbingInstance;

int main() 
{
	printf("Running Hill Climbing algorithm\n");
	HillClimbingInstance.Initialize();	
	int currentThreadCount = 5, numCompletions = 10;
	double sampleDuration = 1.0; // in seconds
	int threadAdjustmentInterval = 0;
	int newMax = HillClimbingInstance.Update(
		currentThreadCount, sampleDuration, numCompletions, &threadAdjustmentInterval);
	printf("New Max = %i (%i)\n", newMax, threadAdjustmentInterval);
}