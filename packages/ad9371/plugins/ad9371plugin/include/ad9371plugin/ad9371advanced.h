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

#ifndef AD9371ADVANCED_H
#define AD9371ADVANCED_H

#include "scopy-ad9371plugin_export.h"
#include <QWidget>
#include <QStackedWidget>
#include <QPushButton>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <tooltemplate.h>
#include <animatedrefreshbtn.h>
#include <iio.h>

namespace scopy {
class IIOWidgetGroup;
}

// Forward declarations for section widgets
namespace scopy::ad9371 {
class ClkSettingsWidget;
class CalibrationWidget;
class TxSettingsWidget;
class DpdSettingsWidget;
class ClgcSettingsWidget;
class VswrSettingsWidget;
class RxSettingsWidget;
class ObsSettingsWidget;
class GainSetupWidget;
class AgcSetupWidget;
class ArmGpioWidget;
class GpioWidget;
class AuxDacWidget;
class JesdFramerWidget;
class JesdDeframerWidget;
class BistWidget;

class SCOPY_AD9371PLUGIN_EXPORT Ad9371Advanced : public QWidget
{
	Q_OBJECT
public:
	Ad9371Advanced(iio_device *device, IIOWidgetGroup *group = nullptr, QWidget *parent = nullptr);
	~Ad9371Advanced();

	void switchToSection(const QString &name);
	QStringList getSections() const;

Q_SIGNALS:
	void readRequested();

private:
	iio_device *m_device = nullptr;
	IIOWidgetGroup *m_widgetGroup = nullptr;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	AnimatedRefreshBtn *m_refreshButton;
	QStackedWidget *m_centralWidget;

	// Navigation buttons (16 sections)
	QPushButton *m_clkSettingsBtn = nullptr;
	QPushButton *m_calibrationsBtn = nullptr;
	QPushButton *m_txSettingsBtn = nullptr;
	QPushButton *m_rxSettingsBtn = nullptr;
	QPushButton *m_obsSettingsBtn = nullptr;
	QPushButton *m_gainSetupBtn = nullptr;
	QPushButton *m_agcSetupBtn = nullptr;
	QPushButton *m_armGpioBtn = nullptr;
	QPushButton *m_gpioBtn = nullptr;
	QPushButton *m_auxDacBtn = nullptr;
	QPushButton *m_jesdFramerBtn = nullptr;
	QPushButton *m_jesdDeframerBtn = nullptr;
	QPushButton *m_bistBtn = nullptr;
	QPushButton *m_dpdSettingsBtn = nullptr;
	QPushButton *m_clgcSettingsBtn = nullptr;
	QPushButton *m_vswrSettingsBtn = nullptr;

	// Navigation layout widgets
	QWidget *m_firstRow = nullptr;
	QWidget *m_secondRow = nullptr;
	QHBoxLayout *m_firstRowLayout = nullptr;
	QHBoxLayout *m_secondRowLayout = nullptr;
	QSpacerItem *m_navigationSpacerItem = nullptr;

	// Content widgets
	ClkSettingsWidget *m_clkSettings = nullptr;
	CalibrationWidget *m_calibrations = nullptr;
	TxSettingsWidget *m_txSettings = nullptr;
	RxSettingsWidget *m_rxSettings = nullptr;
	ObsSettingsWidget *m_obsSettings = nullptr;
	GainSetupWidget *m_gainSetup = nullptr;
	AgcSetupWidget *m_agcSetup = nullptr;
	ArmGpioWidget *m_armGpio = nullptr;
	GpioWidget *m_gpio = nullptr;
	AuxDacWidget *m_auxDac = nullptr;
	JesdFramerWidget *m_jesdFramer = nullptr;
	JesdDeframerWidget *m_jesdDeframer = nullptr;
	BistWidget *m_bist = nullptr;
	DpdSettingsWidget *m_dpdSettings = nullptr;
	ClgcSettingsWidget *m_clgcSettings = nullptr;
	VswrSettingsWidget *m_vswrSettings = nullptr;

	void setupUi();
	void createNavigationButtons();
	void createContentWidgets();
	void updateNavigationButtonsLayout();
	QVBoxLayout *navLayout = nullptr;

	void resizeEvent(QResizeEvent *event) override;
};

} // namespace scopy::ad9371
#endif // AD9371ADVANCED_H
