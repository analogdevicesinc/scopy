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

#ifndef SCOPY_SWIOTINFOPAGE_H
#define SCOPY_SWIOTINFOPAGE_H

#include <QCheckBox>
#include <QPushButton>
#include <QWidget>

#include <infopage.h>

namespace scopy::swiotrefactor {
class SwiotInfoPage : public InfoPage
{
	Q_OBJECT
public:
	explicit SwiotInfoPage(QWidget *parent = nullptr);
	void enableTemperatureReadBtn(bool enable);

Q_SIGNALS:
	void temperatureReadEnabled(bool toggled);

private:
	void initTempCheckBox();

	QCheckBox *m_enTempReadCheckBox;
};
} // namespace scopy::swiotrefactor

#endif // SCOPY_SWIOTINFOPAGE_H
