#ifndef PLOTCOMPONENT_H
#define PLOTCOMPONENT_H

#include "cursorcontroller.h"
#include "scopy-adc_export.h"
#include <gui/plotwidget.h>
#include "toolcomponent.h"
#include <QWidget>
#include <QList>

namespace scopy {
namespace adc {

class ChannelComponent;
class PlotComponent;


class SCOPY_ADC_EXPORT PlotComponentChannel {
public:
	virtual ChannelComponent *channelComponent() = 0;
	virtual void enable() = 0;
	virtual void disable() = 0;
	virtual void onNewData(const float *xData_, const float *yData_, size_t size, bool copy) = 0;
	virtual PlotComponent* plotComponent() = 0;
	virtual PlotChannel* plotChannel() = 0;
	virtual void initPlotComponent(PlotComponent *plotComponent) = 0;
	virtual void deinitPlotComponent() = 0;
};

class SCOPY_ADC_EXPORT PlotComponent : public QWidget, public MetaComponent {
	Q_OBJECT
public:
	PlotComponent(QString name, uint32_t uuid, QWidget *parent = nullptr);
	~PlotComponent();

	PlotWidget *plot(int idx);
	QPair<double, double> xInterval();

public Q_SLOTS:
	virtual void replot();
	virtual void showPlotLabels(bool b);
	virtual void setName(QString s);
	virtual void refreshAxisLabels();
	virtual void selectChannel(ChannelComponent *c);
	virtual void setXInterval(double min, double max);
	virtual void setXInterval(QPair<double,double> p);
	virtual void setXUnit(QString s);

Q_SIGNALS:
	void nameChanged(QString);
	void requestDeletePlot();

public:
	virtual void onStart();
	virtual void onStop();
	virtual void onInit();
	virtual void onDeinit();

	virtual void addChannel(ChannelComponent *);
	virtual void removeChannel(ChannelComponent *);

	uint32_t uuid();
	// TimePlotComponentSettings *createPlotMenu(QWidget *parent);
	// TimePlotComponentSettings *plotMenu();

	CursorController *cursor() const;

	QList<PlotWidget *> plots() const;

protected:
	uint32_t m_uuid;
	QHBoxLayout *m_plotLayout;
	QList<PlotWidget*> m_plots;

	QList<PlotComponentChannel *> m_channels;
	CursorController *m_cursor;
};
}}
#endif // PLOTCOMPONENT_H
