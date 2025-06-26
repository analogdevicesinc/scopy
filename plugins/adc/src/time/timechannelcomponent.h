/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef TIMECHANNELCOMPONENT_H
#define TIMECHANNELCOMPONENT_H

#include "grtimesinkcomponent.h"
#include "iioutil/iiounits.h"
#include "scopy-adc_export.h"
#include <gui/channelcomponent.h>
#include <gui/plotautoscaler.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuplotchannelcurvestylecontrol.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menucontrolbutton.h>
#include "adcinterfaces.h"
#include <iio-widgets/iiowidget.h>
#include <gui/widgets/menuwidget.h>
#include <timechannelsigpath.h>
#include "time/timeplotcomponent.h"

namespace scopy {
namespace adc {

using namespace scopy::gui;
using namespace scopy::datasink;

class GRDeviceAddon;
class SCOPY_ADC_EXPORT TimeChannelComponent : public ChannelComponent,
					      public Channel,
					      public MeasurementProvider,
					      public SampleRateProvider,
					      public ScaleProvider

{
	Q_OBJECT
public:
	TimeChannelComponent(SourceBlock *sourceBlock, uint sourceChannel, uint outputChannel, BlockManager *manager,
			     TimePlotComponent *m_plot, QPen pen, QWidget *parent = nullptr);

	~TimeChannelComponent();

	MeasureManagerInterface *getMeasureManager() override;

	SignalPath *sigpath() override;
	QVBoxLayout *menuLayout();

	YMode ymode() const override;
	void setYMode(YMode newYmode) override;
	bool scaleAvailable() const override;
	double yMin() const override;
	double yMax() const override;
	bool yLock() const override;
	IIOUnit unit() const override;

public Q_SLOTS:
	void enable() override;
	void disable() override;
	void onStart() override;
	void onStop() override;
	void onInit() override;
	void onDeinit() override;

	void onNewData(const float *xData, const float *yData, size_t size, bool copy) override;

	bool sampleRateAvailable() override;
	double sampleRate() override;

	void setSamplingInfo(SamplingInfo p) override;

	void toggleAutoScale();
	void setYModeHelper(YMode mode);

	void addChannelToPlot() override;
	void removeChannelFromPlot() override;

Q_SIGNALS:
	void yModeChanged();

private:
	SourceBlock *m_sourceBlock;
	uint m_sourceChannel;
	BlockManager *m_manager;
	TimeChannelSigpath *m_tch;
	QVBoxLayout *m_layScroll;

	TimeMeasureManager *m_measureMgr;
	MenuPlotAxisRangeControl *m_yCtrl;
	PlotAutoscaler *m_autoscaler;
	MenuOnOffSwitch *m_autoscaleBtn;
	MenuCombo *m_ymodeCb;
	IIOWidget *m_scaleWidget;

	MenuPlotChannelCurveStyleControl *m_curvemenu;
	MenuSectionCollapseWidget *m_yaxisMenu;
	TimePlotComponentChannel *m_timePlotComponentChannel;

	QPushButton *m_snapBtn;

	bool m_yLock;
	bool m_scaleAvailable;
	bool m_autoscaleEnabled;
	bool m_running;

	IIOUnit m_unit;

	QWidget *createMenu(QWidget *parent = nullptr);
	QWidget *createAttrMenu(QWidget *parent);
	QWidget *createYAxisMenu(QWidget *parent);
	QWidget *createCurveMenu(QWidget *parent);
	QPushButton *createSnapshotButton(QWidget *parent);

	Q_PROPERTY(YMode ymode READ ymode WRITE setYMode NOTIFY yModeChanged);

	YMode m_ymode;
};

} // namespace adc
} // namespace scopy
#endif // TIMECHANNELCOMPONENT_H
