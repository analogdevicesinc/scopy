#ifndef TIMEPLOTCOMPONENT_H
#define TIMEPLOTCOMPONENT_H

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

using namespace scopy::gui;
namespace scopy {
namespace adc {

class TimePlotComponentSettings;
class TimePlotComponentChannel;
class PlotComponentChannel;
class ChannelComponent;

class SCOPY_ADC_EXPORT TimePlotComponent : public PlotComponent
{
	Q_OBJECT
public:
	TimePlotComponent(QString name, uint32_t uuid, QWidget *parent = nullptr);
	~TimePlotComponent();

	virtual PlotWidget *timePlot();
	virtual PlotWidget *xyPlot();

public Q_SLOTS:
	void setSingleYModeAll(bool b);
	void showXSourceOnXy(bool b);

	ChannelComponent *XYXChannel();
	void setXYXChannel(ChannelComponent *c);
	void refreshXYXAxis();
	void refreshXYXData();
	void selectChannel(ChannelComponent *c) override;
	void setXInterval(QPair<double,double>p) override;
	void setXInterval(double min, double max) override;
public:
	void addChannel(ChannelComponent *) override;
	void removeChannel(ChannelComponent *) override;

	TimePlotComponentSettings *createPlotMenu(QWidget *parent);
	TimePlotComponentSettings *plotMenu();

	bool singleYMode() const;

private Q_SLOTS:
	void onXyXNewData(const float *xData_, const float *yData_, size_t size, bool copy);

private:
	PlotWidget *m_timePlot;
	PlotWidget *m_xyPlot;
	PlotInfo *m_timeInfo;
	PlotInfo *m_xyInfo;

	TimePlotComponentSettings *m_plotMenu;

	bool m_singleYMode;
	bool m_showXSourceOnXy;

	ChannelComponent *m_XYXChannel;
	const float *xyXData;


private:
	QMetaObject::Connection xyDataConn;
	QMetaObject::Connection xyAxisMinConn;
	QMetaObject::Connection xyAxisMaxConn;
};

} // namespace adc
} // namespace scopy

#endif // TIMEPLOTCOMPONENT_H
