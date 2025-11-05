/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef UTILS_H
#define UTILS_H

#include <algorithm>

namespace utils {

template<typename T>
inline T clamp(T value, T min_val, T max_val) {
	return std::max(min_val, std::min(value, max_val));
}

}

#endif
