/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IOM_WRITER_H
#define IOM_WRITER_H

#include <string>
#include <vector>

#include "IOMStructures.h"
#include "BMessage.h"

namespace iom {

class IOMWriter {
public:
			IOMWriter();
			~IOMWriter();

	bool	WriteToFile(const std::string& filename, const Icon& icon);
	bool	WriteToBuffer(std::vector<uint8_t>& buffer, const Icon& icon);

private:
	void	_BuildMessage(BMessage& msg, const Icon& icon);
	void	_AddStylesToMessage(BMessage& msg, const Icon& icon);
	void	_AddPathsToMessage(BMessage& msg, const Icon& icon);
	void	_AddShapesToMessage(BMessage& msg, const Icon& icon);
	
	void	_AddStyleToMessage(BMessage& container, const Style& style, int index);
	void	_AddPathToMessage(BMessage& container, const Path& path, int index);
	void	_AddShapeToMessage(BMessage& container, const Shape& shape, int index);
	void	_AddGradientToMessage(BMessage& msg, const Gradient& grad);
	void	_AddTransformerToMessage(BMessage& msg, const Transformer& trans);
};

}

#endif
