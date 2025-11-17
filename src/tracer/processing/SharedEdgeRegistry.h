/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef SHARED_EDGE_REGISTRY_H
#define SHARED_EDGE_REGISTRY_H

#include <vector>
#include <map>

#include "IndexedBitmap.h"

class SharedEdgeRegistry {
public:
							SharedEdgeRegistry();
	explicit				SharedEdgeRegistry(double gridResolution);
							~SharedEdgeRegistry();

	void					RegisterPaths(const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
					   					const IndexedBitmap& indexed);

	void					UnifyCoordinates(double snapTolerance);

	void					UpdatePaths(std::vector<std::vector<std::vector<std::vector<double> > > >& layers);

	bool					IsSharedPoint(int layer, int path, int segment, int pointType) const;

	bool					GetUnifiedCoordinate(int layer, int path, int segment, int pointType,
												double& outX, double& outY) const;

	void					GetSharedSegmentMask(int layer, int path, std::vector<bool>& sharedMask) const;

private:
	struct PointKey {
		int gx, gy;

		PointKey(int x, int y) : gx(x), gy(y) {}

		bool operator<(const PointKey& o) const {
			if (gx != o.gx) return gx < o.gx;
			return gy < o.gy;
		}
	};

	struct PathReference {
		int layer;
		int path;
		int segment;
		int pointType;

		PathReference(int l, int p, int s, int t)
			: layer(l), path(p), segment(s), pointType(t) {}
	};

	struct EdgePoint {
		double sumX, sumY;
		int count;
		double unifiedX, unifiedY;
		std::vector<PathReference> owners;

		EdgePoint() : sumX(0), sumY(0), count(0), unifiedX(0), unifiedY(0) {}
	};

	PointKey				_MakeKey(double x, double y) const;
	void					_SnapToGrid(double& x, double& y, double tolerance) const;
	void					_RegisterPoint(double x, double y, int layer, int path, int segment, int pointType);
	bool					_IsSharedBetweenPaths(const EdgePoint& ep) const;

	std::map<PointKey, EdgePoint> fPoints;
	double					fGridResolution;
};

#endif
