#ifndef CHANNELCOMPONENT_H
#define CHANNELCOMPONENT_H

#include <scopy-adc_export.h>
#include <QWidget>
#include "toolcomponent.h"
#include "timeplotcomponent.h"
#include "menucontrolbutton.h"
#include "menucollapsesection.h"
#include "menuplotchannelcurvestylecontrol.h"
#include "menuplotaxisrangecontrol.h"
#include "menuwidget.h"


namespace scopy {
namespace adc {

class TimePlotComponentChannel;


class GRChannel
{
public:
	virtual GRSignalPath *sigpath() = 0;
	virtual void onNewData(const float *xData, const float *yData, size_t size, bool copy) = 0;
};

class SCOPY_ADC_EXPORT ChannelComponent : public QWidget, public ToolComponent, public Menu
{
	Q_OBJECT
public:
	ChannelComponent(QString ch, TimePlotComponent *m_plot, QPen pen, QWidget *parent = nullptr);
	virtual ~ChannelComponent();

	QPen pen() const;
	ChannelData *chData() const;

	TimePlotComponentChannel *plotChannelCmpt() const;
	void setPlotChannelCmpt(TimePlotComponentChannel *newPlotChannelCmpt);

	virtual MenuControlButton* ctrl();
	virtual void addChannelToPlot();
	virtual void removeChannelFromPlot();

	MenuWidget *menu() override;
	static void createMenuControlButton(ChannelComponent *c, QWidget *parent = nullptr);

protected:
	QString m_channelName;
	QPen m_pen;
	QWidget *widget;
	MenuControlButton *m_ctrl;
	MenuWidget *m_menu;

	ChannelData *m_chData;
	TimePlotComponentChannel *m_plotChannelCmpt;

	void initMenu(QWidget *parent = nullptr);

public Q_SLOTS:
	virtual void enable() override;
	virtual void disable() override;
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onInit() override;
	virtual void onDeinit() override;

	// void onNewData(const float *xData, const float *yData, int size, bool latch);
};


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


	bool m_yLock;

	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);

};

} // namespace adc
} // namespace scopy
#endif // CHANNELCOMPONENT_H
