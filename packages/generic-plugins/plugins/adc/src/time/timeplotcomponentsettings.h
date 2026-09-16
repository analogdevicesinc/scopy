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

#ifndef TIMEPLOTCOMPONENTSETTINGS_H
#define TIMEPLOTCOMPONENTSETTINGS_H
#include <QWidget>
#include <timeplotcomponent.h>
#include <gui/toolcomponent.h>
#include "scopy-adc_export.h"
#include "adcinterfaces.h"
#include <gui/widgets/menuplotaxisrangecontrol.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT TimePlotComponentSettings : public QWidget, public ToolComponent
{
	friend class ADC_API;
	Q_OBJECT
public:
	TimePlotComponentSettings(TimePlotComponent *plt, QWidget *parent = nullptr);
	~TimePlotComponentSettings();

	void showDeleteButtons(bool b);
	void showPlotButtons(bool b);

	YMode yMode() const;
	/// Applies mode to every channel on this plot and rescales the Y axis. Unlike driving
	/// the combo directly, this also works when the combo already sits on mode - which is
	/// the case on init, where the combo starts on index 0 and so emits nothing.
	/// Returns false if mode is not one of the combo's current entries.
	bool setYMode(YMode mode);

public Q_SLOTS:
	void addChannel(ChannelComponent *c);
	void removeChannel(ChannelComponent *c);
	void onInit() override;
	void onDeinit() override;
	void onStart() override;
	void onStop() override;
	void updateYAxis();

Q_SIGNALS:
	void requestDeletePlot();
	void requestSettings();

private:
	PlotAutoscaler *m_autoscaler;
	TimePlotComponent *m_plotComponent;
	MenuCombo *m_yModeCb;
	MenuCombo *m_xAxisSrc;
	MenuPlotAxisRangeControl *m_yCtrl;
	MenuOnOffSwitch *m_xAxisShow;
	MenuOnOffSwitch *m_autoscaleBtn;
	QList<ChannelComponent *> m_channels;
	QList<ScaleProvider *> m_scaleProviders;
	QPushButton *m_deletePlot;
	QPushButton *m_deletePlotHover;
	QPushButton *m_settingsPlotHover;
	YMode m_ymode;

	bool m_autoscaleEnabled;
	bool m_running;

private:
	void toggleAutoScale();
	void updateYModeCombo();
	/// Cascades mode to the channels and rescales the axis. Shared by the combo's
	/// currentIndexChanged handler and by setYMode().
	void applyYMode(YMode mode);
};

} // namespace adc
} // namespace scopy

#endif // TIMEPLOTCOMPONENTSETTINGS_H
