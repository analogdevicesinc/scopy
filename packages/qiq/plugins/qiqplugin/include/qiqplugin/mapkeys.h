#ifndef MAPKEYS_H
#define MAPKEYS_H

namespace scopy::qiqplugin {

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
	static constexpr auto CH_X = "x";
	static constexpr auto CH_Y = "y";
};

} // namespace scopy::qiqplugin

#endif // MAPKEYS_H
