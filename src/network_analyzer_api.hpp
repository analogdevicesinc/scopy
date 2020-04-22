#ifndef NETWORK_ANALYZER_API_HPP
#define NETWORK_ANALYZER_API_HPP

#include <network_analyzer.hpp>

namespace adiscope {

class NetworkAnalyzer_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(double min_freq READ getMinFreq WRITE setMinFreq);
	Q_PROPERTY(double max_freq READ getMaxFreq WRITE setMaxFreq);
	Q_PROPERTY(double samples_count READ getSamplesCount WRITE setSamplesCount);
	Q_PROPERTY(double amplitude READ getAmplitude WRITE setAmplitude);
	Q_PROPERTY(double offset READ getOffset WRITE setOffset);

	Q_PROPERTY(double min_mag READ getMinMag WRITE setMinMag);
	Q_PROPERTY(double max_mag READ getMaxMag WRITE setMaxMag);
	Q_PROPERTY(double min_phase READ getMinPhase WRITE setMinPhase);
	Q_PROPERTY(double max_phase READ getMaxPhase WRITE setMaxPhase);

	Q_PROPERTY(bool log_freq READ isLogFreq WRITE setLogFreq);

	Q_PROPERTY(int ref_channel READ getRefChannel WRITE setRefChannel);

	Q_PROPERTY(bool running READ running WRITE run STORED false);
	Q_PROPERTY(bool cursors READ getCursors WRITE setCursors);
	Q_PROPERTY(int line_thickness READ getLineThickness WRITE setLineThickness);
	Q_PROPERTY(int cursors_position READ getCursorsPosition WRITE setCursorsPosition)
	Q_PROPERTY(int cursors_transparency READ getCursorsTransparency WRITE setCursorsTransparency)
	Q_PROPERTY(int plot_type READ getPlotType WRITE setPlotType)
	Q_PROPERTY(QList<double> data READ data STORED false)
	Q_PROPERTY(QList<double> phase READ phase STORED false)
	Q_PROPERTY(QList<double> freq READ freq STORED false)
public:
	explicit NetworkAnalyzer_API(NetworkAnalyzer* net)
		: ApiObject()
		, net(net)
	{}
	~NetworkAnalyzer_API() {}

	double getMinFreq() const;
	double getMaxFreq() const;
	double getSamplesCount() const;
	double getAmplitude() const;
	double getOffset() const;

	void setMinFreq(double freq);
	void setMaxFreq(double freq);
	void setSamplesCount(double step);
	void setAmplitude(double amp);
	void setOffset(double offset);

	double getMinMag() const;
	double getMaxMag() const;
	double getMinPhase() const;
	double getMaxPhase() const;

	void setMinMag(double val);
	void setMaxMag(double val);
	void setMinPhase(double val);
	void setMaxPhase(double val);

	bool isLogFreq() const;
	void setLogFreq(bool is_log);

	int getRefChannel() const;
	void setRefChannel(int chn);

	bool getCursors() const;
	void setCursors(bool enabled);

	bool running() const;
	void run(bool enabled);

	int getLineThickness() const;
	void setLineThickness(int index);
	int getCursorsPosition() const;
	void setCursorsPosition(int val);

	int getCursorsTransparency() const;
	void setCursorsTransparency(int val);

	int getPlotType() const;
	void setPlotType(int val);

	Q_INVOKABLE void show();

	QList<double> data() const;
	QList<double> freq() const;
	QList<double> phase() const;

private:
	NetworkAnalyzer* net;
};
} // namespace adiscope
#endif // NETWORK_ANALYZER_API_HPP
