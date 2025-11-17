/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef REGION_MERGER_H
#define REGION_MERGER_H

#include <vector>
#include <map>
#include <utility>
#include <cmath>

#include "IndexedBitmap.h"
#include "BitmapData.h"
#include "TracingOptions.h"

class RegionMerger {
public:
							RegionMerger();
							~RegionMerger();

	IndexedBitmap			MergeRegions(const IndexedBitmap& indexed,
										const BitmapData& source,
										const TracingOptions& options);

private:
	struct WLSums {
		double W;
		double Wx, Wy, Wxx, Wyy, Wxy;
		double Ws, Wsx, Wsy;

		WLSums()
			: W(0), Wx(0), Wy(0), Wxx(0), Wyy(0), Wxy(0), Ws(0), Wsx(0), Wsy(0)
		{}
	};

	struct EdgeStats {
		int a, b;
		long count;
		double sumDiff;

		WLSums sa;
		WLSums sb;

		EdgeStats() : a(0), b(0), count(0), sumDiff(0.0) {}
		EdgeStats(int ia, int ib) : a(ia), b(ib), count(0), sumDiff(0.0) {}
	};

	struct DSU {
		std::vector<int> parent;
		std::vector<int> rankv;

		void Init(int n) {
			parent.resize(n);
			rankv.resize(n);
			for (int i = 0; i < n; i++) { parent[i] = i; rankv[i] = 0; }
		}
		int Find(int x) {
			while (parent[x] != x) {
				parent[x] = parent[parent[x]];
				x = parent[x];
			}
			return x;
		}
		void Union(int a, int b) {
			int ra = Find(a), rb = Find(b);
			if (ra == rb) return;
			if (rankv[ra] < rankv[rb]) parent[ra] = rb;
			else if (rankv[ra] > rankv[rb]) parent[rb] = ra;
			else { parent[rb] = ra; rankv[ra]++; }
		}
	};

private:
	double				_ColorDiffL2(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1,
									unsigned char r2, unsigned char g2, unsigned char b2, unsigned char a2,
									bool useLinear);

	void				_AccumulateSample(WLSums& s, double x, double y, double luma, double w);
	bool				_GradientFromSums(const WLSums& s, double& gx, double& gy);

	void				_BuildAdjacency(const IndexedBitmap& indexed,
										const BitmapData& source,
										const TracingOptions& options,
										std::map<std::pair<int,int>, EdgeStats>& adj);

	void				_ApplyMerging(const IndexedBitmap& indexed,
									const std::map<std::pair<int,int>, EdgeStats>& adj,
									const TracingOptions& options,
									std::vector<int>& indexMap);
};

#endif
