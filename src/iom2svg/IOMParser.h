/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#ifndef IOM_PARSER_H
#define IOM_PARSER_H

#include "IOMStructures.h"
#include "BMessage.h"

namespace iom {

class IOMParser {
public:
	IOMParser();
	~IOMParser();

	bool				ParseFile(const std::string& filename);
	bool				ParseMessage(const BMessage& message);
	
	const Icon&			GetIcon() const { return *fIcon; }
	Icon* 				TakeIcon() { Icon* icon = fIcon; fIcon = NULL; return icon; }
	const std::string&	GetLastError() const { return fLastError; }

private:
	bool				_ParseMessage(const BMessage& message);
	bool				_ParseStyle(const BMessage& styleMsg, Style& style);
	bool				_ParseGradient(const BMessage& gradMsg, Gradient& gradient);
	bool				_ParsePath(const BMessage& pathMsg, Path& path);
	bool				_ParseShape(const BMessage& shapeMsg, Shape& shape);
	bool				_ParseTransformer(const BMessage& transMsg, Transformer& transformer);

	void				_SetError(const std::string& error);

	Icon*				fIcon;
	std::string			fLastError;
};

}

#endif
