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

#ifndef FFTMENUCONTROLS_H
#define FFTMENUCONTROLS_H

#include "scopy-adc_export.h"
#include <QSpinBox>
#include <QPushButton>
#include <menuspinbox.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/menuonoffswitch.h>

namespace scopy::adc {

struct SCOPY_ADC_EXPORT MarkerMenuControls
{
	MenuSectionCollapseWidget *section;
	MenuCombo *typeCombo;
	gui::MenuSpinbox *countSpin;
	MenuOnOffSwitch *fixedEditSwitch; // shown only for MC_FIXED
};

struct SCOPY_ADC_EXPORT AveragingMenuControls
{
	MenuSectionCollapseWidget *section;
	QSpinBox *sizeSpin;
};

struct SCOPY_ADC_EXPORT MinMaxHoldMenuControls
{
	MenuSectionCollapseWidget *section;
	SmallOnOffSwitch *minSwitch;
	SmallOnOffSwitch *maxSwitch;
	QPushButton *minReset;
	QPushButton *maxReset;
};

SCOPY_ADC_EXPORT MarkerMenuControls buildMarkerMenu(QWidget *parent, bool withImage);

SCOPY_ADC_EXPORT AveragingMenuControls buildAveragingMenu(QWidget *parent);

SCOPY_ADC_EXPORT MinMaxHoldMenuControls buildMinMaxHoldMenu(QWidget *parent);

} // namespace scopy::adc

#endif // FFTMENUCONTROLS_H
