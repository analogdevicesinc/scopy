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

#ifndef FFTPLOTCOMPONENTSETTINGS_H
#define FFTPLOTCOMPONENTSETTINGS_H

#include <QWidget>
#include "scopy-adc_export.h"
#include "menuspinbox.h"
#include "plotautoscaler.h"
#include <toolcomponent.h>
#include "channelcomponent.h"
#include <widgets/menuplotaxisrangecontrol.h>
#include <widgets/menuplotchannelcurvestylecontrol.h>

namespace scopy {
namespace adc {

class FFTPlotComponent;
class SCOPY_ADC_EXPORT FFTPlotComponentSettings : public QWidget, public ToolComponent
{
	Q_OBJECT
public:
	FFTPlotComponentSettings(FFTPlotComponent *plt, QWidget *parent = nullptr);
	~FFTPlotComponentSettings();

	void showDeleteButtons(bool b);
	void onStart() override;
	void onStop() override;

public Q_SLOTS:
	void addChannel(ChannelComponent *c);
	void removeChannel(ChannelComponent *c);

Q_SIGNALS:
	void requestDeletePlot();
	void requestSettings();

private:
	FFTPlotComponent *m_plotComponent;
	MenuPlotAxisRangeControl *m_yCtrl;
	MenuPlotChannelCurveStyleControl *m_curve;
	MenuSpinbox *m_yPwrOffset;
	PlotAutoscaler *m_autoscaler;

	MenuCombo *m_windowCb;
	MenuOnOffSwitch *m_windowChkb;

	QList<ChannelComponent *> m_channels;
	QPushButton *m_deletePlot;
	QPushButton *m_deletePlotHover;
	QPushButton *m_settingsPlotHover;

	bool m_autoscaleEnabled;
	bool m_running;

private:
	void toggleAutoScale();
	void updateYModeCombo();

	QMap<ChannelComponent *, QList<QMetaObject::Connection>> connections;
};
} // namespace adc
} // namespace scopy

#endif // FFTPLOTCOMPONENTSETTINGS_H
