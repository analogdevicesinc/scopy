/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#ifndef GRFFTDEVICECOMPONENT_H
#define GRFFTDEVICECOMPONENT_H

#include "scopy-adc_export.h"
#include "grdevicecomponent.h"

namespace scopy::adc {

class SCOPY_ADC_EXPORT GRFFTDeviceComponent : public GRDeviceComponent
{
	Q_OBJECT
public:
	GRFFTDeviceComponent(GRIIODeviceSourceNode *node, QWidget *parent = nullptr);
	~GRFFTDeviceComponent() = default;

protected:
	QWidget *buildExtraMenu(QWidget *parent) override;

private:
	QWidget *createMarkerMenu(QWidget *parent);
	QWidget *createAveragingMenu(QWidget *parent);
	QWidget *createMinMaxHoldMenu(QWidget *parent);
};

} // namespace scopy::adc

#endif // GRFFTDEVICECOMPONENT_H
