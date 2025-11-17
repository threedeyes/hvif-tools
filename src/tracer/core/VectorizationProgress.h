/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef VECTORIZATION_PROGRESS_H
#define VECTORIZATION_PROGRESS_H

enum VectorizationStage {
	STAGE_STARTING = 0,
	STAGE_REMOVE_BACKGROUND = 1,
	STAGE_BLUR = 2,
	STAGE_CREATE_PALETTE = 3,
	STAGE_QUANTIZE_COLORS = 4,
	STAGE_MERGE_REGIONS = 5,
	STAGE_SCAN_PATHS = 6,
	STAGE_TRACE_PATHS = 7,
	STAGE_SIMPLIFY_VW = 8,
	STAGE_FILTER_SMALL = 9,
	STAGE_SIMPLIFY_DP = 10,
	STAGE_SIMPLIFY_ADVANCED = 11,
	STAGE_DETECT_GEOMETRY = 12,
	STAGE_UNIFY_EDGES = 13,
	STAGE_FIX_WINDING = 14,
	STAGE_DETECT_GRADIENTS = 15,
	STAGE_COMPLETE = 16
};

typedef void (*ProgressCallback)(int stage, int percent, void* userData);

#endif
