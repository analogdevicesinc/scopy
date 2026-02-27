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
 *
 */

#ifndef AD9371ADVANCED_H
#define AD9371ADVANCED_H

#include <QBoxLayout>
#include <QPushButton>
#include <QWidget>
#include <animatedrefreshbtn.h>
#include <tooltemplate.h>
#include <iio.h>

#include "clksettingswidget.h"
#include "calibrationswidget.h"
#include "txsettingswidget.h"
#include "rxsettingswidget.h"
#include "obssettingswidget.h"
#include "gainsetupwidget.h"
#include "agcsetupwidget.h"
#include "armgpiowidget.h"
#include "gpiowidget.h"
#include "auxdacwidget.h"
#include "jesdframerwidget.h"
#include "jesddeframerwidget.h"
#include "ad9371bistwidget.h"
#include "dpdsettingswidget.h"
#include "clgcsettingswidget.h"
#include "vswrsettingswidget.h"

namespace scopy {
class IIOWidgetGroup;
namespace ad9371 {

class AD9371Advanced : public QWidget
{
	Q_OBJECT
public:
	explicit AD9371Advanced(iio_context *ctx, IIOWidgetGroup *group = nullptr, bool has_dpd = false,
				QWidget *parent = nullptr);
	~AD9371Advanced();

Q_SIGNALS:
	void readRequested();

private:
	void init();

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

	iio_context *m_ctx = nullptr;
	IIOWidgetGroup *m_group = nullptr;
	bool m_hasDpd;
	ToolTemplate *m_tool;
	QVBoxLayout *m_mainLayout;
	AnimatedRefreshBtn *m_refreshButton;

	ClkSettingsWidget *m_clkSettings = nullptr;
	CalibrationsWidget *m_calibrations = nullptr;
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
	Ad9371BistWidget *m_bist = nullptr;
	DpdSettingsWidget *m_dpdSettings = nullptr;
	ClgcSettingsWidget *m_clgcSettings = nullptr;
	VswrSettingsWidget *m_vswrSettings = nullptr;

	iio_device *m_phyDevice = nullptr;
	QStackedWidget *m_centralWidget = nullptr;

	bool m_isToolInitialized = false;
	void showEvent(QShowEvent *event) override;
};
} // namespace ad9371
} // namespace scopy
#endif // AD9371ADVANCED_H
