#pragma once

namespace scopy::iio {

struct DataFormat {
	unsigned int length;
	unsigned int bits;
	unsigned int shift;
	bool is_signed;
	bool is_fully_defined;
	bool is_be;
	bool with_scale;
	double scale;
	unsigned int repeat;
	double offset; // v1 only; v0 backend sets 0.0
};

} // namespace scopy::iio
