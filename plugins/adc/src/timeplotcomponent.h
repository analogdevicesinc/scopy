#ifndef TIMEPLOTCOMPONENT_H
#define TIMEPLOTCOMPONENT_H

#include "measurementpanel.h"
#include "scopy-adcplugin_export.h"
#include "toolcomponent.h"

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
#include "plotaxis.h"

using namespace	scopy::gui;
namespace scopy {
namespace adc {


class PlotComponent;
class PlotChannelComponent;
class ChannelComponent;

class SCOPY_ADCPLUGIN_EXPORT PlotComponentManager : public QWidget, public MetaComponent {
	Q_OBJECT
public:
	PlotComponentManager(QString name = "PlotComponentManager", QWidget *parent = nullptr);
	~PlotComponentManager();

	uint32_t addPlot(QString name);
	void removePlot(uint32_t uuid);

	void addChannel(ChannelComponent*, uint32_t uuid = 0);
	void moveChannel(ChannelComponent*, uint32_t uuid = 0);
	void removeChannel(ChannelComponent*);

	// PlotComponent* plot(QString name);
	PlotComponent* plot(uint32_t uuid);

	QList<PlotComponent*> plots() const;
	MeasurementsPanel *measurePanel() const;
	StatsPanel *statsPanel() const;

	QWidget *createMenu(QWidget *parent);


public Q_SLOTS:
	void replot();
	void enableMeasurementPanel(bool);
	void enableStatsPanel(bool);

	void setXInterval(double xMin, double xMax);
	void selectChannel(ChannelComponent* c);


private:
	uint32_t m_plotIdx;
	QVBoxLayout *m_lay;
	QList<PlotComponent*> m_plots;
	QList<PlotChannelComponent*> m_channels;
	MeasurementsPanel *m_measurePanel;
	StatsPanel *m_statsPanel;
	// PlotSettings *m_plotSettings;
};

class SCOPY_ADCPLUGIN_EXPORT PlotComponent : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	PlotComponent(QString name, uint32_t uuid, QWidget *parent = nullptr);
	~PlotComponent();

	virtual PlotWidget *timePlot();
	virtual PlotWidget *xyPlot();

public Q_SLOTS:
	virtual void replot();
	// virtual double sampleRate()


	ChannelComponent* XYXChannel();
	void setXYXChannel(ChannelComponent *c);
public:
	void onStart();
	void onStop();
	void onInit();
	void onDeinit();

	void addChannel(ChannelComponent*);
	void removeChannel(ChannelComponent*);

	QWidget *createPlotMenu(QWidget *parent = nullptr);
	uint32_t uuid();
	QWidget *plotMenu();
	void deletePlotMenu();

	bool singleYMode() const;

private:
	uint32_t m_uuid;
	QHBoxLayout *m_plotLayout;
	PlotWidget *m_timePlot;
	PlotWidget *m_xyPlot;
	PlotInfo *m_timeInfo;
	PlotInfo *m_xyInfo;

	QWidget *m_plotMenu;
	MenuCombo *m_xAxisSrc;
	MenuOnOffSwitch *m_xAxisShow;


	bool m_singleYMode;
	PlotAutoscaler *m_autoscaler;
	ChannelComponent *m_XYXChannel;
	const float *xyXData;

	QList<PlotChannelComponent*> m_channels;

private:
	QMetaObject::Connection xyDataConn;
};

class SCOPY_ADCPLUGIN_EXPORT PlotChannelComponent : public QObject, public ToolComponent {
	Q_OBJECT
public:

	PlotChannelComponent(ChannelComponent *ch, PlotComponent *plotComponent, QObject *parent);
	~PlotChannelComponent();

	QWidget *createCurveMenu(QWidget *parent);	

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onNewData(const float *xData_, const float *yData_, size_t size, bool copy);
	void setXyXData(const float *);
	void setSingleYMode(bool);
	void refreshData(bool copy);

	void initPlotComponent(PlotComponent *plotComponent);
	void deinitPlotComponent();

public:
	PlotChannel *m_timePlotCh = nullptr;
	PlotAxis *m_timePlotYAxis = nullptr;
	PlotAxisHandle *m_timePlotAxisHandle = nullptr;

	PlotChannel *m_xyPlotCh = nullptr;
	PlotAxis *m_xyPlotYAxis = nullptr;

	PlotComponent *m_plotComponent = nullptr;
	ChannelComponent *m_ch;
	const float *m_xyXData = 0;
	bool m_singleYMode = false;
};

} // namespace adc
} // namespace scopy


#endif // TIMEPLOTCOMPONENT_H
