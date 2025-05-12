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

#ifndef SETTINGSCUSTOMWIDGET_H
#define SETTINGSCUSTOMWIDGET_H

#include "scopy-m2k-gui_export.h"
#include "customPushButton.h"

#include <QButtonGroup>
#include <QWidget>

using namespace scopy;

namespace Ui {
class SettingsPairWidget;
}

namespace scopy {
namespace m2kgui {
class SCOPY_M2K_GUI_EXPORT SettingsPairWidget : public QWidget
{
	Q_OBJECT

public:
	explicit SettingsPairWidget(QWidget *parent = nullptr);
	~SettingsPairWidget();

	CustomPushButton *getGeneralSettingsBtn();
	QPushButton *getSettingsBtn();

private:
	Ui::SettingsPairWidget *m_ui;
};
} // namespace m2kgui
} // namespace scopy

#endif // SETTINGSPAIRWIDGET_H
