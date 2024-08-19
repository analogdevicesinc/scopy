#ifndef FFTPLOTCOMPONENT_H
#define FFTPLOTCOMPONENT_H

#include "scopy-adc_export.h"

#include <QFuture>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QVariant>

#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucombo.h>
#include <gui/plotautoscaler.h>

#include <plotwidget.h>
#include "plotinfo.h"
#include "plotcomponent.h"
#include "fftplotcomponentsettings.h"
#include <gui/widgets/plotinfowidgets.h>

using namespace scopy::gui;
namespace scopy {
namespace adc {

class PlotComponentChannel;
class ChannelComponent;

class SCOPY_ADC_EXPORT FFTPlotComponent : public PlotComponent
{
	Q_OBJECT
public:
	FFTPlotComponent(QString name, uint32_t uuid, QWidget *parent = nullptr);
	~FFTPlotComponent();

	virtual PlotWidget *fftPlot();

public:
	void addChannel(ChannelComponent *) override;
	void removeChannel(ChannelComponent *) override;

	FFTPlotComponentSettings *createPlotMenu(QWidget *parent);
	FFTPlotComponentSettings *plotMenu();

	FFTSamplingInfo *fftPlotInfo() const;

private:
	PlotWidget *m_fftPlot;
	FFTSamplingInfo *m_fftInfo;

	FFTPlotComponentSettings *m_plotMenu;
};
} // namespace adc
} // namespace scopy
#endif // FFTPLOTCOMPONENT_H
