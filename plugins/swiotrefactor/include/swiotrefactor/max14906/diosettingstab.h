/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#ifndef DIOSETTINGSTAB_H
#define DIOSETTINGSTAB_H

#include "scopy-swiotrefactor_export.h"
#include <QWidget>

#include <gui/spinbox_a.hpp>

namespace scopy::swiotrefactor {
class SCOPY_SWIOTREFACTOR_EXPORT DioSettingsTab : public QWidget
{
	Q_OBJECT
public:
	explicit DioSettingsTab(QWidget *parent = nullptr);

	~DioSettingsTab();

	double getTimeValue() const;

Q_SIGNALS:

	void timeValueChanged(double value);

private:
	PositionSpinButton *m_maxSpinButton;
};
} // namespace scopy::swiotrefactor
#endif // DIOSETTINGSTAB_H
