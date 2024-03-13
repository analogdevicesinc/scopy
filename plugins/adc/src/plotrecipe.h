#ifndef PLOTRECIPE_H
#define PLOTRECIPE_H

#include <QWidget>
#include <gr-util/grtopblock.h>
#include <scopy-adcplugin_export.h>

namespace scopy {
class AdcInstrument;

class SCOPY_ADCPLUGIN_EXPORT AcquisitionChannel : public QObject {
/*	Q_OBJECT
public:
	AcquisitionChannel();
	~AcquisitionChannel();

	void enable();
	void disable();
	void setEnable(bool);*/
};



class SCOPY_ADCPLUGIN_EXPORT AcquisitionManager : public QObject {
/*	Q_OBJECT
public:
	AcquisitionManager();
	~AcquisitionManager();

	void run(bool b);
	void start();
	void stop();*/
};



class SCOPY_ADCPLUGIN_EXPORT PlotRecipe : public QObject {
	Q_OBJECT
public:
	PlotRecipe(QObject *parent = nullptr, bool main = false) {}
	~PlotRecipe() {}

	void setPrefix(QString p) { m_prefix = p;}
	QString prefix() { return m_prefix;}

	virtual void init() = 0;
	virtual void deinit() = 0;

	void setAdcInstrument(AdcInstrument *i) { m_instrument = i;}

protected:
	bool main = false;
	AdcInstrument *m_instrument;
	QString m_prefix;
};
}

#endif // PLOTRECIPE_H
