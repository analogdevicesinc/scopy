#ifndef SPECTRUM_ANALYZER_API_HPP
#define SPECTRUM_ANALYZER_API_HPP

#include "spectrum_analyzer.hpp"

namespace adiscope {
class SpectrumAnalyzer_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool running READ running WRITE run STORED false);
	Q_PROPERTY(bool single READ single WRITE runSingle STORED false);
	Q_PROPERTY(double startFreq READ startFreq WRITE setStartFreq);
	Q_PROPERTY(double stopFreq  READ stopFreq  WRITE setStopFreq);
	Q_PROPERTY(QString units READ units WRITE setUnits);
	Q_PROPERTY(QString resBW READ resBW WRITE setResBW);
	Q_PROPERTY(double topScale READ topScale WRITE setTopScale);
	Q_PROPERTY(double range READ range WRITE setRange);
	Q_PROPERTY(QVariantList channels READ getChannels);
	Q_PROPERTY(int currentChannel READ currentChannel WRITE setCurrentChannel);
	Q_PROPERTY(bool markerTableVisible READ markerTableVisible WRITE
	           setMarkerTableVisible);
	Q_PROPERTY(QVariantList markers READ getMarkers);
public:
	Q_INVOKABLE void show();
	explicit SpectrumAnalyzer_API(SpectrumAnalyzer *sp) :
		ApiObject(), sp(sp) {}
	~SpectrumAnalyzer_API() {}

private:
	SpectrumAnalyzer *sp;
	bool running();
	void run(bool);
	bool single();
	void runSingle(bool);
	QVariantList getChannels();
	QVariantList getMarkers();

	int currentChannel();
	void setCurrentChannel(int);

	double startFreq();
	void setStartFreq(double);

	double stopFreq();
	void setStopFreq(double);

	QString units();
	void setUnits(QString);

	QString resBW();
	void setResBW(QString);

	double topScale();
	void setTopScale(double);

	double range();
	void setRange(double);

	bool markerTableVisible();
	void setMarkerTableVisible(bool);
};
}

#endif // SPECTRUM_ANALYZER_API_HPP
