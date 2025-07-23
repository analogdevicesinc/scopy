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

#ifndef MAPKEYS_H
#define MAPKEYS_H

namespace scopy::extprocplugin {

struct KeysInputConfig
{
	static constexpr auto SAMPLE_COUNT = "sample_count";
	static constexpr auto CHANNEL_COUNT = "channel_count";
	static constexpr auto SAMPLING_FREQUENCY = "sampling_frequency";
	static constexpr auto FREQUENCY_OFFSET = "frequency_offset";
	static constexpr auto INPUT_FILE = "input_file";
	static constexpr auto INPUT_FILE_FORMAT = "input_file_format";
	static constexpr auto CHANNEL_FORMAT = "channel_format";
};

struct KeysOutputConfig
{
	static constexpr auto OUTPUT_FILE = "output_file";
	static constexpr auto OUTPUT_FILE_FORMAT = "output_file_format";
	static constexpr auto ENABLED_ANALYSIS = "enabled_analysis";
};

struct KeysOutputInfo
{
	static constexpr auto CHANNEL_COUNT = "channel_count";
	static constexpr auto CHANNEL_NAMES = "channel_names";
	static constexpr auto CHANNEL_FORMAT = "channel_format";
};

struct KeysAnalysisConfig
{
	static constexpr auto ANALYSIS_TYPE = "analysis_type";
	static constexpr auto OUT_INFO = "output_info";
	static constexpr auto PARAMS = "config";
};

struct KeysRunResults
{
	static constexpr auto OUTPUT_FILE = "output_file";
	static constexpr auto RESULTS = "results";
	static constexpr auto MEASUREMENTS = "measurements";
};

struct KeysAnalysisInfo
{
	static constexpr auto ANALYSIS_TYPE = "analysis_type";
	static constexpr auto PARAMETERS = "parameters";
	static constexpr auto OUT_INFO = "output_info";
	static constexpr auto PLOT_INFO = "plot_info";
	static constexpr auto MEASUREMENTS = "measurements";
};

struct KeysPlotInfo
{
	static constexpr auto ID = "id";
	static constexpr auto TITLE = "title";
	static constexpr auto XLABEL = "xLabel";
	static constexpr auto YLABEL = "yLabel";
	static constexpr auto TYPE = "type";
	static constexpr auto XYVALUES = "xyValues";
	static constexpr auto CHANNELS = "ch";
	static constexpr auto FLAGS = "flags";
};

} // namespace scopy::extprocplugin

#endif // MAPKEYS_H
