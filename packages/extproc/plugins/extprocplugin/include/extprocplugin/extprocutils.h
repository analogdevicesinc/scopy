
/*
 * Copyright (c) 2025 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef EXTPROCUTILS_H
#define EXTPROCUTILS_H

#include <QDir>
#include <QString>
#include <QStringList>
#include "common/scopyconfig.h"

namespace scopy::extprocplugin {

typedef struct
{
	QString name;
	bool enable;
} ChannelInfo;

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

struct DataManagerKeys
{
	static constexpr auto INPUT = "input";
	static constexpr auto TIME = "time";
};
class ExtProcUtils
{
public:
	static QString dataOutPath() { return scopy::config::executableFolderPath() + QDir::separator() + "data.out"; }
	static QString dataInPath() { return scopy::config::executableFolderPath() + QDir::separator() + "data.in"; }
};

} // namespace scopy::extprocplugin

#endif // EXTPROCUTILS_H
