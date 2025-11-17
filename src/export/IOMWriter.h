/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef EXPORT_IOM_WRITER_H
#define EXPORT_IOM_WRITER_H

#include <string>
#include <vector>

#include "IOMStructures.h"
#include "BMessage.h"

namespace iom {

class IOMWriter {
public:
			IOMWriter() {};
			~IOMWriter() {};

	bool	WriteToFile(const std::string& filename, const Icon& icon);
	bool	WriteToBuffer(std::vector<uint8_t>& buffer, const Icon& icon);

private:
	void	_BuildMessage(haiku_compat::BMessage& msg, const Icon& icon);
	void	_AddStylesToMessage(haiku_compat::BMessage& msg, const Icon& icon);
	void	_AddPathsToMessage(haiku_compat::BMessage& msg, const Icon& icon);
	void	_AddShapesToMessage(haiku_compat::BMessage& msg, const Icon& icon);
	
	void	_AddStyleToMessage(haiku_compat::BMessage& container, const Style& style, int index);
	void	_AddPathToMessage(haiku_compat::BMessage& container, const Path& path, int index);
	void	_AddShapeToMessage(haiku_compat::BMessage& container, const Shape& shape, int index);
	void	_AddGradientToMessage(haiku_compat::BMessage& msg, const Gradient& grad);
	void	_AddTransformerToMessage(haiku_compat::BMessage& msg, const Transformer& trans);
};

}

#endif
