/*
 * Copyright 2025-2026, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef SHARED_EDGE_REGISTRY_H
#define SHARED_EDGE_REGISTRY_H

#include <vector>
#include <map>
#include <set>
#include <utility>

class SharedEdgeRegistry {
public:
			    SharedEdgeRegistry();
			    ~SharedEdgeRegistry();

    void					BuildTopology(const std::vector<std::vector<std::vector<std::vector<double> > > >& rawLayers);
    bool					IsJunction(double x, double y) const;

    void					SaveCurve(double x1, double y1, double x2, double y2, double cx, double cy);
    bool					LoadCurve(double x1, double y1, double x2, double y2, double& cx, double& cy) const;

private:
    struct PointKey {
	int x, y;
	bool operator<(const PointKey& o) const {
	    if (x != o.x) return x < o.x;
	    return y < o.y;
	}
	bool operator==(const PointKey& o) const {
	    return x == o.x && y == o.y;
	}
    };

    struct EdgeKey {
	PointKey p1, p2;
	bool operator<(const EdgeKey& o) const {
	    if (p1 < o.p1) return true;
	    if (o.p1 < p1) return false;
	    return p2 < o.p2;
	}
    };

    PointKey				_MakePointKey(double x, double y) const;
    EdgeKey					_MakeEdgeKey(double x1, double y1, double x2, double y2) const;

    std::map<EdgeKey, std::set<int> > fEdgeUsers;
    std::set<PointKey>		fJunctions;
    std::map<EdgeKey, std::pair<double, double> > fCurves;
};

#endif
