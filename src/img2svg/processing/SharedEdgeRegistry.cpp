/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <set>

#include "SharedEdgeRegistry.h"

SharedEdgeRegistry::SharedEdgeRegistry()
	: fGridResolution(8.0)
{
}

SharedEdgeRegistry::SharedEdgeRegistry(double gridResolution)
	: fGridResolution(gridResolution)
{
	if (fGridResolution < 1.0)
		fGridResolution = 1.0;

	if (fGridResolution > 32.0)
		fGridResolution = 32.0;
}

SharedEdgeRegistry::~SharedEdgeRegistry()
{
}

SharedEdgeRegistry::PointKey
SharedEdgeRegistry::_MakeKey(double x, double y) const
{
	int gx = (int)(x * fGridResolution + 0.5);
	int gy = (int)(y * fGridResolution + 0.5);
	return PointKey(gx, gy);
}

void
SharedEdgeRegistry::_SnapToGrid(double& x, double& y, double tolerance) const
{
	double rx = x - std::floor(x);
	double ry = y - std::floor(y);
	
	double snapThreshold = tolerance;
	if (snapThreshold > 0.25)
		snapThreshold = 0.25;

	if (rx < snapThreshold) {
		x = std::floor(x);
	} else if (rx > (1.0 - snapThreshold)) {
		x = std::floor(x) + 1.0;
	} else if (std::fabs(rx - 0.5) < snapThreshold) {
		x = std::floor(x) + 0.5;
	}

	if (ry < snapThreshold) {
		y = std::floor(y);
	} else if (ry > (1.0 - snapThreshold)) {
		y = std::floor(y) + 1.0;
	} else if (std::fabs(ry - 0.5) < snapThreshold) {
		y = std::floor(y) + 0.5;
	}
}

void
SharedEdgeRegistry::_RegisterPoint(double x, double y, int layer, int path, int segment, int pointType)
{
	PointKey key = _MakeKey(x, y);
	std::map<PointKey, EdgePoint>::iterator it = fPoints.find(key);
	if (it == fPoints.end()) {
		fPoints[key] = EdgePoint();
		it = fPoints.find(key);
	}
	it->second.sumX += x;
	it->second.sumY += y;
	it->second.count++;
	it->second.owners.push_back(PathReference(layer, path, segment, pointType));
}

bool
SharedEdgeRegistry::_IsSharedBetweenPaths(const EdgePoint& ep) const
{
	if (ep.owners.size() < 2)
		return false;

	std::map<int, std::set<int> > layerPaths;
	for (size_t i = 0; i < ep.owners.size(); i++) {
		layerPaths[ep.owners[i].layer].insert(ep.owners[i].path);
	}

	int totalUniquePaths = 0;
	for (std::map<int, std::set<int> >::const_iterator it = layerPaths.begin();
		 it != layerPaths.end(); ++it) {
		totalUniquePaths += it->second.size();
	}

	return totalUniquePaths >= 2;
}

void
SharedEdgeRegistry::RegisterPaths(
	const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
	const IndexedBitmap& indexed)
{
	fPoints.clear();

	for (size_t k = 0; k < layers.size(); k++) {
		for (size_t i = 0; i < layers[k].size(); i++) {
			for (size_t j = 0; j < layers[k][i].size(); j++) {
				const std::vector<double>& seg = layers[k][i][j];
				if (seg.size() < 4) continue;

				int type = (int)seg[0];

				_RegisterPoint(seg[1], seg[2], k, i, j, 0);

				if (type == 1 && seg.size() >= 5) {
					_RegisterPoint(seg[3], seg[4], k, i, j, 1);
				} else if (type == 2 && seg.size() >= 7) {
					_RegisterPoint(seg[5], seg[6], k, i, j, 2);
				}
			}
		}
	}
}

void
SharedEdgeRegistry::UnifyCoordinates(double snapTolerance)
{
	std::map<PointKey, EdgePoint>::iterator it;

	for (it = fPoints.begin(); it != fPoints.end(); ++it) {
		EdgePoint& ep = it->second;

		if (ep.count == 0)
			continue;

		ep.unifiedX = ep.sumX / ep.count;
		ep.unifiedY = ep.sumY / ep.count;

		if (_IsSharedBetweenPaths(ep))
			_SnapToGrid(ep.unifiedX, ep.unifiedY, snapTolerance);
	}
}

void
SharedEdgeRegistry::UpdatePaths(std::vector<std::vector<std::vector<std::vector<double> > > >& layers)
{
	std::map<PointKey, EdgePoint>::const_iterator it;

	for (it = fPoints.begin(); it != fPoints.end(); ++it) {
		const EdgePoint& ep = it->second;

		if (!_IsSharedBetweenPaths(ep))
			continue;

		for (size_t i = 0; i < ep.owners.size(); i++) {
			const PathReference& ref = ep.owners[i];

			if (ref.layer >= (int)layers.size())
				continue;

			if (ref.path >= (int)layers[ref.layer].size())
				continue;

			if (ref.segment >= (int)layers[ref.layer][ref.path].size())
				continue;

			std::vector<double>& seg = layers[ref.layer][ref.path][ref.segment];

			if (ref.pointType == 0) {
				seg[1] = ep.unifiedX;
				seg[2] = ep.unifiedY;
			} else if (ref.pointType == 1 && seg.size() >= 5) {
				seg[3] = ep.unifiedX;
				seg[4] = ep.unifiedY;
			} else if (ref.pointType == 2 && seg.size() >= 7) {
				seg[5] = ep.unifiedX;
				seg[6] = ep.unifiedY;
			}
		}
	}
}

bool
SharedEdgeRegistry::IsSharedPoint(int layer, int path, int segment, int pointType) const
{
	std::map<PointKey, EdgePoint>::const_iterator it;

	for (it = fPoints.begin(); it != fPoints.end(); ++it) {
		const EdgePoint& ep = it->second;

		bool foundThis = false;

		for (size_t i = 0; i < ep.owners.size(); i++) {
			const PathReference& ref = ep.owners[i];

			if (ref.layer == layer && ref.path == path && 
				ref.segment == segment && ref.pointType == pointType) {
				foundThis = true;
				break;
			}
		}

		if (foundThis && _IsSharedBetweenPaths(ep))
			return true;
	}

	return false;
}

bool
SharedEdgeRegistry::GetUnifiedCoordinate(int layer, int path, int segment, int pointType,
										 double& outX, double& outY) const
{
	std::map<PointKey, EdgePoint>::const_iterator it;

	for (it = fPoints.begin(); it != fPoints.end(); ++it) {
		const EdgePoint& ep = it->second;

		for (size_t i = 0; i < ep.owners.size(); i++) {
			const PathReference& ref = ep.owners[i];

			if (ref.layer == layer && ref.path == path && 
				ref.segment == segment && ref.pointType == pointType) {
				outX = ep.unifiedX;
				outY = ep.unifiedY;
				return true;
			}
		}
	}

	return false;
}

void
SharedEdgeRegistry::GetSharedSegmentMask(int layer, int path,
										 std::vector<bool>& sharedMask) const
{
	sharedMask.clear();

	std::map<PointKey, EdgePoint>::const_iterator it;
	std::map<int, bool> sharedSegments;

	for (it = fPoints.begin(); it != fPoints.end(); ++it) {
		const EdgePoint& ep = it->second;
		if (!_IsSharedBetweenPaths(ep))
			continue;

		int segIdx = -1;
		bool foundInThisPath = false;

		for (size_t i = 0; i < ep.owners.size(); i++) {
			const PathReference& ref = ep.owners[i];

			if (ref.layer == layer && ref.path == path) {
				foundInThisPath = true;
				segIdx = ref.segment;
				break;
			}
		}

		if (foundInThisPath && segIdx >= 0) {
			sharedSegments[segIdx] = true;
		}
	}

	int maxSeg = 0;
	std::map<int, bool>::const_iterator si;
	for (si = sharedSegments.begin(); si != sharedSegments.end(); ++si) {
		if (si->first > maxSeg) maxSeg = si->first;
	}

	sharedMask.resize(maxSeg + 1, false);
	for (si = sharedSegments.begin(); si != sharedSegments.end(); ++si) {
		sharedMask[si->first] = true;
	}
}
