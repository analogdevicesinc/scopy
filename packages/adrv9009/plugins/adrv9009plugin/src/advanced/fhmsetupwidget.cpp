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

#include "advanced/fhmsetupwidget.h"
#include "adrv9009widgetfactory.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_FHMSETUP, "FHMSetup")

using namespace scopy;
using namespace scopy::adrv9009;

FhmSetupWidget::FhmSetupWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_FHMSETUP) << "No device provided to FHM Setup widget";
		return;
	}

	setupUi();
}

FhmSetupWidget::~FhmSetupWidget() {}

void FhmSetupWidget::setupUi()
{
	// Main layout for this widget
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// Create scroll area for all sections
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create content widget for scroll area
	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	// Add Settings section
	contentLayout->addWidget(createSettingsSection(contentWidget));

	// Add Config section
	contentLayout->addWidget(createConfigSection(contentWidget));

	// Add spacer to push sections to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	// Store first section as m_section for compatibility
	m_section = qobject_cast<MenuSectionCollapseWidget *>(contentLayout->itemAt(0)->widget());

	qDebug(CAT_FHMSETUP) << "FHM Setup widget created with 8 attributes in 2 sections";
}

QWidget *FhmSetupWidget::createSettingsSection(QWidget *parent)
{
	// Settings section (5 attributes - checkboxes and frequency)
	MenuSectionCollapseWidget *settingsSection = new MenuSectionCollapseWidget(
		"Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// FHM Enable - Checkbox
	auto fhmEnable = Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,fhm-mode-fhm-enable", "FHM ENABLE");
	if(fhmEnable) {
		settingsSection->contentLayout()->addWidget(fhmEnable);
		connect(this, &FhmSetupWidget::readRequested, fhmEnable, &IIOWidget::readAsync);
	}

	// Enable MCS Sync - Checkbox
	auto mcsSyncEnable = Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,fhm-mode-enable-mcs-sync",
									 "ENABLE MCS SYNC");
	if(mcsSyncEnable) {
		settingsSection->contentLayout()->addWidget(mcsSyncEnable);
		connect(this, &FhmSetupWidget::readRequested, mcsSyncEnable, &IIOWidget::readAsync);
	}

	// FHM Trigger Mode - Checkbox
	auto triggerMode = Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,fhm-mode-fhm-trigger-mode",
								       "FHM TRIGGER MODE");
	if(triggerMode) {
		settingsSection->contentLayout()->addWidget(triggerMode);
		connect(this, &FhmSetupWidget::readRequested, triggerMode, &IIOWidget::readAsync);
	}

	// FHM Exit Mode - Checkbox
	auto exitMode =
		Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,fhm-mode-fhm-exit-mode", "FHM EXIT MODE");
	if(exitMode) {
		settingsSection->contentLayout()->addWidget(exitMode);
		connect(this, &FhmSetupWidget::readRequested, exitMode, &IIOWidget::readAsync);
	}

	// FHM Init Frequency (Hz) - Range Widget
	auto initFreq = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,fhm-mode-fhm-init-frequency_hz",
								 "[100000000 1 6000000000]", "FHM INIT FREQUENCY (Hz)");
	if(initFreq) {
		settingsSection->contentLayout()->addWidget(initFreq);
		connect(this, &FhmSetupWidget::readRequested, initFreq, &IIOWidget::readAsync);
	}

	return settingsSection;
}

QWidget *FhmSetupWidget::createConfigSection(QWidget *parent)
{
	// Config section (3 attributes - GPIO pin and frequency ranges)
	MenuSectionCollapseWidget *configSection = new MenuSectionCollapseWidget(
		"Config", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	// FHM GPIO Pin - Range Widget [0 1 15]
	auto gpioPin = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,fhm-config-fhm-gpio-pin", "[0 1 15]",
								"FHM GPIO PIN");
	if(gpioPin) {
		configSection->contentLayout()->addWidget(gpioPin);
		connect(this, &FhmSetupWidget::readRequested, gpioPin, &IIOWidget::readAsync);
	}

	// FHM Min Frequency (MHz) - Range Widget [100 1 6000]
	auto minFreq = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,fhm-config-fhm-min-freq_mhz",
								"[100 1 6000]", "FHM MIN FREQ (MHz)");
	if(minFreq) {
		configSection->contentLayout()->addWidget(minFreq);
		connect(this, &FhmSetupWidget::readRequested, minFreq, &IIOWidget::readAsync);
	}

	// FHM Max Frequency (MHz) - Range Widget [100 1 6000]
	auto maxFreq = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,fhm-config-fhm-max-freq_mhz",
								"[100 1 6000]", "FHM MAX FREQ (MHz)");
	if(maxFreq) {
		configSection->contentLayout()->addWidget(maxFreq);
		connect(this, &FhmSetupWidget::readRequested, maxFreq, &IIOWidget::readAsync);
	}

	return configSection;
}

MenuSectionCollapseWidget *FhmSetupWidget::section() const { return m_section; }