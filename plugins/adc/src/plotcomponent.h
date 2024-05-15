#ifndef PLOTCOMPONENT_H
#define PLOTCOMPONENT_H

#include "measurementpanel.h"
#include "scopy-adcplugin_export.h"
#include "toolcomponent.h"

#include <QFuture>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>

#include <plotwidget.h>
#include "plotinfo.h"
#include "plotaxis.h"
#include "pluginbase/preferences.h"

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

	void addPlot(QString name);
	void removePlot(QString name);

	void addChannel(ChannelComponent*, int idx = 0);
	void moveChannel(ChannelComponent*, int newIdx = 0);
	void removeChannel(ChannelComponent*);

	PlotComponent* plot(QString name);
	PlotComponent* plot(int i);
	QList<PlotComponent*> plots() const;
	MeasurementsPanel *measurePanel() const;
	StatsPanel *statsPanel() const;


public Q_SLOTS:
	void replot();
	void enableMeasurementPanel(bool);
	void enableStatsPanel(bool);

	/*void selectChannel(ChannelComponent* c); {
		PlotChannel *ch =  // find channel in channelmap
		m_plot->selectChannel(ch);
	}*/


private:
	QVBoxLayout *m_lay;
	QList<PlotComponent*> m_plots;
	QList<PlotChannelComponent*> m_channels;
	MeasurementsPanel *m_measurePanel;
	StatsPanel *m_statsPanel;
};

class SCOPY_ADCPLUGIN_EXPORT PlotComponent : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	PlotComponent(QString name = "PlotComponent", QWidget *parent = nullptr);
	~PlotComponent();

	virtual PlotWidget *timePlot();
	virtual PlotWidget *xyPlot();

public Q_SLOTS:
	virtual void replot();
	// virtual double sampleRate()

	void setXYXChannel(ChannelComponent *c);
public:
	void onStart();
	void onStop();
	void onInit();
	void onDeinit();
	void addChannel(ChannelComponent*);
	void removeChannel(ChannelComponent*);
	QWidget *createPlotMenu();

private:
	QHBoxLayout *m_plotLayout;
	PlotWidget *m_timePlot;
	PlotWidget *m_xyPlot;
	TimePlotInfo *m_timeInfo;
	TimePlotInfo *m_xyInfo;

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

};

} // namespace adc
} // namespace scopy

#endif // PLOTCOMPONENT_H
