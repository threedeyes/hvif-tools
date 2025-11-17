/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include "IconAdapter.h"
#include "Utils.h"

namespace adapter {

haiku::Icon
HVIFAdapter::FromHVIF(const hvif::HVIFIcon& hvif)
{
	haiku::Icon icon;
	icon.filename = hvif.filename;

	for (size_t i = 0; i < hvif.styles.size(); ++i) {
		haiku::Style style;
		style.isGradient = hvif.styles[i].isGradient;
		if (style.isGradient) {
			style.gradient = ConvertGradient(hvif.styles[i].gradient);
		} else {
			style.solidColor = ConvertColor(hvif.styles[i].color);
		}
		icon.styles.push_back(style);
	}

	for (size_t i = 0; i < hvif.paths.size(); ++i) {
		haiku::Path path;
		path.closed = hvif.paths[i].closed;
		
		if (hvif.paths[i].type == "points") {
			for (size_t j = 0; j + 1 < hvif.paths[i].points.size(); j += 2) {
				float x = hvif.paths[i].points[j] / 102.0f;
				float y = hvif.paths[i].points[j + 1] / 102.0f;
				haiku::PathPoint pt = ConvertNode(x, y, x, y, x, y);
				path.points.push_back(pt);
			}
		} else {
			for (size_t j = 0; j + 5 < hvif.paths[i].points.size(); j += 6) {
				float x = hvif.paths[i].points[j] / 102.0f;
				float y = hvif.paths[i].points[j + 1] / 102.0f;
				float x_in = hvif.paths[i].points[j + 2] / 102.0f;
				float y_in = hvif.paths[i].points[j + 3] / 102.0f;
				float x_out = hvif.paths[i].points[j + 4] / 102.0f;
				float y_out = hvif.paths[i].points[j + 5] / 102.0f;
				haiku::PathPoint pt = ConvertNode(x, y, x_in, y_in, x_out, y_out);
				path.points.push_back(pt);
			}
		}
		icon.paths.push_back(path);
	}

	for (size_t i = 0; i < hvif.shapes.size(); ++i) {
		haiku::Shape shape;
		shape.styleIndex = hvif.shapes[i].styleIndex;

		for (size_t j = 0; j < hvif.shapes[i].pathIndices.size(); ++j) {
			shape.pathIndices.push_back(static_cast<int>(hvif.shapes[i].pathIndices[j]));
		}

		shape.hasTransform = hvif.shapes[i].hasTransform;
		if (shape.hasTransform) {
			if (hvif.shapes[i].transformType == "translate" && hvif.shapes[i].transform.size() >= 2) {
				shape.transform.push_back(1.0);
				shape.transform.push_back(0.0);
				shape.transform.push_back(0.0);
				shape.transform.push_back(1.0);
				shape.transform.push_back(static_cast<double>(hvif.shapes[i].transform[0] / 102.0f));
				shape.transform.push_back(static_cast<double>(hvif.shapes[i].transform[1] / 102.0f));
			} else if (hvif.shapes[i].transformType == "matrix" && hvif.shapes[i].transform.size() >= 6) {
				for (size_t j = 0; j < hvif.shapes[i].transform.size(); ++j) {
					shape.transform.push_back(static_cast<double>(hvif.shapes[i].transform[j]));
				}
			} else {
				shape.hasTransform = false;
			}
		}

		for (size_t j = 0; j < hvif.shapes[i].transformers.size(); ++j) {
			shape.transformers.push_back(ConvertTransformer(hvif.shapes[i].transformers[j]));
		}

		shape.minLOD = static_cast<float>(hvif.shapes[i].minLOD) * 4.0f / 255.0f;
		shape.maxLOD = static_cast<float>(hvif.shapes[i].maxLOD) * 4.0f / 255.0f;

		icon.shapes.push_back(shape);
	}

	return icon;
}

hvif::HVIFIcon
HVIFAdapter::ToHVIF(const haiku::Icon& icon)
{
	hvif::HVIFIcon hvif;
	hvif.filename = icon.filename;

	for (size_t i = 0; i < icon.styles.size(); ++i) {
		hvif::Style style;
		style.isGradient = icon.styles[i].isGradient;
		if (style.isGradient) {
			style.gradient = ConvertGradientToHVIF(icon.styles[i].gradient);
		} else {
			style.color = ConvertColorToHVIF(icon.styles[i].solidColor);
		}
		hvif.styles.push_back(style);
	}

	for (size_t i = 0; i < icon.paths.size(); ++i) {
		hvif::Path path;
		path.closed = icon.paths[i].closed;
		path.type = "curves";

		for (size_t j = 0; j < icon.paths[i].points.size(); ++j) {
			const haiku::PathPoint& pt = icon.paths[i].points[j];
			path.points.push_back(static_cast<float>(pt.x));
			path.points.push_back(static_cast<float>(pt.y));
			path.points.push_back(static_cast<float>(pt.x_in));
			path.points.push_back(static_cast<float>(pt.y_in));
			path.points.push_back(static_cast<float>(pt.x_out));
			path.points.push_back(static_cast<float>(pt.y_out));
		}
		hvif.paths.push_back(path);
	}

	for (size_t i = 0; i < icon.shapes.size(); ++i) {
		hvif::Shape shape;
		shape.styleIndex = static_cast<uint8_t>(icon.shapes[i].styleIndex);

		for (size_t j = 0; j < icon.shapes[i].pathIndices.size(); ++j) {
			shape.pathIndices.push_back(static_cast<uint8_t>(icon.shapes[i].pathIndices[j]));
		}

		shape.hasTransform = icon.shapes[i].hasTransform;
		if (shape.hasTransform && icon.shapes[i].transform.size() >= 6) {
			shape.transformType = "matrix";
			for (size_t j = 0; j < icon.shapes[i].transform.size(); ++j) {
				shape.transform.push_back(static_cast<float>(icon.shapes[i].transform[j]));
			}
		}

		for (size_t j = 0; j < icon.shapes[i].transformers.size(); ++j) {
			shape.transformers.push_back(ConvertTransformerToHVIF(icon.shapes[i].transformers[j]));
		}

		shape.minLOD = static_cast<uint8_t>(utils::clamp(
			static_cast<int>(icon.shapes[i].minLOD * 255.0f / 4.0f), 0, 255));
		shape.maxLOD = static_cast<uint8_t>(utils::clamp(
			static_cast<int>(icon.shapes[i].maxLOD * 255.0f / 4.0f), 0, 255));
		shape.hasLOD = (shape.minLOD != 0 || shape.maxLOD != 255);

		hvif.shapes.push_back(shape);
	}

	return hvif;
}

haiku::Color
HVIFAdapter::ConvertColor(const hvif::Color& c)
{
	uint8_t r = 0, g = 0, b = 0, a = 255;

	switch (c.tag) {
		case hvif::RGBA:
			if (c.data.size() >= 4) {
				r = c.data[0]; g = c.data[1]; b = c.data[2]; a = c.data[3];
			}
			break;
		case hvif::RGB:
			if (c.data.size() >= 3) {
				r = c.data[0]; g = c.data[1]; b = c.data[2];
			}
			break;
		case hvif::KA:
			if (c.data.size() >= 2) {
				r = g = b = c.data[0]; a = c.data[1];
			}
			break;
		case hvif::K:
			if (c.data.size() >= 1) {
				r = g = b = c.data[0];
			}
			break;
		default:
			break;
	}

	uint32_t argb = (static_cast<uint32_t>(a) << 24) |
	                (static_cast<uint32_t>(r) << 16) |
	                (static_cast<uint32_t>(g) << 8) |
	                static_cast<uint32_t>(b);
	return haiku::Color(argb);
}

hvif::Color
HVIFAdapter::ConvertColorToHVIF(const haiku::Color& c)
{
	hvif::Color color;
	color.tag = hvif::RGBA;
	color.data.push_back(c.Red());
	color.data.push_back(c.Green());
	color.data.push_back(c.Blue());
	color.data.push_back(c.Alpha());
	return color;
}

haiku::Gradient
HVIFAdapter::ConvertGradient(const hvif::Gradient& g)
{
	haiku::Gradient grad;
	grad.type = static_cast<haiku::GradientType>(g.type);
	grad.interpolation = haiku::INTERPOLATION_LINEAR;
	grad.hasTransform = g.hasMatrix;

	if (g.hasMatrix) {
		for (size_t i = 0; i < g.matrix.size(); ++i) {
			grad.transform.push_back(static_cast<double>(g.matrix[i]));
		}
	}

	for (size_t i = 0; i < g.stops.size(); ++i) {
		haiku::ColorStop stop;
		stop.color = ConvertColor(g.stops[i].color);
		stop.offset = static_cast<float>(g.stops[i].offset) / 255.0f;
		grad.stops.push_back(stop);
	}

	return grad;
}

hvif::Gradient
HVIFAdapter::ConvertGradientToHVIF(const haiku::Gradient& g)
{
	hvif::Gradient grad;
	grad.type = static_cast<hvif::GradientTypes>(g.type);
	grad.flags = 0;
	grad.hasMatrix = g.hasTransform;

	if (g.hasTransform) {
		for (size_t i = 0; i < g.transform.size(); ++i) {
			grad.matrix.push_back(static_cast<float>(g.transform[i]));
		}
	}

	for (size_t i = 0; i < g.stops.size(); ++i) {
		hvif::GradientStop stop;
		stop.color = ConvertColorToHVIF(g.stops[i].color);
		stop.offset = static_cast<uint8_t>(g.stops[i].offset * 255.0f);
		grad.stops.push_back(stop);
	}

	return grad;
}

haiku::PathPoint
HVIFAdapter::ConvertNode(float x, float y, float x_in, float y_in, float x_out, float y_out)
{
	haiku::PathPoint pt;
	pt.x = x;
	pt.y = y;
	pt.x_in = x_in;
	pt.y_in = y_in;
	pt.x_out = x_out;
	pt.y_out = y_out;
	pt.connected = true;
	return pt;
}

haiku::Transformer
HVIFAdapter::ConvertTransformer(const hvif::Transformer& t)
{
	haiku::Transformer trans;

	if (t.tag == hvif::STROKE) {
		trans.type = haiku::TRANSFORMER_STROKE;
		trans.width = t.width / 102.0;
		trans.lineJoin = t.lineJoin;
		trans.lineCap = t.lineCap;
		trans.miterLimit = t.miterLimit;
	} else if (t.tag == hvif::CONTOUR) {
		trans.type = haiku::TRANSFORMER_CONTOUR;
		trans.width = t.width / 102.0;
	} else if (t.tag == hvif::AFFINE) {
		trans.type = haiku::TRANSFORMER_AFFINE;
		for (size_t i = 0; i < t.data.size(); ++i) {
			trans.matrix.push_back(static_cast<double>(t.data[i]));
		}
	} else {
		trans.type = haiku::TRANSFORMER_PERSPECTIVE;
	}

	return trans;
}

hvif::Transformer
HVIFAdapter::ConvertTransformerToHVIF(const haiku::Transformer& t)
{
	hvif::Transformer trans;
	
	if (t.type == haiku::TRANSFORMER_STROKE) {
		trans.tag = hvif::STROKE;
		trans.width = static_cast<float>(t.width * 102.0);
		trans.lineJoin = static_cast<uint8_t>(t.lineJoin);
		trans.lineCap = static_cast<uint8_t>(t.lineCap);
		trans.miterLimit = static_cast<uint8_t>(t.miterLimit);
	} else if (t.type == haiku::TRANSFORMER_CONTOUR) {
		trans.tag = hvif::CONTOUR;
		trans.width = static_cast<float>(t.width * 102.0);
	} else if (t.type == haiku::TRANSFORMER_AFFINE) {
		trans.tag = hvif::AFFINE;
		for (size_t i = 0; i < t.matrix.size(); ++i) {
			trans.data.push_back(static_cast<float>(t.matrix[i]));
		}
	} else {
		trans.tag = hvif::PERSPECTIVE;
	}

	return trans;
}

haiku::Icon
IOMAdapter::FromIOM(const iom::Icon& iom)
{
	haiku::Icon icon;
	icon.filename = iom.filename;

	for (size_t i = 0; i < iom.styles.size(); ++i) {
		haiku::Style style;
		style.name = iom.styles[i].name;
		style.isGradient = iom.styles[i].isGradient;
		if (style.isGradient) {
			style.gradient = ConvertGradient(iom.styles[i].gradient);
		} else {
			style.solidColor = ConvertColor(iom.styles[i].color);
		}
		icon.styles.push_back(style);
	}

	for (size_t i = 0; i < iom.paths.size(); ++i) {
		haiku::Path path;
		path.name = iom.paths[i].name;
		path.closed = iom.paths[i].closed;

		for (size_t j = 0; j < iom.paths[i].points.size(); ++j) {
			path.points.push_back(ConvertPoint(iom.paths[i].points[j]));
		}
		icon.paths.push_back(path);
	}

	for (size_t i = 0; i < iom.shapes.size(); ++i) {
		haiku::Shape shape;
		shape.name = iom.shapes[i].name;
		shape.styleIndex = iom.shapes[i].styleIndex;
		shape.pathIndices = iom.shapes[i].pathIndices;
		shape.hasTransform = iom.shapes[i].hasTransform;
		shape.transform = iom.shapes[i].transform;
		shape.minLOD = iom.shapes[i].minVisibility;
		shape.maxLOD = iom.shapes[i].maxVisibility;

		for (size_t j = 0; j < iom.shapes[i].transformers.size(); ++j) {
			shape.transformers.push_back(ConvertTransformer(iom.shapes[i].transformers[j]));
		}
		icon.shapes.push_back(shape);
	}

	return icon;
}

iom::Icon
IOMAdapter::ToIOM(const haiku::Icon& icon)
{
	iom::Icon iom;
	iom.filename = icon.filename;

	for (size_t i = 0; i < icon.styles.size(); ++i) {
		iom::Style style;
		style.name = icon.styles[i].name;
		style.isGradient = icon.styles[i].isGradient;
		if (style.isGradient) {
			style.gradient = ConvertGradientToIOM(icon.styles[i].gradient);
		} else {
			style.color = ConvertColorToIOM(icon.styles[i].solidColor);
		}
		iom.styles.push_back(style);
	}

	for (size_t i = 0; i < icon.paths.size(); ++i) {
		iom::Path path;
		path.name = icon.paths[i].name;
		path.closed = icon.paths[i].closed;

		for (size_t j = 0; j < icon.paths[i].points.size(); ++j) {
			path.points.push_back(ConvertPointToIOM(icon.paths[i].points[j]));
		}
		iom.paths.push_back(path);
	}

	for (size_t i = 0; i < icon.shapes.size(); ++i) {
		iom::Shape shape;
		shape.name = icon.shapes[i].name;
		shape.what = 1;
		shape.styleIndex = icon.shapes[i].styleIndex;
		shape.pathIndices = icon.shapes[i].pathIndices;
		shape.hasTransform = icon.shapes[i].hasTransform;
		shape.transform = icon.shapes[i].transform;
		shape.hinting = false;
		shape.minVisibility = icon.shapes[i].minLOD;
		shape.maxVisibility = icon.shapes[i].maxLOD;

		for (size_t j = 0; j < icon.shapes[i].transformers.size(); ++j) {
			shape.transformers.push_back(ConvertTransformerToIOM(icon.shapes[i].transformers[j]));
		}
		iom.shapes.push_back(shape);
	}

	return iom;
}

haiku::Color
IOMAdapter::ConvertColor(uint32_t iomColor)
{
	uint8_t r = iomColor & 0xFF;
	uint8_t g = (iomColor >> 8) & 0xFF;
	uint8_t b = (iomColor >> 16) & 0xFF;
	uint8_t a = (iomColor >> 24) & 0xFF;

	uint32_t argb = (static_cast<uint32_t>(a) << 24) |
		(static_cast<uint32_t>(r) << 16) |
		(static_cast<uint32_t>(g) << 8) |
		static_cast<uint32_t>(b);

	return haiku::Color(argb);
}

uint32_t
IOMAdapter::ConvertColorToIOM(const haiku::Color& c)
{
	return (static_cast<uint32_t>(c.Alpha()) << 24) |
		(static_cast<uint32_t>(c.Blue()) << 16) |
		(static_cast<uint32_t>(c.Green()) << 8) |
		static_cast<uint32_t>(c.Red());
}

haiku::Gradient
IOMAdapter::ConvertGradient(const iom::Gradient& g)
{
	haiku::Gradient grad;
	grad.type = static_cast<haiku::GradientType>(g.type);
	grad.interpolation = static_cast<haiku::InterpolationType>(g.interpolation);
	grad.hasTransform = g.hasTransform;
	grad.transform = g.transform;

	for (size_t i = 0; i < g.stops.size(); ++i) {
		haiku::ColorStop stop(ConvertColor(g.stops[i].color), g.stops[i].offset);
		grad.stops.push_back(stop);
	}

	return grad;
}

iom::Gradient
IOMAdapter::ConvertGradientToIOM(const haiku::Gradient& g)
{
	iom::Gradient grad;
	grad.type = static_cast<iom::GradientType>(g.type);
	grad.interpolation = static_cast<iom::InterpolationType>(g.interpolation);
	grad.hasTransform = g.hasTransform;
	grad.transform = g.transform;
	grad.inheritTransformation = true;

	for (size_t i = 0; i < g.stops.size(); ++i) {
		iom::ColorStop stop;
		stop.color = ConvertColorToIOM(g.stops[i].color);
		stop.offset = g.stops[i].offset;
		grad.stops.push_back(stop);
	}

	return grad;
}

haiku::PathPoint
IOMAdapter::ConvertPoint(const iom::ControlPoint& cp)
{
	haiku::PathPoint pt;
	pt.x = cp.x;
	pt.y = cp.y;
	pt.x_in = cp.x_in;
	pt.y_in = cp.y_in;
	pt.x_out = cp.x_out;
	pt.y_out = cp.y_out;
	pt.connected = cp.connected;
	return pt;
}

iom::ControlPoint
IOMAdapter::ConvertPointToIOM(const haiku::PathPoint& p)
{
	iom::ControlPoint cp;
	cp.x = static_cast<float>(p.x);
	cp.y = static_cast<float>(p.y);
	cp.x_in = static_cast<float>(p.x_in);
	cp.y_in = static_cast<float>(p.y_in);
	cp.x_out = static_cast<float>(p.x_out);
	cp.y_out = static_cast<float>(p.y_out);
	cp.connected = p.connected;
	return cp;
}

haiku::Transformer
IOMAdapter::ConvertTransformer(const iom::Transformer& t)
{
	haiku::Transformer trans;
	trans.type = static_cast<haiku::TransformerType>(t.type);
	trans.matrix = t.matrix;
	trans.width = t.width;
	trans.lineJoin = t.lineJoin;
	trans.lineCap = t.lineCap;
	trans.miterLimit = t.miterLimit;
	return trans;
}

iom::Transformer
IOMAdapter::ConvertTransformerToIOM(const haiku::Transformer& t)
{
	iom::Transformer trans;
	trans.type = static_cast<iom::TransformerType>(t.type);
	trans.matrix = t.matrix;
	trans.width = t.width;
	trans.lineJoin = static_cast<int32_t>(t.lineJoin);
	trans.lineCap = static_cast<int32_t>(t.lineCap);
	trans.miterLimit = t.miterLimit;
	return trans;
}

}
