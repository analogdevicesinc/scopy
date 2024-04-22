#ifndef CHANNELCOMPONENT_H
#define CHANNELCOMPONENT_H

#include <scopy-adcplugin_export.h>
#include <QWidget>
#include "plotcomponent.h"
#include "toolcomponent.h"
#include "plotchannel.h"

namespace scopy {
namespace adc {
class SCOPY_ADCPLUGIN_EXPORT ChannelComponent : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	ChannelComponent(QString ch, PlotComponent *plotComponent, QPen pen, QWidget *parent = nullptr);
	virtual ~ChannelComponent();

	QWidget *getMenuControlWidget();
	void setMenuControlWidget(QWidget *);

	QPen pen() const;
	bool enabled() const;
	PlotChannel *plotCh() const;

	virtual QWidget *createCurveMenu(QWidget *parent);

protected:
	QString m_channelName;
	PlotChannel *m_plotCh;
	PlotAxis *m_plotAxis;
	PlotAxisHandle *m_plotAxisHandle;
	PlotComponent *m_plotComponent;
	QPen m_pen;
	QWidget *widget;
	QWidget *m_mcw;

	bool m_enabled;

public Q_SLOTS:
	virtual void enableChannel();
	virtual void disableChannel();
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onInit() override;
	virtual void onDeinit() override;

	void onNewData(const float *xData, const float *yData, int size);
};
}
}
#endif // CHANNELCOMPONENT_H
