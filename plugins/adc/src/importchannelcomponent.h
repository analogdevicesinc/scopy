#ifndef IMPORTCHANNELCOMPONENT_H
#define IMPORTCHANNELCOMPONENT_H

#include "channelcomponent.h"
#include <gui/plotautoscaler.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>

namespace scopy {
namespace adc {
class SCOPY_ADC_EXPORT ImportChannelComponent : public ChannelComponent
{
	Q_OBJECT
public:
	ImportChannelComponent(ImportFloatChannelNode *ifcn, QPen pen, QWidget *parent = nullptr);
	~ImportChannelComponent();

	virtual void onInit() override;

public Q_SLOTS:
	void forgetChannel();

private:
	ImportFloatChannelNode *m_node;

	QVBoxLayout *m_layScroll;
	MenuPlotChannelCurveStyleControl *m_curvemenu;

	MenuPlotAxisRangeControl *m_yCtrl;
	PlotAutoscaler *m_autoscaler;
	QPushButton *m_autoscaleBtn;
	TimePlotComponentChannel *m_timePlotChannelComponent;

	bool m_yLock;

	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);

	// ChannelComponent interface
public:
	void addChannelToPlot() override;
	void removeChannelFromPlot() override;
};
} // namespace adc
} // namespace scopy

#endif // IMPORTCHANNELCOMPONENT_H
