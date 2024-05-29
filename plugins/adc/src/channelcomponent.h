#ifndef CHANNELCOMPONENT_H
#define CHANNELCOMPONENT_H

#include <scopy-adcplugin_export.h>
#include <QWidget>
#include "toolcomponent.h"
#include "timeplotcomponent.h"


namespace scopy {
namespace adc {

class PlotChannelComponent;

class SCOPY_ADCPLUGIN_EXPORT ChannelComponent : public QWidget,  public ToolComponent
{
	Q_OBJECT
public:
	ChannelComponent(QString ch, PlotComponent *m_plot, QPen pen, QWidget *parent = nullptr);
	virtual ~ChannelComponent();

	QWidget *getMenuControlWidget();
	void setMenuControlWidget(QWidget *);

	QPen pen() const;
	bool enabled() const;

	// virtual QWidget *createCurveMenu(QWidget *parent);

	ChannelData *chData() const;

	PlotChannelComponent *plotChannelCmpt() const;
	void setPlotChannelCmpt(PlotChannelComponent *newPlotChannelCmpt);

protected:

	QString m_channelName;
	QPen m_pen;
	QWidget *widget;
	QWidget *m_mcw;
	bool m_enabled;

	ChannelData *m_chData;
	PlotChannelComponent *m_plotChannelCmpt;

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
