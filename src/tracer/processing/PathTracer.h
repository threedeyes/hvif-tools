/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef PATH_TRACER_H
#define PATH_TRACER_H

#include <vector>

class SharedEdgeRegistry;

class PathTracer {
public:
							PathTracer();
							~PathTracer();

	std::vector<std::vector<double> >
							TracePath(const std::vector<std::vector<double> >& path,
									float lineThreshold, float quadraticThreshold);

	std::vector<std::vector<double> >
							TracePathWithEdgeInfo(
									const std::vector<std::vector<double> >& path,
									float lineThreshold,
									float quadraticThreshold,
									const SharedEdgeRegistry* edgeRegistry,
									int layer,
									int pathIndex);

	std::vector<std::vector<std::vector<double> > >
							BatchTracePaths(const std::vector<std::vector<std::vector<double> > >& internodePaths,
										float lineThreshold, float quadraticThreshold);

private:
	std::vector<std::vector<double> >
							_FitSequence(const std::vector<std::vector<double> >& path,
										float lineThreshold, float quadraticThreshold,
										int sequenceStart, int sequenceEnd, int depth);

	std::vector<std::vector<double> >
							_FitSequenceWithEdges(
										const std::vector<std::vector<double> >& path,
										float lineThreshold, float quadraticThreshold,
										int sequenceStart, int sequenceEnd, int depth,
										const SharedEdgeRegistry* edgeRegistry,
										int layer, int pathIndex);
};

#endif
