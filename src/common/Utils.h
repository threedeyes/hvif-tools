/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef UTILS_H
#define UTILS_H

#include <algorithm>
#include <string>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <stdint.h>

namespace utils {

template<typename T>
inline T clamp(T value, T min_val, T max_val) {
	return std::max(min_val, std::min(value, max_val));
}

template<typename T>
inline std::string ToString(T value) {
	std::ostringstream oss;
	oss << value;
	return oss.str();
}

inline std::string FormatFixed(double value, int precision) {
	std::ostringstream oss;
	oss.setf(std::ios::fixed, std::ios::floatfield);
	oss.precision(precision);
	oss << value;
	return oss.str();
}

inline long RoundToLong(double x) {
	return static_cast<long>(x + (x >= 0 ? 0.5 : -0.5));
}

inline bool FloatEqual(float a, float b, float epsilon = 1e-3f) {
	return std::fabs(a - b) < epsilon;
}

inline bool DoubleEqual(double a, double b, double epsilon = 1e-6) {
	return std::fabs(a - b) < epsilon;
}

inline uint8_t MapCapFromNanoSVG(int nsvgCap) {
	switch (nsvgCap) {
		case 0: return 0;
		case 1: return 2;
		case 2: return 1;
		default: return 0;
	}
}

inline uint8_t MapJoinFromNanoSVG(int nsvgJoin) {
	switch (nsvgJoin) {
		case 0: return 0;
		case 1: return 2;
		case 2: return 3;
		default: return 0;
	}
}

inline void InvertAffine(float out[6], const float in[6]) {
	float a = in[0];
	float b = in[1];
	float c = in[2];
	float d = in[3];
	float e = in[4];
	float f = in[5];

	float det = a * d - b * c;
	if (det < 1e-12f && det > -1e-12f) {
		out[0] = 1; out[1] = 0; out[2] = 0;
		out[3] = 1; out[4] = 0; out[5] = 0;
		return;
	}

	float invDet = 1.0f / det;
	out[0] =  d * invDet;
	out[1] = -b * invDet;
	out[2] = -c * invDet;
	out[3] =  a * invDet;
	out[4] = (c * f - d * e) * invDet;
	out[5] = (b * e - a * f) * invDet;
}

inline std::string GetLineJoinName(int lineJoin) {
	switch (lineJoin) {
		case 0: return "miter";
		case 1: return "miter";
		case 2: return "round";
		case 3: return "bevel";
		case 4: return "miter";
		default: return "miter";
	}
}

inline std::string GetLineCapName(int lineCap) {
	switch (lineCap) {
		case 0: return "butt";
		case 1: return "square";
		case 2: return "round";
		default: return "butt";
	}
}

}

#endif
