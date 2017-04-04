// Licensed to the .NET Foundation under one or more agreements.
// The .NET Foundation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

//=========================================================================

//
// HillClimbing.h
//
// Defines classes for the ThreadPool's HillClimbing concurrency-optimization
// algorithm.
//

//=========================================================================

#ifndef _HILLCLIMBING_H
#define _HILLCLIMBING_H

#include "complex.h"
#include "random.h"
#include <stdio.h>

enum HillClimbingStateTransition 
{
    Warmup, 
    Initializing,
    RandomMove,
    ClimbingMove,
    ChangePoint,
    Stabilizing,
    Starvation, //used by ThreadpoolMgr
    ThreadTimedOut, //used by ThreadpoolMgr
    Undefined,
};

class HillClimbing	
{
private:
    int m_wavePeriod;
    int m_samplesToMeasure;
    double m_targetThroughputRatio;
    double m_targetSignalToNoiseRatio;
    double m_maxChangePerSecond;
    double m_maxChangePerSample;
    int m_maxThreadWaveMagnitude;
    DWORD m_sampleIntervalLow;
    double m_threadMagnitudeMultiplier;
    DWORD m_sampleIntervalHigh;
    double m_throughputErrorSmoothingFactor;
    double m_gainExponent;
    double m_maxSampleError;

    double m_currentControlSetting;
    LONGLONG m_totalSamples;
    int m_lastThreadCount;
    double m_elapsedSinceLastChange; //elapsed seconds since last thread count change
    double m_completionsSinceLastChange; //number of completions since last thread count change

    double m_averageThroughputNoise;

    double* m_samples;      //Circular buffer of the last m_samplesToMeasure samples
    double* m_threadCounts; //Thread counts effective at each of m_samples

    unsigned int m_currentSampleInterval;
    CLRRandom m_randomIntervalGenerator;

    int m_accumulatedCompletionCount;
    double m_accumulatedSampleDuration;

    void ChangeThreadCount(int newThreadCount, HillClimbingStateTransition transition);
    void LogTransition(int threadCount, double throughput, HillClimbingStateTransition transition);

    Complex GetWaveComponent(double* samples, int sampleCount, double period);

public:
    void Initialize();
    int Update(int currentThreadCount, double sampleDuration, int numCompletions, int* pNewSampleInterval);
    void ForceChange(int newThreadCount, HillClimbingStateTransition transition);
};

#define HillClimbingLogCapacity 200

struct HillClimbingLogEntry
{
    DWORD TickCount;
    HillClimbingStateTransition Transition;
    int NewControlSetting;
    int LastHistoryCount;
    float LastHistoryMean;
};

//GARY_DECL(HillClimbingLogEntry, HillClimbingLog, HillClimbingLogCapacity);
extern HillClimbingLogEntry HillClimbingLog[HillClimbingLogCapacity];
//GVAL_DECL(int, HillClimbingLogFirstIndex);
extern int HillClimbingLogFirstIndex;
//GVAL_DECL(int, HillClimbingLogSize);
extern int HillClimbingLogSize;
//typedef DPTR(HillClimbingLogEntry) PTR_HillClimbingLogEntry;
typedef HillClimbingLogEntry *PTR_HillClimbingLogEntry;

// Taken from win32threadpool.h (just the bits we need)
const int CpuUtilizationHigh = 95;                    // remove threads when above this
const int CpuUtilizationLow = 80;                    // inject more threads if below this

class ThreadpoolMgr
{
private:

public:
	static LONG MinLimitTotalWorkerThreads;         // same as MinLimitTotalCPThreads
	static LONG MaxLimitTotalWorkerThreads;
	static LONG cpuUtilization;
	static HillClimbing HillClimbingInstance;
};

#endif
