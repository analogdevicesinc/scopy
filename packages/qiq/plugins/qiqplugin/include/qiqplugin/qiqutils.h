#ifndef QIQUTILS_H
#define QIQUTILS_H

#include <QString>
#include <QStringList>

namespace scopy::qiqplugin {

typedef struct
{
	int samplesCount;
	QString deviceName;
	QStringList enChnls;
} BufferParams;

typedef struct
{
	QVector<double> x; // can be empty
	QVector<double> y;
} CurveData;

struct FileFormatTypes
{
	static constexpr auto BINARY_INTERLEAVED = "binary-interleaved";
};

struct ChannelFormatTypes
{
	static constexpr auto FLOAT32 = "float32";
	static constexpr auto FLOAT64 = "float64";
	static constexpr auto INT8 = "int8";
	static constexpr auto UINT8 = "uint8";
	static constexpr auto INT16 = "int16";
	static constexpr auto UINT16 = "uint16";
	static constexpr auto INT32 = "int32";
	static constexpr auto UINT32 = "uint32";
	static constexpr auto INT64 = "int64";
	static constexpr auto UINT64 = "uint64";
};

struct CommandNames
{
	static constexpr auto SET_INPUT_CONFIG = "set_input_config";
	static constexpr auto SET_ANALYSIS_CONFIG = "set_analysis_config";
	static constexpr auto SET_OUTPUT_CONFIG = "set_output_config";
	static constexpr auto RUN = "run";
	static constexpr auto GET_ANALYSIS_TYPES = "get_analysis_types";
	static constexpr auto GET_ANALYSIS_INFO = "get_analysis_info";
};

} // namespace scopy::qiqplugin

#endif // QIQUTILS_H
