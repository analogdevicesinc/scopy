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

#include "advanced/jesdsettingswidget.h"
#include "adrv9009widgetfactory.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLabel>
#include <QLoggingCategory>
#include <style.h>
#include <iio.h>

Q_LOGGING_CATEGORY(CAT_JESDSETTINGS, "JesdSettings")

using namespace scopy;
using namespace scopy::adrv9009;

JesdSettingsWidget::JesdSettingsWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_JESDSETTINGS) << "No device provided to JESD Settings widget";
		return;
	}

	setupUi();

	// Apply established ADRV styling pattern (from adrv9002/initialcalibrationswidget.cpp:61)
	Style::setStyle(this, style::properties::widget::border_interactive);
}

JesdSettingsWidget::~JesdSettingsWidget() {}

void JesdSettingsWidget::setupUi()
{
	// Main layout for this widget
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// Create scroll area for all content
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create content widget for scroll area
	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	// Create main settings section using MenuSectionCollapseWidget
	MenuSectionCollapseWidget *settingsSection = new MenuSectionCollapseWidget(
		"JESD Settings", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);

	contentLayout->addWidget(settingsSection);

	// All widgets go into the settings section (13 attributes total)

	// SER AMPLITUDE - Range Widget [0 1 15]
	auto serAmplitude = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,jesd204-ser-amplitude", "[0 1 15]",
								     "SER AMPLITUDE");
	if(serAmplitude) {
		settingsSection->contentLayout()->addWidget(serAmplitude);
		connect(this, &JesdSettingsWidget::readRequested, serAmplitude, &IIOWidget::readAsync);
	}

	// SER PRE EMPHASIS - Range Widget [0 1 4]
	auto serPreEmphasis = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,jesd204-ser-pre-emphasis",
								       "[0 1 4]", "SER PRE EMPHASIS");
	if(serPreEmphasis) {
		settingsSection->contentLayout()->addWidget(serPreEmphasis);
		connect(this, &JesdSettingsWidget::readRequested, serPreEmphasis, &IIOWidget::readAsync);
	}

	// SERIALIZER INVERT POLARITY - Grouped lane checkboxes
	auto serializerLanes = createLaneCheckboxGroup("SERIALIZER INVERT POLARITY",
						       "adi,jesd204-ser-invert-lane-polarity", settingsSection);
	settingsSection->contentLayout()->addWidget(serializerLanes);

	// DESERIALIZER INVERT POLARITY - Grouped lane checkboxes
	auto deserializerLanes = createLaneCheckboxGroup("DESERIALIZER INVERT POLARITY",
							 "adi,jesd204-des-invert-lane-polarity", settingsSection);
	settingsSection->contentLayout()->addWidget(deserializerLanes);

	// DES EQ SETTING - Range Widget [0 1 4]
	auto desEqSetting = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,jesd204-des-eq-setting", "[0 1 4]",
								     "DES EQ SETTING");
	if(desEqSetting) {
		settingsSection->contentLayout()->addWidget(desEqSetting);
		connect(this, &JesdSettingsWidget::readRequested, desEqSetting, &IIOWidget::readAsync);
	}

	// SYSREF LVDS MODE - Checkbox
	auto sysrefLvdsMode = Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,jesd204-sysref-lvds-mode",
									  "SYSREF LVDS MODE");
	if(sysrefLvdsMode) {
		settingsSection->contentLayout()->addWidget(sysrefLvdsMode);
		connect(this, &JesdSettingsWidget::readRequested, sysrefLvdsMode, &IIOWidget::readAsync);
	}

	// SYSREF LVDS PN INVERT - Checkbox
	auto sysrefLvdsPnInvert = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,jesd204-sysref-lvds-pn-invert", "SYSREF LVDS PN INVERT");
	if(sysrefLvdsPnInvert) {
		settingsSection->contentLayout()->addWidget(sysrefLvdsPnInvert);
		connect(this, &JesdSettingsWidget::readRequested, sysrefLvdsPnInvert, &IIOWidget::readAsync);
	}

	// Add spacer to push content to top
	settingsSection->contentLayout()->addItem(
		new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Add spacer to main layout to push settings section to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_JESDSETTINGS) << "JESD Settings widget created with 13 widgets in flat layout";
}

QWidget *JesdSettingsWidget::createLaneCheckboxGroup(const QString &groupLabel, const QString &baseAttr,
						     QWidget *parent)
{
	// Create simple container without border styling
	QWidget *groupWidget = new QWidget(parent);

	// Create vertical layout for group
	QVBoxLayout *groupLayout = new QVBoxLayout(groupWidget);
	groupLayout->setContentsMargins(0, 5, 0, 10);
	groupLayout->setSpacing(5);

	// Group label with proper Scopy styling
	QLabel *label = new QLabel(groupLabel);
	Style::setStyle(label, style::properties::label::menuMedium);
	groupLayout->addWidget(label);

	// Create horizontal layout for lane checkboxes
	QWidget *lanesWidget = new QWidget();
	QHBoxLayout *lanesLayout = new QHBoxLayout(lanesWidget);
	lanesLayout->setContentsMargins(20, 0, 0, 0);
	lanesLayout->setSpacing(15);

	// Create switches directly
	scopy::MenuOnOffSwitch *lane0 = new scopy::MenuOnOffSwitch("LANE0", lanesWidget);
	scopy::MenuOnOffSwitch *lane1 = new scopy::MenuOnOffSwitch("LANE1", lanesWidget);
	scopy::MenuOnOffSwitch *lane2 = new scopy::MenuOnOffSwitch("LANE2", lanesWidget);
	scopy::MenuOnOffSwitch *lane3 = new scopy::MenuOnOffSwitch("LANE3", lanesWidget);

	lanesLayout->addWidget(lane0);
	lanesLayout->addWidget(lane1);
	lanesLayout->addWidget(lane2);
	lanesLayout->addWidget(lane3);

	// Function to update hardware from all switches
	auto updateHardware = [this, baseAttr, lane0, lane1, lane2, lane3]() {
		int bitmask = 0;
		if(lane0->onOffswitch()->isChecked())
			bitmask |= (1 << 0);
		if(lane1->onOffswitch()->isChecked())
			bitmask |= (1 << 1);
		if(lane2->onOffswitch()->isChecked())
			bitmask |= (1 << 2);
		if(lane3->onOffswitch()->isChecked())
			bitmask |= (1 << 3);
		QString value = QString::number(bitmask);
		iio_device_debug_attr_write(m_device, baseAttr.toUtf8().constData(), value.toUtf8().constData());
	};

	// Connect each switch to update function
	connect(lane0->onOffswitch(), &QAbstractButton::toggled, this, updateHardware);
	connect(lane1->onOffswitch(), &QAbstractButton::toggled, this, updateHardware);
	connect(lane2->onOffswitch(), &QAbstractButton::toggled, this, updateHardware);
	connect(lane3->onOffswitch(), &QAbstractButton::toggled, this, updateHardware);

	// Add stretch to push lanes to left
	lanesLayout->addStretch();
	groupLayout->addWidget(lanesWidget);

	// Function to read from hardware and set switches
	auto readFromHardware = [this, baseAttr, lane0, lane1, lane2, lane3]() {
		char value[16];
		int ret = iio_device_debug_attr_read(m_device, baseAttr.toUtf8().constData(), value, sizeof(value));
		if(ret < 0)
			return;
		int bitmask = QString(value).toInt();
		lane0->onOffswitch()->setChecked((bitmask & (1 << 0)) != 0);
		lane1->onOffswitch()->setChecked((bitmask & (1 << 1)) != 0);
		lane2->onOffswitch()->setChecked((bitmask & (1 << 2)) != 0);
		lane3->onOffswitch()->setChecked((bitmask & (1 << 3)) != 0);
	};

	// Read initial values and connect refresh
	readFromHardware();
	connect(this, &JesdSettingsWidget::readRequested, this, readFromHardware);

	return groupWidget;
}
