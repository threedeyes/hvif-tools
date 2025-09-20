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

	bool		_Solve3x3(double M[3][3], double B[3], double X[3]);
	double		_Luma(unsigned char r, unsigned char g, unsigned char b);
	void		_ClampColor(double& v);
	double		_L2rgb(const unsigned char a[3], const unsigned char b[3]);

	IndexedBitmap::LinearGradient
		_DetectForPath(int layerIndex,
					   const std::vector<std::vector<double>>& segments,
					   const IndexedBitmap& indexed,
					   const BitmapData& src,
					   const TracingOptions& options);
};

#endif // GRADIENT_DETECTOR_H
