#ifndef CHANNELCOMPONENT_H
#define CHANNELCOMPONENT_H

#include <scopy-adc_export.h>
#include <QWidget>
#include "toolcomponent.h"
#include "timeplotcomponent.h"


namespace scopy {
namespace adc {

class TimePlotComponentChannel;

class SCOPY_ADC_EXPORT ChannelComponent : public QWidget,  public ToolComponent
{
	Q_OBJECT
public:
	ChannelComponent(QString ch, TimePlotComponent *m_plot, QPen pen, QWidget *parent = nullptr);
	virtual ~ChannelComponent();

	QWidget *getMenuControlWidget();
	void setMenuControlWidget(QWidget *);

	QPen pen() const;

	ChannelData *chData() const;
	TimePlotComponentChannel *plotChannelCmpt() const;
	void setPlotChannelCmpt(TimePlotComponentChannel *newPlotChannelCmpt);

protected:

	QString m_channelName;
	QPen m_pen;
	QWidget *widget;
	QWidget *m_mcw;

	ChannelData *m_chData;
	TimePlotComponentChannel *m_plotChannelCmpt;

public Q_SLOTS:
	virtual void enableChannel();
	virtual void disableChannel();
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onInit() override;
	virtual void onDeinit() override;

	// void onNewData(const float *xData, const float *yData, int size, bool latch);
};

}
}
#endif // CHANNELCOMPONENT_H
