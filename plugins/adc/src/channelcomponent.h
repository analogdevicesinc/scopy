#ifndef CHANNELCOMPONENT_H
#define CHANNELCOMPONENT_H

#include <scopy-adc_export.h>
#include <QWidget>
#include "toolcomponent.h"
#include "plotcomponent.h"
#include "menucontrolbutton.h"
#include "menuwidget.h"

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT GRChannel : public DataProcessor
{
public:
	virtual GRSignalPath *sigpath() = 0;
};

class TimePlotComponentChannel;

class SCOPY_ADC_EXPORT ChannelComponent : public QWidget, public ToolComponent, public Menu, public SamplingInfoComponent, public DataProcessor
{
	Q_OBJECT
public:
	ChannelComponent(QString ch, QPen pen, QWidget *parent = nullptr);
	virtual ~ChannelComponent();

	QPen pen() const;
	ChannelData *chData() const;

	PlotComponentChannel *plotChannelCmpt() const;
	void setPlotChannelCmpt(PlotComponentChannel *newPlotChannelCmpt);

	virtual MenuControlButton *ctrl();
	virtual void addChannelToPlot();
	virtual void removeChannelFromPlot();
	virtual void setMenuControlButtonVisible(bool b);

	MenuWidget *menu() override;
	static void createMenuControlButton(ChannelComponent *c, QWidget *parent = nullptr);

	virtual SamplingInfo samplingInfo() override;
	virtual void setSamplingInfo(SamplingInfo p) override;

protected:
	QString m_channelName;
	QPen m_pen;
	QWidget *widget;
	MenuControlButton *m_ctrl;
	MenuWidget *m_menu;

	ChannelData *m_chData;

	PlotComponentChannel *m_plotChannelCmpt;
	DataProcessor *m_dataProcessor;

	SamplingInfo m_samplingInfo;

	void initMenu(QWidget *parent = nullptr);

public Q_SLOTS:
	virtual void enable() override;
	virtual void disable() override;
	virtual void onStart() override;
	virtual void onStop() override;
	virtual void onInit() override;
	virtual void onDeinit() override;
	virtual void onNewData(const float *xData, const float *yData, size_t size, bool copy) override;
};

} // namespace adc
} // namespace scopy
#endif // CHANNELCOMPONENT_H
