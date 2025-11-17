/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>

#include "RegionMerger.h"
#include "MathUtils.h"

RegionMerger::RegionMerger() {}
RegionMerger::~RegionMerger() {}

double
RegionMerger::_ColorDiffL2(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1,
						unsigned char r2, unsigned char g2, unsigned char b2, unsigned char a2,
						bool useLinear)
{
	return MathUtils::PerceptualColorDistanceForMerge(r1, g1, b1, a1, r2, g2, b2, a2);
}

void
RegionMerger::_AccumulateSample(WLSums& s, double x, double y, double luma, double w)
{
	s.W   += w;
	s.Wx  += w * x;
	s.Wy  += w * y;
	s.Wxx += w * x * x;
	s.Wyy += w * y * y;
	s.Wxy += w * x * y;

	s.Ws  += w * luma;
	s.Wsx += w * luma * x;
	s.Wsy += w * luma * y;
}

bool
RegionMerger::_GradientFromSums(const WLSums& s, double& gx, double& gy)
{
	double M[3][3] = {
		{ s.W,  s.Wx, s.Wy },
		{ s.Wx, s.Wxx, s.Wxy },
		{ s.Wy, s.Wxy, s.Wyy }
	};
	double B[3] = { s.Ws, s.Wsx, s.Wsy };
	double X[3] = { 0,0,0 };
	if (!MathUtils::Solve3x3Normalized(M, B, X))
		return false;
	gx = X[1];
	gy = X[2];
	return true;
}

void
RegionMerger::_BuildAdjacency(const IndexedBitmap& indexed,
							const BitmapData& source,
							const TracingOptions& options,
							std::map<std::pair<int,int>, EdgeStats>& adj)
{
	const std::vector<std::vector<int> >& arr = indexed.Array();
	const std::vector<std::vector<unsigned char> >& palette = indexed.Palette();

	if (arr.empty() || arr[0].empty())
		return;

	int h = (int)arr.size();
	int w = (int)arr[0].size();

	int ys = 1, ye = h - 2;
	int xs = 1, xe = w - 2;

	bool useLinear = options.fRegionMergeUseLinearRGB;

	for (int y = ys; y <= ye; y++) {
		for (int x = xs; x <= xe; x++) {
			int a = arr[y][x];
			if (a < 0) continue;

			int b = arr[y][x + 1];
			if (b >= 0 && b != a) {
				int aa = a < b ? a : b;
				int bb = a < b ? b : a;

				if (aa < (int)palette.size() && bb < (int)palette.size()) {
					unsigned char alphaA = palette[aa][3];
					unsigned char alphaB = palette[bb][3];
					
					if (MathUtils::IsTransparent(alphaA) || MathUtils::IsTransparent(alphaB)) {
						continue;
					}
					
					if (MathUtils::AlphaGroup(alphaA) != MathUtils::AlphaGroup(alphaB)) {
						continue;
					}
					
					std::pair<int,int> key(aa, bb);
					std::map<std::pair<int,int>, EdgeStats>::iterator it = adj.find(key);
					if (it == adj.end()) {
						adj[key] = EdgeStats(aa, bb);
						it = adj.find(key);
					}

					int ax = x - 1;
					int ay = y - 1;
					int bx = x;
					int by = y - 1;

					if (ax >= 0 && ax < source.Width() && ay >= 0 && ay < source.Height() &&
						bx >= 0 && bx < source.Width() && by >= 0 && by < source.Height()) {

						unsigned char r1 = source.GetPixelComponent(ax, ay, 0);
						unsigned char g1 = source.GetPixelComponent(ax, ay, 1);
						unsigned char b1 = source.GetPixelComponent(ax, ay, 2);
						unsigned char a1 = source.GetPixelComponent(ax, ay, 3);

						unsigned char r2 = source.GetPixelComponent(bx, by, 0);
						unsigned char g2 = source.GetPixelComponent(bx, by, 1);
						unsigned char b2 = source.GetPixelComponent(bx, by, 2);
						unsigned char a2 = source.GetPixelComponent(bx, by, 3);

						double diff = _ColorDiffL2(r1,g1,b1,a1,r2,g2,b2,a2,useLinear);
						if (diff > MathUtils::MAX_DISTANCE * 0.5) continue;
						
						it->second.sumDiff += diff;
						it->second.count++;

						double R1 = (double)r1, G1 = (double)g1, B1 = (double)b1;
						double R2 = (double)r2, G2 = (double)g2, B2 = (double)b2;
						if (useLinear) {
							R1 = MathUtils::SRGBToLinear(R1);
							G1 = MathUtils::SRGBToLinear(G1);
							B1 = MathUtils::SRGBToLinear(B1);
							R2 = MathUtils::SRGBToLinear(R2);
							G2 = MathUtils::SRGBToLinear(G2);
							B2 = MathUtils::SRGBToLinear(B2);
						}
						double s1 = MathUtils::LumaD(R1,G1,B1);
						double s2 = MathUtils::LumaD(R2,G2,B2);
						_AccumulateSample(it->second.sa, (double)ax, (double)ay, s1, 1.0);
						_AccumulateSample(it->second.sb, (double)bx, (double)by, s2, 1.0);
					}
				}
			}

			b = arr[y + 1][x];
			if (b >= 0 && b != a) {
				int aa = a < b ? a : b;
				int bb = a < b ? b : a;

				if (aa < (int)palette.size() && bb < (int)palette.size()) {
					unsigned char alphaA = palette[aa][3];
					unsigned char alphaB = palette[bb][3];

					if (MathUtils::IsTransparent(alphaA) || MathUtils::IsTransparent(alphaB))
						continue;
					
					if (MathUtils::AlphaGroup(alphaA) != MathUtils::AlphaGroup(alphaB))
						continue;

					std::pair<int,int> key(aa, bb);
					std::map<std::pair<int,int>, EdgeStats>::iterator it = adj.find(key);
					if (it == adj.end()) {
						adj[key] = EdgeStats(aa, bb);
						it = adj.find(key);
					}

					int ax = x - 1;
					int ay = y - 1;
					int bx = x - 1;
					int by = y;

					if (ax >= 0 && ax < source.Width() && ay >= 0 && ay < source.Height() &&
						bx >= 0 && bx < source.Width() && by >= 0 && by < source.Height()) {

						unsigned char r1 = source.GetPixelComponent(ax, ay, 0);
						unsigned char g1 = source.GetPixelComponent(ax, ay, 1);
						unsigned char b1 = source.GetPixelComponent(ax, ay, 2);
						unsigned char a1 = source.GetPixelComponent(ax, ay, 3);

						unsigned char r2 = source.GetPixelComponent(bx, by, 0);
						unsigned char g2 = source.GetPixelComponent(bx, by, 1);
						unsigned char b2 = source.GetPixelComponent(bx, by, 2);
						unsigned char a2 = source.GetPixelComponent(bx, by, 3);

						double diff = _ColorDiffL2(r1,g1,b1,a1,r2,g2,b2,a2,useLinear);
						if (diff > MathUtils::MAX_DISTANCE * 0.5)
							continue;

						it->second.sumDiff += diff;
						it->second.count++;

						double R1 = (double)r1, G1 = (double)g1, B1 = (double)b1;
						double R2 = (double)r2, G2 = (double)g2, B2 = (double)b2;
						if (useLinear) {
							R1 = MathUtils::SRGBToLinear(R1);
							G1 = MathUtils::SRGBToLinear(G1);
							B1 = MathUtils::SRGBToLinear(B1);
							R2 = MathUtils::SRGBToLinear(R2);
							G2 = MathUtils::SRGBToLinear(G2);
							B2 = MathUtils::SRGBToLinear(B2);
						}
						double s1 = MathUtils::LumaD(R1,G1,B1);
						double s2 = MathUtils::LumaD(R2,G2,B2);
						_AccumulateSample(it->second.sa, (double)ax, (double)ay, s1, 1.0);
						_AccumulateSample(it->second.sb, (double)bx, (double)by, s2, 1.0);
					}
				}
			}
		}
	}
}

void
RegionMerger::_ApplyMerging(const IndexedBitmap& indexed,
							const std::map<std::pair<int,int>, EdgeStats>& adj,
							const TracingOptions& options,
							std::vector<int>& indexMap)
{
	int K = (int)indexed.Palette().size();
	DSU dsu;
	dsu.Init(K);

	const std::vector<std::vector<unsigned char> >& palette = indexed.Palette();

	double colorTol = MathUtils::AdaptiveThreshold(K, (double)options.fRegionMergeBoundaryColorTol);
	double angleTolDeg = (double)options.fRegionMergeAngleToleranceDeg;
	double angleTolRad = angleTolDeg * M_PI / 180.0;
	int minCount = options.fRegionMergeMinBoundaryCount;

	const int MIN_REGION_AREA = 50; // TODO ?

	std::map<std::pair<int,int>, EdgeStats>::const_iterator it = adj.begin();
	for (; it != adj.end(); ++it) {
		const EdgeStats& es = it->second;
		if (es.count < (long)minCount)
			continue;

		unsigned char aA = (es.a >= 0 && es.a < K) ? palette[es.a][3] : 255;
		unsigned char aB = (es.b >= 0 && es.b < K) ? palette[es.b][3] : 255;

		if (MathUtils::IsTransparent(aA) || MathUtils::IsTransparent(aB))
			continue;

		if (MathUtils::AlphaGroup(aA) != MathUtils::AlphaGroup(aB))
			continue;

		double meanDiff = es.count > 0 ? (es.sumDiff / (double)es.count) : MathUtils::MAX_DISTANCE;
		if (meanDiff > colorTol)
			continue;

		double gxA = 0.0, gyA = 0.0, gxB = 0.0, gyB = 0.0;
		bool okA = _GradientFromSums(es.sa, gxA, gyA);
		bool okB = _GradientFromSums(es.sb, gxB, gyB);

		bool mergeOK = false;

		if (okA && okB) {
			double nA = std::sqrt(gxA*gxA + gyA*gyA);
			double nB = std::sqrt(gxB*gxB + gyB*gyB);
			if (nA < 1e-8 || nB < 1e-8) {
				mergeOK = true;
			} else {
				double uxA = gxA / nA, uyA = gyA / nA;
				double uxB = gxB / nB, uyB = gyB / nB;
				double dot = uxA * uxB + uyA * uyB;
				if (dot > 1.0) dot = 1.0;
				if (dot < -1.0) dot = -1.0;
				double ang = std::acos(dot);
				if (ang <= angleTolRad || (std::fabs(ang - M_PI) <= angleTolRad)) {
					mergeOK = true;
				}
			}
		} else if (!okA && !okB) {
			mergeOK = true;
		} else {
			mergeOK = false;

			int areaA = (int)es.sa.W;
			int areaB = (int)es.sb.W;

			if (areaA < MIN_REGION_AREA || areaB < MIN_REGION_AREA) {
				mergeOK = true;
			}
		}

		if (mergeOK) {
			dsu.Union(es.a, es.b);
		}
	}

	indexMap.resize(K);
	for (int i = 0; i < K; i++) {
		indexMap[i] = dsu.Find(i);
	}
}

IndexedBitmap
RegionMerger::MergeRegions(const IndexedBitmap& indexed,
						const BitmapData& source,
						const TracingOptions& options)
{
	if (!options.fDetectGradients)
		return indexed;

	const std::vector<std::vector<int> >& arr = indexed.Array();
	if (arr.empty() || arr[0].empty())
		return indexed;

	std::map<std::pair<int,int>, EdgeStats> adjacency;
	_BuildAdjacency(indexed, source, options, adjacency);

	std::vector<int> indexMap;
	_ApplyMerging(indexed, adjacency, options, indexMap);

	std::vector<std::vector<int> > newArray;
	newArray.resize(arr.size());
	for (int j = 0; j < (int)arr.size(); j++) {
		newArray[j].resize(arr[j].size());
		for (int i = 0; i < (int)arr[j].size(); i++) {
			int v = arr[j][i];
			if (v >= 0 && v < (int)indexMap.size())
				newArray[j][i] = indexMap[v];
			else
				newArray[j][i] = v;
		}
	}

	return IndexedBitmap(newArray, indexed.Palette());
}
