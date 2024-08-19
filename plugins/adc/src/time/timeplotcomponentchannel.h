#ifndef TIMEPLOTCOMPONENTCHANNEL_H
#define TIMEPLOTCOMPONENTCHANNEL_H

#include "scopy-adc_export.h"
#include <QObject>
#include <toolcomponent.h>
#include <channelcomponent.h>
#include <timeplotcomponent.h>
#include <plotcomponent.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT TimePlotComponentChannel : public QObject, public PlotComponentChannel
{
	Q_OBJECT
public:
	TimePlotComponentChannel(ChannelComponent *ch, TimePlotComponent *plotComponent, QObject *parent);
	~TimePlotComponentChannel();

	QWidget *createCurveMenu(QWidget *parent);
	ChannelComponent *channelComponent() override;
	PlotComponent *plotComponent() override;
	PlotChannel *plotChannel() override;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onNewData(const float *xData_, const float *yData_, size_t size, bool copy) override;
	void setXyXData(const float *);
	void lockYAxis(bool);
	void refreshData(bool copy);

	void initPlotComponent(PlotComponent *plotComponent) override;
	void deinitPlotComponent() override;

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
} // namespace adc
} // namespace scopy
#endif // TIMEPLOTCOMPONENTCHANNEL_H
