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

#include "settings_pair_widget.hpp"

#include "ui_settings_pair_widget.h"

using namespace scopy::m2kgui;

SettingsPairWidget::SettingsPairWidget(QWidget *parent)
	: QWidget(parent)
	, m_ui(new Ui::SettingsPairWidget)
{
	m_ui->setupUi(this);
}

SettingsPairWidget::~SettingsPairWidget() { delete m_ui; }

CustomPushButton *SettingsPairWidget::getGeneralSettingsBtn() { return m_ui->btnGenSettings; }

QPushButton *SettingsPairWidget::getSettingsBtn() { return m_ui->btnSettings; }

#include "moc_settings_pair_widget.cpp"
