#ifndef TIMEPLOTCOMPONENTCHANNEL_H
#define TIMEPLOTCOMPONENTCHANNEL_H

#include "scopy-adc_export.h"
#include <QObject>
#include <toolcomponent.h>
#include <channelcomponent.h>
#include <timeplotcomponent.h>

namespace scopy {
namespace adc {
class SCOPY_ADC_EXPORT TimePlotComponentChannel : public QObject {
	Q_OBJECT
public:

	TimePlotComponentChannel(ChannelComponent *ch, TimePlotComponent *plotComponent, QObject *parent);
	~TimePlotComponentChannel();

	QWidget *createCurveMenu(QWidget *parent);

public Q_SLOTS:
	void enable();
	void disable();
	void onNewData(const float *xData_, const float *yData_, size_t size, bool copy);
	void setXyXData(const float *);
	void setSingleYMode(bool);
	void refreshData(bool copy);

	void initPlotComponent(TimePlotComponent *plotComponent);
	void deinitPlotComponent();

public:
	PlotChannel *m_timePlotCh = nullptr;
	PlotAxis *m_timePlotYAxis = nullptr;
	PlotAxisHandle *m_timePlotAxisHandle = nullptr;

	PlotChannel *m_xyPlotCh = nullptr;
	PlotAxis *m_xyPlotYAxis = nullptr;

	TimePlotComponent *m_plotComponent = nullptr;
	ChannelComponent *m_ch;
	const float *m_xyXData = 0;
	bool m_singleYMode = false;
	bool m_enabled;
};
}
}
#endif // TIMEPLOTCOMPONENTCHANNEL_H
