/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <cmath>
#include <cfloat>
#include <algorithm>

#include "GradientDetector.h"
#include "MathUtils.h"

static inline double sqr(double v) { return v * v; }

GradientDetector::GradientDetector() {}
GradientDetector::~GradientDetector() {}

double
GradientDetector::_Luma(unsigned char r, unsigned char g, unsigned char b)
{
	return MathUtils::LumaD((double)r, (double)g, (double)b);
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

void
GradientDetector::_Bounds(const std::vector<std::vector<double> >& pts,
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
								  const std::vector<std::vector<double> >& poly)
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
GradientDetector::_FlattenPath(const std::vector<std::vector<double> >& segments,
							   std::vector<std::vector<double> >& outPoints,
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

double
GradientDetector::_PointSegmentDistance(double px, double py, double x1, double y1, double x2, double y2)
{
	double vx = x2 - x1;
	double vy = y2 - y1;
	double wx = px - x1;
	double wy = py - y1;

	double vv = vx*vx + vy*vy;
	if (vv <= 1e-12) {
		double dx = px - x1;
		double dy = py - y1;
		return std::sqrt(dx*dx + dy*dy);
	}
	double t = (wx*vx + wy*vy) / vv;
	if (t < 0.0) t = 0.0;
	else if (t > 1.0) t = 1.0;
	double cx = x1 + t * vx;
	double cy = y1 + t * vy;
	double dx = px - cx;
	double dy = py - cy;
	return std::sqrt(dx*dx + dy*dy);
}

double
GradientDetector::_DistanceToPolygon(double px, double py, const std::vector<std::vector<double> >& poly)
{
	if (poly.size() < 2) return 0.0;
	double best = DBL_MAX;
	for (size_t i = 0, j = 1; j < poly.size(); i = j, j++) {
		double d = _PointSegmentDistance(px, py, poly[i][0], poly[i][1], poly[j][0], poly[j][1]);
		if (d < best) best = d;
	}
	return best;
}

double
GradientDetector::_ComputeVariance(const std::vector<double>& vals, const std::vector<double>& weights)
{
	if (vals.empty()) return 0.0;

	double sumW = 0.0, sumVal = 0.0;
	for (size_t i = 0; i < vals.size(); i++) {
		double w = (i < weights.size()) ? weights[i] : 1.0;
		sumW += w;
		sumVal += w * vals[i];
	}

	if (sumW < 1e-12) return 0.0;
	double mean = sumVal / sumW;

	double variance = 0.0;
	for (size_t i = 0; i < vals.size(); i++) {
		double w = (i < weights.size()) ? weights[i] : 1.0;
		double dev = vals[i] - mean;
		variance += w * dev * dev;
	}

	return variance / sumW;
}

bool
GradientDetector::_ComputeChannelGradient(const std::vector<double>& vx,
										  const std::vector<double>& vy,
										  const std::vector<double>& vw,
										  const std::vector<double>& vals,
										  double& outGradX,
										  double& outGradY,
										  double& outR2)
{
	if (vx.size() < 10) return false;

	double minX = vx[0], maxX = vx[0];
	double minY = vy[0], maxY = vy[0];
	for (size_t i = 1; i < vx.size(); i++) {
		if (vx[i] < minX) minX = vx[i];
		if (vx[i] > maxX) maxX = vx[i];
		if (vy[i] < minY) minY = vy[i];
		if (vy[i] > maxY) maxY = vy[i];
	}
	double rangeX = maxX - minX;
	double rangeY = maxY - minY;
	if (rangeX < 1e-6) rangeX = 1.0;
	if (rangeY < 1e-6) rangeY = 1.0;

	std::vector<double> normX(vx.size());
	std::vector<double> normY(vy.size());
	for (size_t i = 0; i < vx.size(); i++) {
		normX[i] = (vx[i] - minX) / rangeX;
		normY[i] = (vy[i] - minY) / rangeY;
	}

	double W = 0.0, Wx = 0.0, Wy = 0.0, Wxx = 0.0, Wyy = 0.0, Wxy = 0.0;
	double Wv = 0.0, Wvx = 0.0, Wvy = 0.0;

	for (size_t i = 0; i < normX.size(); i++) {
		double w = vw[i];
		double x = normX[i];
		double y = normY[i];
		double v = vals[i];

		W   += w;
		Wx  += w * x;
		Wy  += w * y;
		Wxx += w * x * x;
		Wyy += w * y * y;
		Wxy += w * x * y;

		Wv  += w * v;
		Wvx += w * v * x;
		Wvy += w * v * y;
	}

	double M[3][3] = { { W,   Wx,  Wy },
					   { Wx,  Wxx, Wxy },
					   { Wy,  Wxy, Wyy } };

	double B[3] = { Wv, Wvx, Wvy };
	double X[3] = { 0, 0, 0 };

	if (!MathUtils::Solve3x3Normalized(M, B, X))
		return false;

	outGradX = X[1] / rangeX;
	outGradY = X[2] / rangeY;

	double mean_v = Wv / W;
	double ss_tot = 0.0, ss_res = 0.0;

	for (size_t i = 0; i < normX.size(); i++) {
		double w = vw[i];
		double predicted = X[0] + X[1] * normX[i] + X[2] * normY[i];

		ss_tot += w * (vals[i] - mean_v) * (vals[i] - mean_v);
		ss_res += w * (vals[i] - predicted) * (vals[i] - predicted);
	}

	outR2 = (ss_tot > 1e-12) ? (1.0 - ss_res / ss_tot) : 0.0;

	return true;
}

bool
GradientDetector::_ComputeRobustDirection(const std::vector<double>& vx,
										  const std::vector<double>& vy,
										  const std::vector<double>& vw,
										  const std::vector<double>& vr,
										  const std::vector<double>& vg,
										  const std::vector<double>& vb,
										  double& outDirX,
										  double& outDirY,
										  double& outConfidence)
{
	double gradRx = 0.0, gradRy = 0.0, R2r = 0.0;
	double gradGx = 0.0, gradGy = 0.0, R2g = 0.0;
	double gradBx = 0.0, gradBy = 0.0, R2b = 0.0;

	bool okR = _ComputeChannelGradient(vx, vy, vw, vr, gradRx, gradRy, R2r);
	bool okG = _ComputeChannelGradient(vx, vy, vw, vg, gradGx, gradGy, R2g);
	bool okB = _ComputeChannelGradient(vx, vy, vw, vb, gradBx, gradBy, R2b);

	if (!okR && !okG && !okB)
		return false;

	double varR = _ComputeVariance(vr, vw);
	double varG = _ComputeVariance(vg, vw);
	double varB = _ComputeVariance(vb, vw);

	double magR = std::sqrt(gradRx * gradRx + gradRy * gradRy);
	double magG = std::sqrt(gradGx * gradGx + gradGy * gradGy);
	double magB = std::sqrt(gradBx * gradBx + gradBy * gradBy);

	double reliabilityR = okR ? (R2r * varR * magR) : 0.0;
	double reliabilityG = okG ? (R2g * varG * magG) : 0.0;
	double reliabilityB = okB ? (R2b * varB * magB) : 0.0;

	double totalReliability = reliabilityR + reliabilityG + reliabilityB;
	if (totalReliability < 1e-6)
		return false;

	double weightR = reliabilityR / totalReliability;
	double weightG = reliabilityG / totalReliability;
	double weightB = reliabilityB / totalReliability;

	outDirX = 0.0;
	outDirY = 0.0;

	std::vector<double> directions[3][2];
	int validChannels = 0;

	if (okR && magR > 1e-6) {
		double ux = gradRx / magR;
		double uy = gradRy / magR;
		directions[validChannels][0].push_back(ux);
		directions[validChannels][1].push_back(uy);
		validChannels++;
	}
	if (okG && magG > 1e-6) {
		double ux = gradGx / magG;
		double uy = gradGy / magG;
		directions[validChannels][0].push_back(ux);
		directions[validChannels][1].push_back(uy);
		validChannels++;
	}
	if (okB && magB > 1e-6) {
		double ux = gradBx / magB;
		double uy = gradBy / magB;
		directions[validChannels][0].push_back(ux);
		directions[validChannels][1].push_back(uy);
		validChannels++;
	}

	if (validChannels >= 2) {
		double dot01 = directions[0][0][0] * directions[1][0][0] +
					   directions[0][1][0] * directions[1][1][0];
		if (dot01 < 0) {
			directions[1][0][0] = -directions[1][0][0];
			directions[1][1][0] = -directions[1][1][0];
		}

		if (validChannels >= 3) {
			double dot02 = directions[0][0][0] * directions[2][0][0] +
						   directions[0][1][0] * directions[2][1][0];
			if (dot02 < 0) {
				directions[2][0][0] = -directions[2][0][0];
				directions[2][1][0] = -directions[2][1][0];
			}
		}
	}

	if (okR && magR > 1e-6) {
		double ux = (validChannels > 0) ? directions[0][0][0] : gradRx / magR;
		double uy = (validChannels > 0) ? directions[0][1][0] : gradRy / magR;
		outDirX += weightR * ux;
		outDirY += weightR * uy;
	}
	if (okG && magG > 1e-6) {
		double ux = (validChannels > 1) ? directions[1][0][0] : gradGx / magG;
		double uy = (validChannels > 1) ? directions[1][1][0] : gradGy / magG;
		outDirX += weightG * ux;
		outDirY += weightG * uy;
	}
	if (okB && magB > 1e-6) {
		double ux = (validChannels > 2) ? directions[2][0][0] : gradBx / magB;
		double uy = (validChannels > 2) ? directions[2][1][0] : gradBy / magB;
		outDirX += weightB * ux;
		outDirY += weightB * uy;
	}

	double normDir = std::sqrt(outDirX * outDirX + outDirY * outDirY);
	if (normDir < 1e-8)
		return false;

	outDirX /= normDir;
	outDirY /= normDir;

	double consensusScore = 0.0;

	if (okR && magR > 1e-6) {
		double ux = gradRx / magR;
		double uy = gradRy / magR;
		double dot = ux * outDirX + uy * outDirY;
		consensusScore += std::fabs(dot) * weightR;
	}
	if (okG && magG > 1e-6) {
		double ux = gradGx / magG;
		double uy = gradGy / magG;
		double dot = ux * outDirX + uy * outDirY;
		consensusScore += std::fabs(dot) * weightG;
	}
	if (okB && magB > 1e-6) {
		double ux = gradBx / magB;
		double uy = gradBy / magB;
		double dot = ux * outDirX + uy * outDirY;
		consensusScore += std::fabs(dot) * weightB;
	}

	double avgR2 = (weightR * R2r + weightG * R2g + weightB * R2b);

	outConfidence = consensusScore * avgR2;

	if (validChannels < 2 && outConfidence < 0.5)
		return false;

	return true;
}

IndexedBitmap::LinearGradient
GradientDetector::_DetectForPath(int layerIndex,
								 const std::vector<std::vector<double> >& segments,
								 const IndexedBitmap& indexed,
								 const BitmapData& src,
								 const TracingOptions& options)
{
	IndexedBitmap::LinearGradient result;

	std::vector<std::vector<double> > poly;
	_FlattenPath(segments, poly, options.fGradientMaxSubdiv);
	if (poly.size() < 4) return result;

	double minX, minY, maxX, maxY;
	_Bounds(poly, minX, minY, maxX, maxY);

	double regionSize = std::max(maxX - minX, maxY - minY);
	double minSizeThreshold = MathUtils::AdaptiveThreshold(indexed.Palette().size(), options.fGradientMinSize);

	if (regionSize < minSizeThreshold)
		return result;

	int xs = (int)std::floor(minX); if (xs < 0) xs = 0;
	int ys = (int)std::floor(minY); if (ys < 0) ys = 0;
	int xe = (int)std::ceil(maxX);  if (xe > src.Width()-1) xe = src.Width()-1;
	int ye = (int)std::ceil(maxY);  if (ye > src.Height()-1) ye = src.Height()-1;

	double regionArea = (maxX - minX) * (maxY - minY);
	int adaptiveStride = options.fGradientSampleStride;
	if (regionArea > 10000) {
		adaptiveStride = 3;
	} else if (regionArea > 2500) {
		adaptiveStride = 2;
	} else {
		adaptiveStride = 1;
	}

	std::vector<double> vx, vy, vw;
	std::vector<double> vr, vg, vb, va;

	for (int y = ys; y <= ye; y += adaptiveStride) {
		for (int x = xs; x <= xe; x += adaptiveStride) {
			double px = (double)x + 0.5;
			double py = (double)y + 0.5;

			if (!_PointInPolygon(px, py, poly))
				continue;

			unsigned char r8 = src.GetPixelComponent(x, y, 0);
			unsigned char g8 = src.GetPixelComponent(x, y, 1);
			unsigned char b8 = src.GetPixelComponent(x, y, 2);
			unsigned char a8 = src.GetPixelComponent(x, y, 3);

			if (MathUtils::IsTransparent(a8))
				continue;

			double dist = _DistanceToPolygon(px, py, poly);
			double wBorder = dist / 3.0;
			if (wBorder > 1.0) wBorder = 1.0;
			if (wBorder < 0.1) wBorder = 0.1;
			double wAlpha = (double)a8 / 255.0;
			double w = wBorder * wBorder * wAlpha;

			double r = (double)r8;
			double g = (double)g8;
			double b = (double)b8;
			double a = (double)a8;

			if (options.fGradientUseLinearRGB) {
				r = MathUtils::SRGBToLinear(r);
				g = MathUtils::SRGBToLinear(g);
				b = MathUtils::SRGBToLinear(b);
			}

			vx.push_back((double)x);
			vy.push_back((double)y);
			vw.push_back(w);
			vr.push_back(r);
			vg.push_back(g);
			vb.push_back(b);
			va.push_back(a);
		}
	}

	int minSamples = options.fGradientMinSamples;
	if ((int)vx.size() < minSamples)
		return result;

	double dirX = 0.0, dirY = 0.0, confidence = 0.0;
	if (!_ComputeRobustDirection(vx, vy, vw, vr, vg, vb, dirX, dirY, confidence))
		return result;

	double minConfidence = 0.3;
	if (confidence < minConfidence)
		return result;

	std::vector<double> tvals(vx.size());
	double Wt = 0.0, Wtt = 0.0, W = 0.0;
	for (size_t i = 0; i < vx.size(); i++) {
		double t = vx[i] * dirX + vy[i] * dirY;
		tvals[i] = t;
		double w = vw[i];
		W   += w;
		Wt  += w * t;
		Wtt += w * t * t;
	}

	double denom = W * Wtt - Wt * Wt;
	if (std::fabs(denom) < 1e-12)
		return result;

	double Wr = 0.0, Wrt = 0.0;
	double Wg = 0.0, Wgt = 0.0;
	double Wb = 0.0, Wbt = 0.0;
	double Wa = 0.0, Wat = 0.0;

	for (size_t i = 0; i < vx.size(); i++) {
		double w = vw[i];
		double t = tvals[i];
		double r = vr[i], g = vg[i], b = vb[i], a = va[i];

		Wr += w * r; Wrt += w * r * t;
		Wg += w * g; Wgt += w * g * t;
		Wb += w * b; Wbt += w * b * t;
		Wa += w * a; Wat += w * a * t;
	}

	double br = (W * Wrt - Wt * Wr) / denom;
	double ar = (Wr - br * Wt) / W;

	double bg = (W * Wgt - Wt * Wg) / denom;
	double ag = (Wg - bg * Wt) / W;

	double bb = (W * Wbt - Wt * Wb) / denom;
	double ab = (Wb - bb * Wt) / W;

	double ba = (W * Wat - Wt * Wa) / denom;
	double aa = (Wa - ba * Wt) / W;

	double mean_r = Wr / W;
	double mean_g = Wg / W;
	double mean_b = Wb / W;

	double ss_tot_r = 0.0, ss_res_r = 0.0;
	double ss_tot_g = 0.0, ss_res_g = 0.0;
	double ss_tot_b = 0.0, ss_res_b = 0.0;

	for (size_t i = 0; i < vx.size(); i++) {
		double w = vw[i];
		double t = tvals[i];

		double pr = ar + br * t;
		double pg = ag + bg * t;
		double pb = ab + bb * t;

		ss_tot_r += w * (vr[i] - mean_r) * (vr[i] - mean_r);
		ss_tot_g += w * (vg[i] - mean_g) * (vg[i] - mean_g);
		ss_tot_b += w * (vb[i] - mean_b) * (vb[i] - mean_b);

		ss_res_r += w * (vr[i] - pr) * (vr[i] - pr);
		ss_res_g += w * (vg[i] - pg) * (vg[i] - pg);
		ss_res_b += w * (vb[i] - pb) * (vb[i] - pb);
	}

	double R2r = (ss_tot_r <= 1e-12) ? 0.0 : (1.0 - ss_res_r / ss_tot_r);
	double R2g = (ss_tot_g <= 1e-12) ? 0.0 : (1.0 - ss_res_g / ss_tot_g);
	double R2b = (ss_tot_b <= 1e-12) ? 0.0 : (1.0 - ss_res_b / ss_tot_b);

	double wsum = 0.0;
	double R2total = 0.0;
	if (ss_tot_r > 1e-12) { R2total += R2r * ss_tot_r; wsum += ss_tot_r; }
	if (ss_tot_g > 1e-12) { R2total += R2g * ss_tot_g; wsum += ss_tot_g; }
	if (ss_tot_b > 1e-12) { R2total += R2b * ss_tot_b; wsum += ss_tot_b; }
	if (wsum > 0.0) R2total /= wsum; else R2total = 0.0;

	double minR2Total = (options.fGradientMinR2 > options.fGradientMinR2Total)
						? options.fGradientMinR2
						: options.fGradientMinR2Total;

	if (!(R2total >= (double)minR2Total))
		return result;

	double tmin_samples = DBL_MAX, tmax_samples = -DBL_MAX;
	for (size_t i = 0; i < tvals.size(); i++) {
		if (tvals[i] < tmin_samples) tmin_samples = tvals[i];
		if (tvals[i] > tmax_samples) tmax_samples = tvals[i];
	}

	double gradientLength = tmax_samples - tmin_samples;
	if (gradientLength < minSizeThreshold)
		return result;

	double t1c = tmin_samples;
	double t2c = tmax_samples;

	double r1 = ar + br * t1c, g1 = ag + bg * t1c, b1 = ab + bb * t1c, a1 = aa + ba * t1c;
	double r2 = ar + br * t2c, g2 = ag + bg * t2c, b2 = ab + bb * t2c, a2 = aa + ba * t2c;

	if (options.fGradientUseLinearRGB) {
		r1 = MathUtils::LinearToSRGB(r1); g1 = MathUtils::LinearToSRGB(g1);	b1 = MathUtils::LinearToSRGB(b1);
		r2 = MathUtils::LinearToSRGB(r2); g2 = MathUtils::LinearToSRGB(g2); b2 = MathUtils::LinearToSRGB(b2);
	}

	_ClampColor(r1); _ClampColor(g1); _ClampColor(b1); _ClampColor(a1);
	_ClampColor(r2); _ClampColor(g2); _ClampColor(b2); _ClampColor(a2);

	unsigned char cstart[4] = { (unsigned char)(r1 + 0.5), (unsigned char)(g1 + 0.5), (unsigned char)(b1 + 0.5), (unsigned char)(a1 + 0.5) };
	unsigned char cend[4]   = { (unsigned char)(r2 + 0.5), (unsigned char)(g2 + 0.5), (unsigned char)(b2 + 0.5), (unsigned char)(a2 + 0.5) };

	unsigned char cstartRGB[3] = { cstart[0], cstart[1], cstart[2] };
	unsigned char cendRGB[3]   = { cend[0], cend[1], cend[2] };

	double minDelta = MathUtils::AdaptiveThreshold(indexed.Palette().size(), options.fGradientMinDelta);
	if (_L2rgb(cstartRGB, cendRGB) < minDelta)
		return result;

	int tmin_poly_idx = -1, tmax_poly_idx = -1;
	double tmin_poly = DBL_MAX, tmax_poly = -DBL_MAX;
	for (size_t i = 0; i < poly.size(); i++) {
		double t = poly[i][0] * dirX + poly[i][1] * dirY;
		if (t < tmin_poly) { tmin_poly = t; tmin_poly_idx = (int)i; }
		if (t > tmax_poly) { tmax_poly = t; tmax_poly_idx = (int)i; }
	}

	if (tmin_poly_idx < 0 || tmax_poly_idx < 0)
		return result;

	double x1 = poly[tmin_poly_idx][0];
	double y1 = poly[tmin_poly_idx][1];
	double x2 = poly[tmax_poly_idx][0];
	double y2 = poly[tmax_poly_idx][1];

	result.valid = true;
	result.x1 = x1; result.y1 = y1;
	result.x2 = x2; result.y2 = y2;
	result.c1[0] = cstart[0]; result.c1[1] = cstart[1]; result.c1[2] = cstart[2]; result.c1[3] = cstart[3];
	result.c2[0] = cend[0];   result.c2[1] = cend[1];   result.c2[2] = cend[2];   result.c2[3] = cend[3];

	return result;
}

std::vector<std::vector<IndexedBitmap::LinearGradient> >
GradientDetector::DetectLinearGradients(const IndexedBitmap& indexed,
										const BitmapData& sourceBitmap,
										const std::vector<std::vector<std::vector<std::vector<double> > > >& layers,
										const TracingOptions& options)
{
	std::vector<std::vector<IndexedBitmap::LinearGradient> > out;
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
