#ifndef TIMEPLOTCOMPONENT_H
#define TIMEPLOTCOMPONENT_H

#include "scopy-adc_export.h"
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

using namespace scopy::gui;
namespace scopy {
namespace adc {

class TimePlotComponent;
class TimePlotComponentSettings;
class TimePlotComponentChannel;
class ChannelComponent;

class SCOPY_ADC_EXPORT TimePlotComponent : public QWidget, public MetaComponent
{
	Q_OBJECT
public:
	TimePlotComponent(QString name, uint32_t uuid, QWidget *parent = nullptr);
	~TimePlotComponent();

	virtual PlotWidget *timePlot();
	virtual PlotWidget *xyPlot();

public Q_SLOTS:
	virtual void replot();
	void showPlotLabels(bool b);
	void setSingleYMode(bool b);
	void showXSourceOnXy(bool b);
	void setName(QString s);
	// virtual double sampleRate()

	ChannelComponent *XYXChannel();
	void setXYXChannel(ChannelComponent *c);
Q_SIGNALS:
	void nameChanged(QString);

public:
	void onStart();
	void onStop();
	void onInit();
	void onDeinit();

	void addChannel(ChannelComponent *);
	void removeChannel(ChannelComponent *);

	uint32_t uuid();
	TimePlotComponentSettings *createPlotMenu(QWidget *parent);
	TimePlotComponentSettings *plotMenu();

	bool singleYMode() const;

private:
	uint32_t m_uuid;
	QHBoxLayout *m_plotLayout;
	PlotWidget *m_timePlot;
	PlotWidget *m_xyPlot;
	PlotInfo *m_timeInfo;
	PlotInfo *m_xyInfo;

	TimePlotComponentSettings *m_plotMenu;

	bool m_singleYMode;
	bool m_showXSourceOnXy;

	ChannelComponent *m_XYXChannel;
	const float *xyXData;

	QList<TimePlotComponentChannel *> m_channels;

private:
	QMetaObject::Connection xyDataConn;
};

} // namespace adc
} // namespace scopy

#endif // TIMEPLOTCOMPONENT_H
