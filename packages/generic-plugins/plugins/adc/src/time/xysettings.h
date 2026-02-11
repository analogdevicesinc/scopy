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

#ifndef XYSETTINGS_H
#define XYSETTINGS_H

#include <QVBoxLayout>
#include <QWidget>

#include "scopy-adc_export.h"
#include <menucombo.h>
#include <widgets/menuonoffswitch.h>
#include <widgets/menusectionwidget.h>

namespace scopy {
namespace adc {

class SCOPY_ADC_EXPORT XYSettings : public QWidget
{
	Q_OBJECT
public:
	XYSettings(QWidget *parent = nullptr);
	~XYSettings();

	MenuCombo *xAxisSource();
	QAbstractButton *showXSource();

Q_SIGNALS:
	void xAxisSourceChanged(int index);
	void showXSourceToggled(bool checked);

private:
	void initUI();

	QVBoxLayout *m_layout;
	MenuSectionWidget *m_controls;
	MenuCombo *m_xAxisSrc;
	MenuOnOffSwitch *m_xAxisShow;
};

} // namespace adc
} // namespace scopy

#endif // XYSETTINGS_H
