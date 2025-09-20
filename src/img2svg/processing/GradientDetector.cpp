/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <cfloat>
#include <algorithm>

#include "GradientDetector.h"

static inline double sqr(double v) { return v * v; }

GradientDetector::GradientDetector() {}
GradientDetector::~GradientDetector() {}

double
GradientDetector::_Luma(unsigned char r, unsigned char g, unsigned char b)
{
	return 0.2126 * (double)r + 0.7152 * (double)g + 0.0722 * (double)b;
}

void
GradientDetector::_ClampColor(double& v)
{
	if (v < 0.0) v = 0.0;
	else if (v > 255.0) v = 255.0;
}

double
GradientDetector::_L2rgb(const unsigned char a[3], const unsigned char b[3])
{
	double dr = (double)a[0] - (double)b[0];
	double dg = (double)a[1] - (double)b[1];
	double db = (double)a[2] - (double)b[2];
	return std::sqrt(dr*dr + dg*dg + db*db);
}

bool
GradientDetector::_Solve3x3(double M[3][3], double B[3], double X[3])
{
	int i, j, k;
	double A[3][4];
	for (i = 0; i < 3; i++) {
		for (j = 0; j < 3; j++) A[i][j] = M[i][j];
		A[i][3] = B[i];
	}
	for (i = 0; i < 3; i++) {
		int piv = i;
		double maxabs = std::fabs(A[i][i]);
		for (k = i + 1; k < 3; k++) {
			double v = std::fabs(A[k][i]);
			if (v > maxabs) { maxabs = v; piv = k; }
		}
		if (maxabs < 1e-12) return false;
		if (piv != i) {
			for (j = i; j < 4; j++) {
				double tmp = A[i][j]; A[i][j] = A[piv][j]; A[piv][j] = tmp;
			}
		}
		double diag = A[i][i];
		for (j = i; j < 4; j++) A[i][j] /= diag;
		for (k = 0; k < 3; k++) if (k != i) {
			double f = A[k][i];
			for (j = i; j < 4; j++) A[k][j] -= f * A[i][j];
		}
	}
	for (i = 0; i < 3; i++) X[i] = A[i][3];
	return true;
}

void
GradientDetector::_Bounds(const std::vector<std::vector<double>>& pts,
						  double& minX, double& minY, double& maxX, double& maxY)
{
	if (pts.empty()) { minX = minY = maxX = maxY = 0; return; }
	minX = maxX = pts[0][0];
	minY = maxY = pts[0][1];
	for (size_t i = 1; i < pts.size(); i++) {
		if (pts[i][0] < minX) minX = pts[i][0];
		if (pts[i][0] > maxX) maxX = pts[i][0];
		if (pts[i][1] < minY) minY = pts[i][1];
		if (pts[i][1] > maxY) maxY = pts[i][1];
	}
}

bool
GradientDetector::_PointInPolygon(double x, double y,
								  const std::vector<std::vector<double>>& poly)
{
	bool inside = false;
	size_t n = poly.size();
	if (n == 0) return false;
	for (size_t i = 0, j = n - 1; i < n; j = i++) {
		double xi = poly[i][0], yi = poly[i][1];
		double xj = poly[j][0], yj = poly[j][1];
		bool cond = ((yi > y) != (yj > y));
		double denom = (yj - yi);
		if (denom == 0.0) denom = 1e-12;
		double xint = (xj - xi) * (y - yi) / denom + xi;
		if (cond && (x < xint)) inside = !inside;
	}
	return inside;
}

void
GradientDetector::_FlattenPath(const std::vector<std::vector<double>>& segments,
							   std::vector<std::vector<double>>& outPoints,
							   int maxSubdiv)
{
	outPoints.clear();
	for (size_t s = 0; s < segments.size(); s++) {
		const std::vector<double>& seg = segments[s];
		if (seg.size() < 4) continue;
		int type = (int)seg[0];
		if (type == 1) {
			if (outPoints.empty()) {
				std::vector<double> p(2);
				p[0] = seg[1]; p[1] = seg[2];
				outPoints.push_back(p);
			}
			std::vector<double> q(2);
			q[0] = seg[3]; q[1] = seg[4];
			outPoints.push_back(q);
		} else {
			int n = (maxSubdiv > 1) ? maxSubdiv : 1;
			double x1 = seg[1], y1 = seg[2];
			double cx = seg[3], cy = seg[4];
			double x2 = seg[5], y2 = seg[6];
			for (int i = 0; i <= n; i++) {
				double t = (double)i / (double)n;
				double it = 1.0 - t;
				double x = it*it*x1 + 2.0*it*t*cx + t*t*x2;
				double y = it*it*y1 + 2.0*it*t*cy + t*t*y2;
				if (outPoints.empty() || i > 0) {
					std::vector<double> p(2);
					p[0] = x; p[1] = y;
					outPoints.push_back(p);
				}
			}
		}
	}
	if (!outPoints.empty()) {
		const std::vector<double>& a = outPoints.front();
		const std::vector<double>& b = outPoints.back();
		if (std::fabs(a[0]-b[0]) + std::fabs(a[1]-b[1]) > 1e-6) {
			outPoints.push_back(outPoints.front());
		}
	}
}

IndexedBitmap::LinearGradient
GradientDetector::_DetectForPath(int layerIndex,
								 const std::vector<std::vector<double>>& segments,
								 const IndexedBitmap& indexed,
								 const BitmapData& src,
								 const TracingOptions& options)
{
	IndexedBitmap::LinearGradient result;

	std::vector<std::vector<double>> poly;
	_FlattenPath(segments, poly, options.fGradientMaxSubdiv);
	if (poly.size() < 4) return result;

	double minX, minY, maxX, maxY;
	_Bounds(poly, minX, minY, maxX, maxY);
	if ((maxX - minX) < options.fGradientMinSize && (maxY - minY) < options.fGradientMinSize)
		return result;

	int stride = options.fGradientSampleStride;
	if (stride < 1) stride = 1;

	int xs = (int)std::floor(minX); if (xs < 0) xs = 0;
	int ys = (int)std::floor(minY); if (ys < 0) ys = 0;
	int xe = (int)std::ceil(maxX);  if (xe > src.Width()-1) xe = src.Width()-1;
	int ye = (int)std::ceil(maxY);  if (ye > src.Height()-1) ye = src.Height()-1;

	std::vector<double> vx, vy, vl;
	std::vector<unsigned char> vr, vg, vb, va;

	for (int y = ys; y <= ye; y += stride) {
		for (int x = xs; x <= xe; x += stride) {
			if (!_PointInPolygon((double)x + 0.5, (double)y + 0.5, poly))
				continue;
			// Account for 1-pixel border in indexed array
			if (y + 1 < 0 || y + 1 >= (int)indexed.Array().size()) continue;
			if (x + 1 < 0 || x + 1 >= (int)indexed.Array()[0].size()) continue;
			if (indexed.Array()[y + 1][x + 1] != layerIndex) continue;

			unsigned char r = src.GetPixelComponent(x, y, 0);
			unsigned char g = src.GetPixelComponent(x, y, 1);
			unsigned char b = src.GetPixelComponent(x, y, 2);
			unsigned char a = src.GetPixelComponent(x, y, 3);
			if (a == 0) continue;

			vx.push_back((double)x);
			vy.push_back((double)y);
			vl.push_back(_Luma(r,g,b));
			vr.push_back(r); vg.push_back(g); vb.push_back(b); va.push_back(a);
		}
	}

	if ((int)vx.size() < options.fGradientMinSamples)
		return result;

	// Linear regression on luminance: s ≈ A + Bx x + By y
	double N = (double)vx.size();
	double Sx=0, Sy=0, Sxx=0, Syy=0, Sxy=0, Ss=0, Ssx=0, Ssy=0;
	for (size_t i = 0; i < vx.size(); i++) {
		double x = vx[i], y = vy[i], s = vl[i];
		Sx += x; Sy += y; Ss += s;
		Sxx += x*x; Syy += y*y; Sxy += x*y;
		Ssx += s*x; Ssy += s*y;
	}
	double M[3][3] = { { N,  Sx,  Sy },
	                   { Sx, Sxx, Sxy },
	                   { Sy, Sxy, Syy } };
	double B[3] = { Ss, Ssx, Ssy };
	double X[3] = { 0,0,0 };
	if (!_Solve3x3(M, B, X))
		return result;
	double A = X[0], Bx = X[1], By = X[2];

	double norm = std::sqrt(Bx*Bx + By*By);
	if (norm < 1e-6)
		return result;

	double vxn = Bx / norm;
	double vyn = By / norm;

	// t projections and per-channel regressions C ≈ a + b t
	double tmin = DBL_MAX, tmax = -DBL_MAX;
	double tsum = 0.0, t2sum = 0.0;
	std::vector<double> tv;
	tv.resize(vx.size());
	for (size_t i = 0; i < vx.size(); i++) {
		double t = vx[i]*vxn + vy[i]*vyn;
		tv[i] = t;
		if (t < tmin) tmin = t;
		if (t > tmax) tmax = t;
		tsum += t;
		t2sum += t*t;
	}
	double denom = N * t2sum - tsum * tsum;
	if (std::fabs(denom) < 1e-9)
		return result;

	// size along gradient axis
	if ((tmax - tmin) < options.fGradientMinSize)
		return result;

	// R^2 on luminance along t
	double ss_tot = 0.0;
	double smean = Ss / N;
	double ss_res = 0.0;
	{
		double cs = 0.0, cst = 0.0;
		for (size_t i = 0; i < tv.size(); i++) {
			double s = vl[i];
			ss_tot += sqr(s - smean);
			cs += s;
			cst += s * tv[i];
		}
		double bs = (N * cst - tsum * cs) / denom;
		double as = (cs - bs * tsum) / N;
		for (size_t i = 0; i < tv.size(); i++) {
			double s = vl[i];
			double sh = as + bs * tv[i];
			ss_res += sqr(s - sh);
		}
	}
	if (ss_tot <= 1e-12) return result;
	double R2 = 1.0 - ss_res / ss_tot;
	if (R2 < (double)options.fGradientMinR2)
		return result;

	double cr=0, cg=0, cb=0, ca=0, crt=0, cgt=0, cbt=0, cat=0;
	for (size_t i = 0; i < tv.size(); i++) {
		cr += (double)vr[i]; cg += (double)vg[i]; cb += (double)vb[i]; ca += (double)va[i];
		crt += (double)vr[i] * tv[i];
		cgt += (double)vg[i] * tv[i];
		cbt += (double)vb[i] * tv[i];
		cat += (double)va[i] * tv[i];
	}
	double br = (N * crt - tsum * cr) / denom;
	double bg = (N * cgt - tsum * cg) / denom;
	double bb = (N * cbt - tsum * cb) / denom;
	double ba = (N * cat - tsum * ca) / denom;

	double ar = (cr - br * tsum) / N;
	double ag = (cg - bg * tsum) / N;
	double ab = (cb - bb * tsum) / N;
	double aa = (ca - ba * tsum) / N;

	double r1 = ar + br * tmin, g1 = ag + bg * tmin, b1 = ab + bb * tmin, a1 = aa + ba * tmin;
	double r2 = ar + br * tmax, g2 = ag + bg * tmax, b2 = ab + bb * tmax, a2 = aa + ba * tmax;
	_ClampColor(r1); _ClampColor(g1); _ClampColor(b1); _ClampColor(a1);
	_ClampColor(r2); _ClampColor(g2); _ClampColor(b2); _ClampColor(a2);

	unsigned char cstart[4] = { (unsigned char)(r1+0.5), (unsigned char)(g1+0.5), (unsigned char)(b1+0.5), (unsigned char)(a1+0.5) };
	unsigned char cend[4]   = { (unsigned char)(r2+0.5), (unsigned char)(g2+0.5), (unsigned char)(b2+0.5), (unsigned char)(a2+0.5) };

	unsigned char cstartRGB[3] = { cstart[0], cstart[1], cstart[2] };
	unsigned char cendRGB[3]   = { cend[0], cend[1], cend[2] };
	if (_L2rgb(cstartRGB, cendRGB) < (double)options.fGradientMinDelta)
		return result;

	double tminBest = DBL_MAX, tmaxBest = -DBL_MAX;
	double x1=0,y1=0,x2=0,y2=0;
	for (size_t i = 0; i < vx.size(); i++) {
		double t = tv[i];
		if (t < tminBest) { tminBest = t; x1 = vx[i]; y1 = vy[i]; }
		if (t > tmaxBest) { tmaxBest = t; x2 = vx[i]; y2 = vy[i]; }
	}

	result.valid = true;
	result.x1 = x1; result.y1 = y1;
	result.x2 = x2; result.y2 = y2;
	result.c1[0] = cstart[0]; result.c1[1] = cstart[1]; result.c1[2] = cstart[2]; result.c1[3] = cstart[3];
	result.c2[0] = cend[0];   result.c2[1] = cend[1];   result.c2[2] = cend[2];   result.c2[3] = cend[3];

	return result;
}

std::vector<std::vector<IndexedBitmap::LinearGradient>>
GradientDetector::DetectLinearGradients(const IndexedBitmap& indexed,
										const BitmapData& sourceBitmap,
										const std::vector<std::vector<std::vector<std::vector<double>>>>& layers,
										const TracingOptions& options)
{
	std::vector<std::vector<IndexedBitmap::LinearGradient>> out;
	out.resize(layers.size());
	for (size_t k = 0; k < layers.size(); k++) {
		out[k].resize(layers[k].size());
		for (size_t i = 0; i < layers[k].size(); i++) {
			if (layers[k][i].empty()) {
				out[k][i] = IndexedBitmap::LinearGradient();
				continue;
			}
			out[k][i] = _DetectForPath((int)k, layers[k][i], indexed, sourceBitmap, options);
		}
	}
	return out;
}
