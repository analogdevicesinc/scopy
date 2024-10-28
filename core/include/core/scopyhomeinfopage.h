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

#ifndef SCOPYHOMEINFOPAGE_H
#define SCOPYHOMEINFOPAGE_H

#include "scopy-core_export.h"

#include <QPushButton>
#include <QWidget>

#include <hoverwidget.h>

namespace Ui {
class ScopyHomeInfoPage;
}

namespace scopy {
class SCOPY_CORE_EXPORT ScopyHomeInfoPage : public QWidget
{
	Q_OBJECT

public:
	explicit ScopyHomeInfoPage(QWidget *parent = nullptr);
	~ScopyHomeInfoPage();

private:
	Ui::ScopyHomeInfoPage *ui;

	void initReportButton();
};
} // namespace scopy

#endif // SCOPYHOMEINFOPAGE_H
