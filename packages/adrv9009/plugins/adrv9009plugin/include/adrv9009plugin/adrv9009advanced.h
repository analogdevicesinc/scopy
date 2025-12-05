/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef ADRV9009ADVANCED_H
#define ADRV9009ADVANCED_H

#include "scopy-adrv9009plugin_export.h"
#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <tooltemplate.h>
#include <animatedrefreshbtn.h>
#include <iio.h>

// Forward declarations for section widgets
namespace scopy::adrv9009 {
class TxSettingsWidget;
class RxSettingsWidget;
class OrxSettingsWidget;
} // namespace scopy::adrv9009

namespace scopy::adrv9009 {

class SCOPY_ADRV9009PLUGIN_EXPORT Adrv9009Advanced : public QWidget
{
	Q_OBJECT
public:
	Adrv9009Advanced(iio_device *device, QWidget *parent = nullptr);
	~Adrv9009Advanced();

Q_SIGNALS:
	void readRequested();

private:
	iio_device *m_device = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	AnimatedRefreshBtn *m_refreshButton;
	QStackedWidget *m_centralWidget;

	// Navigation buttons (13 sections)
	QPushButton *m_clkSettingsBtn = nullptr;
	QPushButton *m_calibrationsBtn = nullptr;
	QPushButton *m_txSettingsBtn = nullptr;
	QPushButton *m_rxSettingsBtn = nullptr;
	QPushButton *m_orxSettingsBtn = nullptr;
	QPushButton *m_fhmSetupBtn = nullptr;
	QPushButton *m_paProtectionBtn = nullptr;
	QPushButton *m_gainSetupBtn = nullptr;
	QPushButton *m_agcSetupBtn = nullptr;
	QPushButton *m_gpioConfigBtn = nullptr;
	QPushButton *m_auxDacBtn = nullptr;
	QPushButton *m_jesd204SettingsBtn = nullptr;
	QPushButton *m_bistBtn = nullptr;

	// Content widgets (placeholders for Phase 1)
	QWidget *m_clkSettings = nullptr;
	QWidget *m_calibrations = nullptr;
	QWidget *m_fhmSetup = nullptr;
	QWidget *m_paProtection = nullptr;
	QWidget *m_gainSetup = nullptr;
	QWidget *m_agcSetup = nullptr;
	QWidget *m_gpioConfig = nullptr;
	TxSettingsWidget *m_txSettings = nullptr;
	RxSettingsWidget *m_rxSettings = nullptr;
	OrxSettingsWidget *m_orxSettings = nullptr;
	QWidget *m_auxDac = nullptr;
	QWidget *m_jesd204Settings = nullptr;
	QWidget *m_bist = nullptr;

	void setupUi();
	void createNavigationButtons();
	void createContentWidgets();
	QWidget *createPlaceholderWidget(const QString &sectionName);
};

} // namespace scopy::adrv9009
#endif // ADRV9009ADVANCED_H