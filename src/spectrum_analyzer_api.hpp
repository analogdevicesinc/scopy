#ifndef SPECTRUM_ANALYZER_API_HPP
#define SPECTRUM_ANALYZER_API_HPP

#include "spectrum_analyzer.hpp"

namespace adiscope {

class SpectrumAnalyzer_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool running READ running WRITE run STORED false);
	Q_PROPERTY(bool single READ isSingle WRITE single STORED false);
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
	Q_PROPERTY(bool logScale READ getLogScale WRITE setLogScale)
public:
	Q_INVOKABLE void show();
	explicit SpectrumAnalyzer_API(SpectrumAnalyzer *sp) :
		ApiObject(), sp(sp) {}
	~SpectrumAnalyzer_API() {}

private:
	SpectrumAnalyzer *sp;
	bool running();
	void run(bool);

	bool isSingle();
	void single(bool);

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

	bool getLogScale() const;
	void setLogScale(bool useLogScale);

};

class SpectrumChannel_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool enabled READ enabled WRITE enable);
	Q_PROPERTY(int type READ type WRITE setType);
	Q_PROPERTY(int window READ window WRITE setWindow);
	Q_PROPERTY(int averaging READ averaging WRITE setAveraging);
	Q_PROPERTY(QList<double> data READ data STORED false)
	Q_PROPERTY(QList<double> freq READ freq STORED false)

public:
	explicit SpectrumChannel_API(SpectrumAnalyzer *sp,
				     boost::shared_ptr<SpectrumChannel> spch) :
		ApiObject(), spch(spch),sp(sp) {}
	~SpectrumChannel_API() {}

	bool enabled();
	int type();
	int window();
	int averaging();

	void enable(bool);
	void setType(int);
	void setWindow(int);
	void setAveraging(int);

	QList<double> data() const;
	QList<double> freq() const;

private:
	SpectrumAnalyzer *sp;
	boost::shared_ptr<SpectrumChannel> spch;
};

class SpectrumMarker_API :public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(int chId READ chId WRITE setChId);
	Q_PROPERTY(int mkId READ mkId WRITE setMkId);
	Q_PROPERTY(bool en READ enabled WRITE setEnabled);
	Q_PROPERTY(bool visible READ visible WRITE setVisible);
	Q_PROPERTY(int type READ type WRITE setType);
	Q_PROPERTY(double freq READ freq WRITE setFreq);
	Q_PROPERTY(double magnitude READ magnitude);

	int m_chid;
	int m_mkid;
	int m_type;
public:
	explicit SpectrumMarker_API(SpectrumAnalyzer *sp,int chid, int mkid) :
		ApiObject(), sp(sp), m_mkid(mkid), m_chid(chid), m_type(0) {}
	~SpectrumMarker_API() {}

	int chId();
	void setChId(int);

	int mkId();
	void setMkId(int);

	int type();
	void setType(int);

	double freq();
	void setFreq(double);

	bool enabled();
	void setEnabled(bool);

	bool visible();
	void setVisible(bool);

	double magnitude();

	SpectrumAnalyzer *sp;

};
}

#endif // SPECTRUM_ANALYZER_API_HPP
