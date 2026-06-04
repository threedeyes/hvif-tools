/*
 * Copyright 2025-2026, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef PARALLEL_UTILS_H
#define PARALLEL_UTILS_H

#include <thread>
#include <vector>
#include <functional>
#include <algorithm>
#include <future>

class ParallelUtils {
public:
    static void ParallelFor(int start, int end, std::function<void(int)> func) {
	int totalIterations = end - start;
	if (totalIterations <= 0) return;

	const int minIterationsPerThread = 32;

	unsigned int hardwareThreads = std::thread::hardware_concurrency();
	if (hardwareThreads == 0) hardwareThreads = 2;

	unsigned int numThreads = totalIterations / minIterationsPerThread;
	if (numThreads > hardwareThreads) {
	    numThreads = hardwareThreads;
	}
	if (numThreads == 0) {
	    numThreads = 1;
	}

	// If workload is too small, execute sequentially on the caller thread
	if (numThreads <= 1) {
	    for (int j = start; j < end; ++j) {
		func(j);
	    }
	    return;
	}

	std::vector<std::future<void> > futures;
	int blockSize = totalIterations / numThreads;
	int remainder = totalIterations % numThreads;

	int currentStart = start;

	for (unsigned int i = 0; i < numThreads; ++i) {
	    int currentEnd = currentStart + blockSize + (i < (unsigned int)remainder ? 1 : 0);

	    if (currentEnd > currentStart) {
		futures.push_back(std::async(std::launch::async, [currentStart, currentEnd, func]() {
		    for (int j = currentStart; j < currentEnd; ++j) {
			func(j);
		    }
		}));
	    }

	    currentStart = currentEnd;
	}

	for (size_t i = 0; i < futures.size(); i++) {
	    futures[i].wait();
	}
    }
};

#endif
