/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <sstream>
#include <cstring>

#include "nanosvg.h"
#include "nanosvgrast.h"

#ifdef __HAIKU__
#include <Application.h>
#include <Bitmap.h>
#include <BitmapStream.h>
#include <File.h>
#include <TranslatorRoster.h>
#include <TranslationUtils.h>
#include <DataIO.h>
#else
#include "stb_image_write.h"
#endif

#include "PNGWriter.h"
#include "SVGWriter.h"

namespace haiku {

PNGWriter::PNGWriter()
{
#ifdef __HAIKU__
	if (be_app == NULL)
		be_app = new BApplication("application/x-vnd.hvif-tools");
#endif
}

PNGWriter::~PNGWriter()
{
}

bool
PNGWriter::WriteToFile(const Icon& icon, const std::string& filename, 
	const PNGWriterOptions& opts)
{
	std::vector<uint8_t> pixelData;
	int width, height;

	if (!_RasterizeIcon(icon, opts, pixelData, width, height))
		return false;

#ifdef __HAIKU__
	BBitmap* bitmap = _CreateBBitmapFromPixelData(pixelData, width, height);
	if (!bitmap)
		return false;

	bool result = _SaveBBitmapToPNG(bitmap, filename);
	delete bitmap;
	return result;
#else
	int result = stbi_write_png(filename.c_str(), width, height, 4, 
		&pixelData[0], width * 4);

	return result != 0;
#endif
}

bool
PNGWriter::WriteToBuffer(const Icon& icon, std::vector<uint8_t>& buffer,
	const PNGWriterOptions& opts)
{
	std::vector<uint8_t> pixelData;
	int width, height;

	if (!_RasterizeIcon(icon, opts, pixelData, width, height))
		return false;

#ifdef __HAIKU__
	BBitmap* bitmap = _CreateBBitmapFromPixelData(pixelData, width, height);
	if (!bitmap)
		return false;

	bool result = _SaveBBitmapToPNGBuffer(bitmap, buffer);
	delete bitmap;
	return result;
#else
	struct WriteContext {
		std::vector<uint8_t>* buffer;
	};

	WriteContext ctx;
	ctx.buffer = &buffer;

	stbi_write_func* writeFunc = [](void* context, void* data, int size) {
		WriteContext* ctx = static_cast<WriteContext*>(context);
		uint8_t* bytes = static_cast<uint8_t*>(data);
		ctx->buffer->insert(ctx->buffer->end(), bytes, bytes + size);
	};

	buffer.clear();
	int result = stbi_write_png_to_func(writeFunc, &ctx, width, height, 4,
		&pixelData[0], width * 4);

	return result != 0;
#endif
}

bool
PNGWriter::_RasterizeIcon(const Icon& icon, const PNGWriterOptions& opts,
	std::vector<uint8_t>& pixelData, int& outWidth, int& outHeight)
{
	int width = opts.width;
	int height = opts.height;

	if (opts.scale != 1.0f) {
		width = static_cast<int>(width * opts.scale);
		height = static_cast<int>(height * opts.scale);
	}

	if (width <= 0 || height <= 0)
		return false;

	std::string svgString = _GenerateSVGString(icon, width, height);
	if (svgString.empty())
		return false;

	std::vector<char> svgBuffer;
	svgBuffer.reserve(svgString.size() + 1);
	for (size_t i = 0; i < svgString.size(); ++i)
		svgBuffer.push_back(svgString[i]);
	svgBuffer.push_back('\0');

	NSVGimage* image = nsvgParse(&svgBuffer[0], "px", 96.0f);
	if (!image)
		return false;

	NSVGrasterizer* rast = nsvgCreateRasterizer();
	if (!rast) {
		nsvgDelete(image);
		return false;
	}

	pixelData.resize(width * height * 4);

	float scale = 1.0f;
	if (image->width > 0 && image->height > 0) {
		float scaleX = static_cast<float>(width) / image->width;
		float scaleY = static_cast<float>(height) / image->height;
		scale = (scaleX < scaleY) ? scaleX : scaleY;
	}

	nsvgRasterize(rast, image, 0, 0, scale, &pixelData[0], width, height, width * 4);

	nsvgDeleteRasterizer(rast);
	nsvgDelete(image);

	outWidth = width;
	outHeight = height;

	return true;
}

std::string
PNGWriter::_GenerateSVGString(const Icon& icon, int width, int height)
{
	SVGWriter writer;
	SVGWriterOptions opts;

	opts.width = width;
	opts.height = height;
	opts.viewBox = "0 0 64 64";
	opts.includeNames = false;
	opts.coordinateScale = 1.0f;

	return writer.Write(icon, opts);
}

#ifdef __HAIKU__
BBitmap*
PNGWriter::_CreateBBitmapFromPixelData(const std::vector<uint8_t>& pixelData, 
	int width, int height)
{
	BBitmap* bitmap = new BBitmap(BRect(0, 0, width - 1, height - 1), B_RGBA32);
	if (!bitmap->IsValid()) {
		delete bitmap;
		return NULL;
	}

	uint8* bits = (uint8*)bitmap->Bits();
	int32 bpr = bitmap->BytesPerRow();

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			int srcOffset = (y * width + x) * 4;
			int dstOffset = y * bpr + x * 4;

			bits[dstOffset + 0] = pixelData[srcOffset + 2];
			bits[dstOffset + 1] = pixelData[srcOffset + 1];
			bits[dstOffset + 2] = pixelData[srcOffset + 0];
			bits[dstOffset + 3] = pixelData[srcOffset + 3];
		}
	}

	return bitmap;
}

bool
PNGWriter::_SaveBBitmapToPNG(BBitmap* bitmap, const std::string& filename)
{
	BFile file(filename.c_str(), B_WRITE_ONLY | B_CREATE_FILE | B_ERASE_FILE);
	if (file.InitCheck() != B_OK)
		return false;

	BBitmapStream stream(bitmap);
	
	BTranslatorRoster* roster = BTranslatorRoster::Default();
	if (!roster)
		return false;

	status_t result = roster->Translate(&stream, NULL, NULL, &file, B_PNG_FORMAT);

	BBitmap* temp;
	stream.DetachBitmap(&temp);

	return result == B_OK;
}

bool
PNGWriter::_SaveBBitmapToPNGBuffer(BBitmap* bitmap, std::vector<uint8_t>& buffer)
{
	BMallocIO mallocIO;
	
	BBitmapStream stream(bitmap);
	
	BTranslatorRoster* roster = BTranslatorRoster::Default();
	if (!roster)
		return false;

	status_t result = roster->Translate(&stream, NULL, NULL, &mallocIO, B_PNG_FORMAT);

	BBitmap* temp;
	stream.DetachBitmap(&temp);

	if (result != B_OK)
		return false;

	size_t size = mallocIO.BufferLength();
	const void* data = mallocIO.Buffer();
	
	buffer.clear();
	buffer.insert(buffer.end(), (const uint8_t*)data, (const uint8_t*)data + size);

	return true;
}
#endif

}
