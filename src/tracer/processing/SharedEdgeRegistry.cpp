/*
 * Copyright 2025-2026, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "SharedEdgeRegistry.h"
#include <cmath>

SharedEdgeRegistry::SharedEdgeRegistry()
{
}

SharedEdgeRegistry::~SharedEdgeRegistry()
{
}

SharedEdgeRegistry::PointKey
SharedEdgeRegistry::_MakePointKey(double x, double y) const
{
    return {(int)std::round(x * 2.0), (int)std::round(y * 2.0)};
}

SharedEdgeRegistry::EdgeKey
SharedEdgeRegistry::_MakeEdgeKey(double x1, double y1, double x2, double y2) const
{
    PointKey k1 = _MakePointKey(x1, y1);
    PointKey k2 = _MakePointKey(x2, y2);
    if (k2 < k1) return {k2, k1};
    return {k1, k2};
}

void
SharedEdgeRegistry::BuildTopology(const std::vector<std::vector<std::vector<std::vector<double> > > >& rawLayers)
{
    fEdgeUsers.clear();
    fJunctions.clear();
    fCurves.clear();

    for (size_t k = 0; k < rawLayers.size(); k++) {
	for (size_t i = 0; i < rawLayers[k].size(); i++) {
	    const std::vector<std::vector<double> >& path = rawLayers[k][i];
	    int len = path.size();
	    if (len < 2) continue;

	    for (int j = 0; j < len; j++) {
		int next_j = (j + 1) % len;
		EdgeKey ek = _MakeEdgeKey(path[j][0], path[j][1], path[next_j][0], path[next_j][1]);
		fEdgeUsers[ek].insert((int)k);
	    }
	}
    }

    for (size_t k = 0; k < rawLayers.size(); k++) {
	for (size_t i = 0; i < rawLayers[k].size(); i++) {
	    const std::vector<std::vector<double> >& path = rawLayers[k][i];
	    int len = path.size();
	    if (len < 2) continue;

	    for (int j = 0; j < len; j++) {
		int prev_j = (j - 1 + len) % len;
		int next_j = (j + 1) % len;

		EdgeKey edgeIn = _MakeEdgeKey(path[prev_j][0], path[prev_j][1], path[j][0], path[j][1]);
		EdgeKey edgeOut = _MakeEdgeKey(path[j][0], path[j][1], path[next_j][0], path[next_j][1]);

		if (fEdgeUsers[edgeIn] != fEdgeUsers[edgeOut]) {
		    fJunctions.insert(_MakePointKey(path[j][0], path[j][1]));
		}
	    }
	}
    }
}

bool
SharedEdgeRegistry::IsJunction(double x, double y) const
{
    return fJunctions.find(_MakePointKey(x, y)) != fJunctions.end();
}

void
SharedEdgeRegistry::SaveCurve(double x1, double y1, double x2, double y2, double cx, double cy)
{
    fCurves[_MakeEdgeKey(x1, y1, x2, y2)] = std::make_pair(cx, cy);
}

bool
SharedEdgeRegistry::LoadCurve(double x1, double y1, double x2, double y2, double& cx, double& cy) const
{
    std::map<EdgeKey, std::pair<double, double> >::const_iterator it = fCurves.find(_MakeEdgeKey(x1, y1, x2, y2));
    if (it != fCurves.end()) {
	cx = it->second.first;
	cy = it->second.second;
	return true;
    }
    return false;
}