/*
 * Copyright 2025, Gerasim Troeglazov, 3dEyes@gmail.com. All rights reserved.
 * Distributed under the terms of the MIT License.
 */

#include <sstream>
#include <iomanip>
#include <cmath>

#include "SVGWriter.h"
#include "Utils.h"

namespace haiku {

static const double HVIF_SCALE = 102.0;

SVGWriter::SVGWriter()
	: fIdCounter(0), fIncludeNames(false), fCoordinateScale(HVIF_SCALE)
{
}

std::string
SVGWriter::Write(const Icon& icon)
{
	SVGWriterOptions opts;
	return Write(icon, opts);
}

std::string
SVGWriter::Write(const Icon& icon, const SVGWriterOptions& opts)
{
	fIncludeNames = opts.includeNames;
	fCoordinateScale = opts.coordinateScale;
	fIdCounter = 0;

	std::ostringstream svg;

	svg << "<svg width=\"" << opts.width << "\" height=\"" << opts.height 
		<< "\" viewBox=\"" << opts.viewBox << "\" xmlns=\"http://www.w3.org/2000/svg\">\n";

	for (size_t i = 0; i < icon.shapes.size(); ++i) {
		if (icon.shapes[i].maxLOD < 3.99f)
			continue;
		svg << _ShapeToSVG(icon.shapes[i], icon, static_cast<int>(i));
	}

	svg << "</svg>";
	return svg.str();
}

std::string
SVGWriter::_FormatCoord(double value)
{
	double rounded = std::round(value * 100.0) / 100.0;
	long intPart = static_cast<long>(rounded);

	if (std::fabs(rounded - intPart) < 0.001) {
		return utils::ToString(intPart);
	}

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(2) << rounded;
	std::string result = oss.str();

	size_t dotPos = result.find('.');
	if (dotPos != std::string::npos) {
		size_t lastNonZero = result.find_last_not_of('0');
		if (lastNonZero == dotPos) {
			result = result.substr(0, dotPos);
		} else if (lastNonZero != std::string::npos) {
			result = result.substr(0, lastNonZero + 1);
		}
	}

	return result;
}

std::string
SVGWriter::_FormatMatrix(double value)
{
	double rounded = std::round(value * 1000000.0) / 1000000.0;

	if (std::fabs(rounded) < 1e-9) {
		return "0";
	}

	std::ostringstream oss;
	oss << std::fixed << std::setprecision(6) << rounded;
	std::string result = oss.str();

	size_t dotPos = result.find('.');
	if (dotPos != std::string::npos) {
		size_t lastNonZero = result.find_last_not_of('0');
		if (lastNonZero == dotPos) {
			result = result.substr(0, dotPos);
		} else if (lastNonZero != std::string::npos) {
			result = result.substr(0, lastNonZero + 1);
		}
	}

	return result;
}

std::string
SVGWriter::_ColorToCSS(const Color& color)
{
	std::ostringstream css;
	css << "#" << std::hex << std::setw(2) << std::setfill('0')
		<< static_cast<int>(color.Red())
		<< std::setw(2) << std::setfill('0')
		<< static_cast<int>(color.Green())
		<< std::setw(2) << std::setfill('0')
		<< static_cast<int>(color.Blue());
	return css.str();
}

float
SVGWriter::_GetColorAlpha(const Color& color)
{
	return static_cast<float>(color.Alpha()) / 255.0f;
}

bool
SVGWriter::_HasGeometricTransform(const Shape& shape)
{
	if (shape.hasTransform && shape.transform.size() >= 6)
		return true;

	for (size_t i = 0; i < shape.transformers.size(); ++i) {
		if (shape.transformers[i].type == TRANSFORMER_AFFINE ||
			shape.transformers[i].type == TRANSFORMER_PERSPECTIVE) {
			return true;
		}
	}

	return false;
}

void
SVGWriter::_TransformPoint(double& x, double& y, const Shape& shape)
{
	double tx = x / HVIF_SCALE;
	double ty = y / HVIF_SCALE;

	for (size_t i = 0; i < shape.transformers.size(); ++i) {
		const Transformer& t = shape.transformers[i];

		if (t.type == TRANSFORMER_AFFINE && t.matrix.size() >= 6) {
			double newX = tx * t.matrix[0] + ty * t.matrix[2] + t.matrix[4];
			double newY = tx * t.matrix[1] + ty * t.matrix[3] + t.matrix[5];
			tx = newX;
			ty = newY;
		} else if (t.type == TRANSFORMER_PERSPECTIVE && t.matrix.size() >= 9) {
			double w = tx * t.matrix[2] + ty * t.matrix[5] + t.matrix[8];
			if (std::fabs(w) < 1e-9) w = 1.0;
			double newX = (tx * t.matrix[0] + ty * t.matrix[3] + t.matrix[6]) / w;
			double newY = (tx * t.matrix[1] + ty * t.matrix[4] + t.matrix[7]) / w;
			tx = newX;
			ty = newY;
		}
	}

	if (shape.hasTransform && shape.transform.size() >= 6) {
		double newX = tx * shape.transform[0] + ty * shape.transform[2] + shape.transform[4];
		double newY = tx * shape.transform[1] + ty * shape.transform[3] + shape.transform[5];
		tx = newX;
		ty = newY;
	}

	x = tx * HVIF_SCALE;
	y = ty * HVIF_SCALE;
}

double
SVGWriter::_GetTransformScale(const Shape& shape)
{
	double s = 1.0;

	for (size_t i = 0; i < shape.transformers.size(); ++i) {
		const Transformer& t = shape.transformers[i];
		if (t.type == TRANSFORMER_AFFINE && t.matrix.size() >= 2) {
			s *= std::sqrt(t.matrix[0] * t.matrix[0] + t.matrix[1] * t.matrix[1]);
		}
	}

	if (shape.hasTransform && shape.transform.size() >= 2) {
		s *= std::sqrt(shape.transform[0] * shape.transform[0] +
					   shape.transform[1] * shape.transform[1]);
	}

	return s;
}

std::vector<double>
SVGWriter::_CombineGradientMatrix(const Gradient& grad, const Shape& shape)
{
	double m[6];
	if (grad.hasTransform && grad.transform.size() >= 6) {
		for (int i = 0; i < 6; ++i) m[i] = grad.transform[i];
	} else {
		m[0] = 1; m[1] = 0; m[2] = 0; m[3] = 1; m[4] = 0; m[5] = 0;
	}

	auto multMatrix = [](double* result, const double* a, const double* b) {
		result[0] = a[0] * b[0] + a[1] * b[2];
		result[1] = a[0] * b[1] + a[1] * b[3];
		result[2] = a[2] * b[0] + a[3] * b[2];
		result[3] = a[2] * b[1] + a[3] * b[3];
		result[4] = a[4] * b[0] + a[5] * b[2] + b[4];
		result[5] = a[4] * b[1] + a[5] * b[3] + b[5];
	};

	for (size_t i = 0; i < shape.transformers.size(); ++i) {
		const Transformer& t = shape.transformers[i];

		if (t.type == TRANSFORMER_AFFINE && t.matrix.size() >= 6) {
			double tm[6];
			for (int j = 0; j < 6; ++j) tm[j] = t.matrix[j];
			double result[6];
			multMatrix(result, m, tm);
			for (int j = 0; j < 6; ++j) m[j] = result[j];
		} else if (t.type == TRANSFORMER_PERSPECTIVE && t.matrix.size() >= 9) {
			const std::vector<double>& p = t.matrix;
			double w = m[4] * p[2] + m[5] * p[5] + p[8];
			if (std::fabs(w) < 1e-9) w = 1.0;
			double tm[6] = { p[0]/w, p[1]/w, p[3]/w, p[4]/w, p[6]/w, p[7]/w };
			double result[6];
			multMatrix(result, m, tm);
			for (int j = 0; j < 6; ++j) m[j] = result[j];
		}
	}

	if (shape.hasTransform && shape.transform.size() >= 6) {
		double tm[6];
		for (int j = 0; j < 6; ++j) tm[j] = shape.transform[j];
		double result[6];
		multMatrix(result, m, tm);
		for (int j = 0; j < 6; ++j) m[j] = result[j];
	}

	std::vector<double> resultVec(6);
	for (int i = 0; i < 6; ++i) resultVec[i] = m[i];
	return resultVec;
}

std::string
SVGWriter::_GradientToSVG(const Gradient& grad, const std::string& id, 
	const std::string& styleName, const Shape& shape)
{
	std::ostringstream svg;

	bool isLinear = (grad.type == GRADIENT_LINEAR || grad.type == GRADIENT_CONIC || 
					 grad.type == GRADIENT_XY || grad.type == GRADIENT_SQRT_XY ||
					 grad.type == GRADIENT_DIAMOND);
	bool isInverted = (grad.type == GRADIENT_CONIC || grad.type == GRADIENT_XY || 
					   grad.type == GRADIENT_SQRT_XY || grad.type == GRADIENT_DIAMOND);
	bool isConic = (grad.type == GRADIENT_CONIC);

	std::string tagName = isLinear ? "linearGradient" : "radialGradient";

	svg << "<" << tagName << " id=\"" << id << "\"";

	if (fIncludeNames && !styleName.empty()) {
		svg << " data-name=\"" << styleName << "\"";
	}

	svg << " gradientUnits=\"userSpaceOnUse\"";

	std::vector<double> m = _CombineGradientMatrix(grad, shape);
	
	svg << " gradientTransform=\"matrix("
		<< _FormatMatrix(m[0]) << ","
		<< _FormatMatrix(m[1]) << ","
		<< _FormatMatrix(m[2]) << ","
		<< _FormatMatrix(m[3]) << ","
		<< _FormatCoord(m[4] * HVIF_SCALE) << ","
		<< _FormatCoord(m[5] * HVIF_SCALE) << ")\"";

	long baseCoord = 6528;
	long conicCoord = baseCoord * 1.52;

	if (isLinear) {
		if (isConic) {
			svg << " x1=\"" << conicCoord << "\" x2=\"" << -conicCoord 
				<< "\" y1=\"" << -baseCoord << "\" y2=\"" << -baseCoord << "\"";
		} else if (isInverted) {
			svg << " x1=\"" << baseCoord << "\" x2=\"" << -baseCoord 
				<< "\" y1=\"" << -baseCoord << "\" y2=\"" << -baseCoord << "\"";
		} else {
			svg << " x1=\"" << -baseCoord << "\" x2=\"" << baseCoord 
				<< "\" y1=\"" << -baseCoord << "\" y2=\"" << -baseCoord << "\"";
		}
	} else {
		svg << " cx=\"0\" cy=\"0\" r=\"" << baseCoord << "\"";
	}

	svg << ">\n";

	for (size_t i = 0; i < grad.stops.size(); ++i) {
		const ColorStop& stop = grad.stops[i];
		std::string stopColor = _ColorToCSS(stop.color);
		float alpha = _GetColorAlpha(stop.color);

		svg << "<stop offset=\"" << _FormatCoord(stop.offset * 100.0)
			<< "%\" stop-color=\"" << stopColor << "\"";

		if (alpha < 1.0f) {
			svg << " stop-opacity=\"" << _FormatCoord(alpha) << "\"";
		}

		svg << " />\n";
	}

	svg << "</" << tagName << ">\n";

	return svg.str();
}

std::string
SVGWriter::_PathToSVG(const Path& path)
{
	std::ostringstream pathData;

	if (path.points.empty())
		return "";

	const PathPoint& first = path.points[0];
	pathData << "M " << _FormatCoord(first.x * fCoordinateScale)
	         << " " << _FormatCoord(first.y * fCoordinateScale);

	for (size_t i = 1; i < path.points.size(); ++i) {
		const PathPoint& prev = path.points[i - 1];
		const PathPoint& curr = path.points[i];

		pathData << " C "
			<< _FormatCoord(prev.x_out * fCoordinateScale) << " "
			<< _FormatCoord(prev.y_out * fCoordinateScale) << " "
			<< _FormatCoord(curr.x_in * fCoordinateScale) << " "
			<< _FormatCoord(curr.y_in * fCoordinateScale) << " "
			<< _FormatCoord(curr.x * fCoordinateScale) << " "
			<< _FormatCoord(curr.y * fCoordinateScale);
	}

	if (path.closed && path.points.size() > 1) {
		const PathPoint& last = path.points[path.points.size() - 1];
		pathData << " C "
			<< _FormatCoord(last.x_out * fCoordinateScale) << " "
			<< _FormatCoord(last.y_out * fCoordinateScale) << " "
			<< _FormatCoord(first.x_in * fCoordinateScale) << " "
			<< _FormatCoord(first.y_in * fCoordinateScale) << " "
			<< _FormatCoord(first.x * fCoordinateScale) << " "
			<< _FormatCoord(first.y * fCoordinateScale) << " Z";
	}

	return pathData.str();
}

std::string
SVGWriter::_PathToSVGTransformed(const Path& path, const Shape& shape)
{
	std::ostringstream pathData;

	if (path.points.empty())
		return "";

	double sx = path.points[0].x * fCoordinateScale;
	double sy = path.points[0].y * fCoordinateScale;
	_TransformPoint(sx, sy, shape);

	double sox = path.points[0].x_out * fCoordinateScale;
	double soy = path.points[0].y_out * fCoordinateScale;
	_TransformPoint(sox, soy, shape);

	double six = path.points[0].x_in * fCoordinateScale;
	double siy = path.points[0].y_in * fCoordinateScale;
	_TransformPoint(six, siy, shape);

	pathData << "M " << _FormatCoord(sx) << " " << _FormatCoord(sy);

	double pox = sox, poy = soy;

	for (size_t i = 1; i < path.points.size(); ++i) {
		const PathPoint& curr = path.points[i];

		double cx = curr.x * fCoordinateScale;
		double cy = curr.y * fCoordinateScale;
		double cix = curr.x_in * fCoordinateScale;
		double ciy = curr.y_in * fCoordinateScale;
		double cox = curr.x_out * fCoordinateScale;
		double coy = curr.y_out * fCoordinateScale;

		_TransformPoint(cx, cy, shape);
		_TransformPoint(cix, ciy, shape);
		_TransformPoint(cox, coy, shape);

		pathData << " C "
			<< _FormatCoord(pox) << " " << _FormatCoord(poy) << " "
			<< _FormatCoord(cix) << " " << _FormatCoord(ciy) << " "
			<< _FormatCoord(cx) << " " << _FormatCoord(cy);

		pox = cox;
		poy = coy;
	}

	if (path.closed && path.points.size() > 1) {
		pathData << " C "
			<< _FormatCoord(pox) << " " << _FormatCoord(poy) << " "
			<< _FormatCoord(six) << " " << _FormatCoord(siy) << " "
			<< _FormatCoord(sx) << " " << _FormatCoord(sy) << " Z";
	}

	return pathData.str();
}

std::string
SVGWriter::_ShapeToSVG(const Shape& shape, const Icon& icon, int shapeIndex)
{
	std::ostringstream svg;

	bool hasGeomTransform = _HasGeometricTransform(shape);

	float opacity = 1.0f;
	std::string fillColor;
	std::string additionalDefs;

	if (shape.styleIndex >= 0 && shape.styleIndex < static_cast<int>(icon.styles.size())) {
		const Style& style = icon.styles[shape.styleIndex];

		if (style.isGradient) {
			std::string gradientId = _GenerateID();
			fillColor = "url(#" + gradientId + ")";
			additionalDefs = _GradientToSVG(style.gradient, gradientId, style.name, shape);
		} else {
			fillColor = _ColorToCSS(style.solidColor);
			opacity = _GetColorAlpha(style.solidColor);
		}
	}

	bool isStroke = false;
	bool isContour = false;
	Transformer effectTrans;

	for (size_t i = 0; i < shape.transformers.size(); ++i) {
		const Transformer& trans = shape.transformers[i];
		if (trans.type == TRANSFORMER_STROKE) {
			isStroke = true;
			effectTrans = trans;
			break;
		} else if (trans.type == TRANSFORMER_CONTOUR) {
			isContour = true;
			effectTrans = trans;
			break;
		}
	}

	double strokeWidth = std::fabs(effectTrans.width) * fCoordinateScale;
	if (hasGeomTransform) {
		strokeWidth *= _GetTransformScale(shape);
	}

	if (!additionalDefs.empty()) {
		svg << "<g>\n";
		svg << "<defs>\n" << additionalDefs << "</defs>\n";
	}

	if (!shape.pathIndices.empty()) {
		std::ostringstream pathD;
		for (size_t i = 0; i < shape.pathIndices.size(); ++i) {
			int pathIdx = shape.pathIndices[i];
			if (pathIdx >= 0 && pathIdx < static_cast<int>(icon.paths.size())) {
				if (hasGeomTransform) {
					pathD << _PathToSVGTransformed(icon.paths[pathIdx], shape) << " ";
				} else {
					pathD << _PathToSVG(icon.paths[pathIdx]) << " ";
				}
			}
		}
		std::string d = pathD.str();

		std::ostringstream styleStr;
		styleStr << "stroke-width:" << _FormatCoord(strokeWidth) << ";";
		styleStr << "stroke-linejoin:" << utils::GetLineJoinName(effectTrans.lineJoin) << ";";
		styleStr << "stroke-linecap:" << utils::GetLineCapName(effectTrans.lineCap) << ";";

		if (isContour) {
			if (effectTrans.width < 0) {
				styleStr << "stroke:black;fill:white;";
			} else {
				styleStr << "stroke:white;fill:white;";
			}

			std::string maskId = _GenerateID();
			svg << "<mask id=\"" << maskId << "\" maskUnits=\"userSpaceOnUse\" x=\"0\" y=\"0\" width=\"6528\" height=\"6528\">\n";
			svg << "<path d=\"" << d << "\" style=\"" << styleStr.str() << "\" />\n";
			svg << "</mask>\n";
			svg << "<rect x=\"0\" y=\"0\" width=\"6528\" height=\"6528\" fill=\"" << fillColor << "\" mask=\"url(#" << maskId << ")\"";
			if (opacity < 1.0f) {
				svg << " fill-opacity=\"" << _FormatCoord(opacity) << "\"";
			}
			svg << " />\n";
		} else if (isStroke) {
			styleStr << "stroke:" << fillColor << ";fill:none;";
			if (opacity < 1.0f) {
				styleStr << "stroke-opacity:" << _FormatCoord(opacity) << ";";
			}
			
			svg << "<path";
			svg << " id=\"shape_" << shapeIndex << "\"";
			svg << " d=\"" << d << "\"";
			svg << " style=\"" << styleStr.str() << "\"";
			svg << " />\n";
		} else {
			svg << "<path";
			svg << " id=\"shape_" << shapeIndex << "\"";
			svg << " d=\"" << d << "\"";
			svg << " style=\"fill:" << fillColor << ";stroke:none;";
			if (opacity < 1.0f) {
				svg << "fill-opacity:" << _FormatCoord(opacity) << ";";
			}
			svg << "\"";
			svg << " />\n";
		}
	}

	if (!additionalDefs.empty())
		svg << "</g>\n";
	
	return svg.str();
}

std::string
SVGWriter::_GenerateID()
{
	return "id" + utils::ToString(++fIdCounter);
}

}
