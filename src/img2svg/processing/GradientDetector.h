/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef GRADIENT_DETECTOR_H
#define GRADIENT_DETECTOR_H

#include <vector>

#include "BitmapData.h"
#include "IndexedBitmap.h"
#include "TracingOptions.h"

class GradientDetector {
public:
	GradientDetector();
	~GradientDetector();

	std::vector<std::vector<IndexedBitmap::LinearGradient>>
		DetectLinearGradients(const IndexedBitmap& indexed,
							  const BitmapData& sourceBitmap,
							  const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
							  const TracingOptions& options);

private:
	void		_FlattenPath(const std::vector<std::vector<double>>& segments,
							 std::vector<std::vector<double>>& outPoints,
							 int maxSubdiv);
	bool		_PointInPolygon(double x, double y,
								const std::vector<std::vector<double>>& polygon);
	void		_Bounds(const std::vector<std::vector<double>>& pts,
						double& minX, double& minY, double& maxX, double& maxY);

	double		_Luma(unsigned char r, unsigned char g, unsigned char b);
	void		_ClampColor(double& v);
	double		_L2rgb(const unsigned char a[3], const unsigned char b[3]);

	double		_PointSegmentDistance(double px, double py, double x1, double y1, double x2, double y2);
	double		_DistanceToPolygon(double px, double py, const std::vector<std::vector<double> >& poly);

	double		_ComputeVariance(const std::vector<double>& vals,
	                            const std::vector<double>& weights);

	bool		_ComputeChannelGradient(const std::vector<double>& vx,
	                               const std::vector<double>& vy,
	                               const std::vector<double>& vw,
	                               const std::vector<double>& vals,
	                               double& outGradX,
	                               double& outGradY,
	                               double& outR2);

	bool		_ComputeRobustDirection(const std::vector<double>& vx,
	                               const std::vector<double>& vy,
	                               const std::vector<double>& vw,
	                               const std::vector<double>& vr,
	                               const std::vector<double>& vg,
	                               const std::vector<double>& vb,
	                               double& outDirX,
	                               double& outDirY,
	                               double& outConfidence);

	IndexedBitmap::LinearGradient
		_DetectForPath(int layerIndex,
					   const std::vector<std::vector<double>>& segments,
					   const IndexedBitmap& indexed,
					   const BitmapData& src,
					   const TracingOptions& options);
};

#endif
