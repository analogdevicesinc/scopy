#ifndef FFTPLOTCOMPONENTCHANNEL_H
#define FFTPLOTCOMPONENTCHANNEL_H

#include "scopy-gui_export.h"
#include <QObject>
#include "fftplotcomponent.h"
#include <toolcomponent.h>
#include <channelcomponent.h>
#include <plotcomponent.h>
#include <fftmarkercontroller.hpp>

namespace scopy {
namespace adc {

class SCOPY_GUI_EXPORT FFTPlotComponentChannel : public QObject, public PlotComponentChannel
{
	Q_OBJECT
public:
	FFTPlotComponentChannel(ChannelComponent *ch, FFTPlotComponent *plotComponent, QObject *parent);
	~FFTPlotComponentChannel();

	QWidget *createCurveMenu(QWidget *parent);
	ChannelComponent *channelComponent() override;
	PlotComponent *plotComponent() override;
	PlotChannel *plotChannel() override;
	FFTMarkerController *markerController() const;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onNewData(const float *xData_, const float *yData_, size_t size, bool copy) override;
	void lockYAxis(bool);
	void refreshData(bool copy);

	void initPlotComponent(PlotComponent *plotComponent) override;
	void deinitPlotComponent() override;

public:
	PlotChannel *m_fftPlotCh = nullptr;
	PlotAxis *m_fftPlotYAxis = nullptr;
	PlotAxisHandle *m_fftPlotAxisHandle = nullptr;

	FFTPlotComponent *m_plotComponent = nullptr;
	ChannelComponent *m_ch;
	bool m_singleYMode = false;
	bool m_enabled;

private:
	FFTMarkerController *m_markerController;
};
} // namespace adc
} // namespace scopy
#endif // FFTPLOTCOMPONENTCHANNEL_H
