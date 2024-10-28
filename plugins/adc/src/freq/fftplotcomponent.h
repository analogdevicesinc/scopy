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

#ifndef FFTPLOTCOMPONENT_H
#define FFTPLOTCOMPONENT_H

#include "scopy-adc_export.h"

#include <QFuture>
#include <QTimer>
#include <QVBoxLayout>
#include <QtConcurrent>
#include <QVariant>

#include <widgets/menuonoffswitch.h>
#include <widgets/menucombo.h>
#include <plotautoscaler.h>

#include <plotwidget.h>
#include "plotinfo.h"
#include "plotcomponent.h"
#include "fftplotcomponentsettings.h"
#include <widgets/plotinfowidgets.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT FFTPlotComponent : public PlotComponent
{
	Q_OBJECT
public:
	FFTPlotComponent(QString name, uint32_t uuid, QWidget *parent = nullptr);
	~FFTPlotComponent();

	virtual PlotWidget *fftPlot();

public:
	void addChannel(ChannelComponent *) override;
	void removeChannel(ChannelComponent *) override;

	FFTPlotComponentSettings *createPlotMenu(QWidget *parent);
	FFTPlotComponentSettings *plotMenu();

	FFTSamplingInfo *fftPlotInfo() const;

private:
	PlotWidget *m_fftPlot;
	FFTSamplingInfo *m_fftInfo;

	FFTPlotComponentSettings *m_plotMenu;
};
} // namespace adc
} // namespace scopy
#endif // FFTPLOTCOMPONENT_H
