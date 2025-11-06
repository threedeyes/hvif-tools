/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef COLOR_NODE_H
#define COLOR_NODE_H

#include <vector>

class ColorCube;
struct ColorSearchResult;

class ColorNode {
public:
							ColorNode(ColorCube* cube);
							ColorNode(ColorNode* parent, int nodeId, int nodeLevel);
							~ColorNode();

	void					PruneChild();
	void					PruneLevel();
	int						Reduce(int threshold, int nextThreshold);
	void					CreateColormap();
	void					FindClosestColor(int red, int green, int blue, int alpha,
											ColorSearchResult& search);

	ColorNode*				GetChild(int index) const;
	ColorNode*				GetParent() const { return fParent; }
	int						GetMidRed() const { return fMidRed; }
	int						GetMidGreen() const { return fMidGreen; }
	int						GetMidBlue() const { return fMidBlue; }
	int						GetMidAlpha() const { return fMidAlpha; }

	void					SetChild(int index, ColorNode* child);

	void					AddPixelCount(int count) { fNumberPixels += count; }
	void					IncrementUniqueCount() { ++fUniqueCount; }
	void					AddColor(int red, int green, int blue, int alpha);

	static int				GetShift(int level);

private:
	ColorCube*				fCube;
	ColorNode*				fParent;
	std::vector<ColorNode*>	fChildren;
	int						fChildCount;

	int						fId;
	int						fLevel;
	int						fMidRed;
	int						fMidGreen;
	int						fMidBlue;
	int						fMidAlpha;

	int						fNumberPixels;
	int						fUniqueCount;
	int						fTotalRed;
	int						fTotalGreen;
	int						fTotalBlue;
	int						fTotalAlpha;
	int						fColorNumber;
};

#endif
