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

#include "advanced/jesdframerwidget.h"
#include "adrv9009widgetfactory.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>
#include <style.h>
#include <gui/widgets/menusectionwidget.h>
#include <iio.h>

Q_LOGGING_CATEGORY(CAT_JESDFRAMER, "JesdFramer")

using namespace scopy;
using namespace scopy::adrv9009;

JesdFramerWidget::JesdFramerWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_JESDFRAMER) << "No device provided to JESD Framer widget";
		return;
	}

	setupUi();

	// Apply established ADRV styling pattern (from adrv9002/initialcalibrationswidget.cpp:61)
	Style::setStyle(this, style::properties::widget::border_interactive);
}

JesdFramerWidget::~JesdFramerWidget() {}

void JesdFramerWidget::setupUi()
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
	// Use VBoxLayout as main container to allow proper vertical spacing
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	// Create horizontal layout for A|B columns
	QHBoxLayout *columnsLayout = new QHBoxLayout();
	columnsLayout->setSpacing(15);

	// Column A and B side-by-side (50/50 split)
	columnsLayout->addWidget(createFramerColumn("A", "framer-a", contentWidget));
	columnsLayout->addWidget(createFramerColumn("B", "framer-b", contentWidget));

	// Add columns layout to main layout
	contentLayout->addLayout(columnsLayout);

	// Add spacer to push content to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_JESDFRAMER) << "JESD Framer widget created with 39 attributes in A|B column layout";
}

QWidget *JesdFramerWidget::createFramerColumn(const QString &columnType, const QString &attrPrefix, QWidget *parent)
{
	// JESD Framer column using MenuSectionCollapseWidget (22 attributes)
	MenuSectionCollapseWidget *column = new MenuSectionCollapseWidget(QString("JESD Framer %1").arg(columnType),
									  MenuCollapseSection::MHCW_ARROW,
									  MenuCollapseSection::MHW_BASEWIDGET, parent);

	// 1. BANK ID - Range Widget [0 1 15]
	auto bankIdWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, QString("adi,jesd204-%1-bank-id").arg(attrPrefix), "[0 1 15]", "Bank ID");
	if(bankIdWidget) {
		column->contentLayout()->addWidget(bankIdWidget);
		connect(this, &JesdFramerWidget::readRequested, bankIdWidget, &IIOWidget::readAsync);
	}

	// 2. DEVICE ID - Range Widget [0 1 255]
	auto deviceIdWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, QString("adi,jesd204-%1-device-id").arg(attrPrefix), "[0 1 255]", "Device ID");
	if(deviceIdWidget) {
		column->contentLayout()->addWidget(deviceIdWidget);
		connect(this, &JesdFramerWidget::readRequested, deviceIdWidget, &IIOWidget::readAsync);
	}

	// 3. LANE0 ID - Range Widget [0 1 31]
	auto lane0IdWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, QString("adi,jesd204-%1-lane0-id").arg(attrPrefix), "[0 1 31]", "Lane0 ID");
	if(lane0IdWidget) {
		column->contentLayout()->addWidget(lane0IdWidget);
		connect(this, &JesdFramerWidget::readRequested, lane0IdWidget, &IIOWidget::readAsync);
	}

	// 4. M - Combobox [0,2,4]
	QMap<QString, QString> *mOptions = new QMap<QString, QString>();
	mOptions->insert("0", "0");
	mOptions->insert("2", "2");
	mOptions->insert("4", "4");
	auto mWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, QString("adi,jesd204-%1-m").arg(attrPrefix), mOptions, "M");
	if(mWidget) {
		column->contentLayout()->addWidget(mWidget);
		connect(this, &JesdFramerWidget::readRequested, mWidget, &IIOWidget::readAsync);
		mWidget->getUiStrategy()->setInfoMessage(
			"Number of ADCs (0, 2, or 4) where 2 ADCs are required per receive chain (I and Q)");
	}

	// 5. K - Range Widget [1 1 32]
	auto kWidget = Adrv9009WidgetFactory::createRangeWidget(m_device, QString("adi,jesd204-%1-k").arg(attrPrefix),
								"[1 1 32]", "K");
	if(kWidget) {
		column->contentLayout()->addWidget(kWidget);
		connect(this, &JesdFramerWidget::readRequested, kWidget, &IIOWidget::readAsync);
	}

	// 6. F - Combobox [1,2,3,4,6,8]
	QMap<QString, QString> *fOptions = new QMap<QString, QString>();
	fOptions->insert("1", "1");
	fOptions->insert("2", "2");
	fOptions->insert("3", "3");
	fOptions->insert("4", "4");
	fOptions->insert("6", "6");
	fOptions->insert("8", "8");
	auto fWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, QString("adi,jesd204-%1-f").arg(attrPrefix), fOptions, "F");
	if(fWidget) {
		column->contentLayout()->addWidget(fWidget);
		connect(this, &JesdFramerWidget::readRequested, fWidget, &IIOWidget::readAsync);
		fWidget->getUiStrategy()->setInfoMessage("Number of bytes(octets) per frame (Valid 1, 2, 4, 8)");
	}

	// 7. NP - Combobox [12,16,24]
	QMap<QString, QString> *npOptions = new QMap<QString, QString>();
	npOptions->insert("12", "12");
	npOptions->insert("16", "16");
	npOptions->insert("24", "24");
	auto npWidget = Adrv9009WidgetFactory::createCustomComboWidget(
		m_device, QString("adi,jesd204-%1-np").arg(attrPrefix), npOptions, "NP");
	if(npWidget) {
		column->contentLayout()->addWidget(npWidget);
		connect(this, &JesdFramerWidget::readRequested, npWidget, &IIOWidget::readAsync);
		npWidget->getUiStrategy()->setInfoMessage("converter sample resolution (12, 16, 24)");
	}

	// 8. SCRAMBLE - Checkbox
	auto scrambleWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, QString("adi,jesd204-%1-scramble").arg(attrPrefix), "Scramble");
	if(scrambleWidget) {
		column->contentLayout()->addWidget(scrambleWidget);
		connect(this, &JesdFramerWidget::readRequested, scrambleWidget, &IIOWidget::readAsync);
	}

	// 9. EXTERNAL SYSREF - Checkbox
	auto extSysrefWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, QString("adi,jesd204-%1-external-sysref").arg(attrPrefix), "External SYSREF");
	if(extSysrefWidget) {
		column->contentLayout()->addWidget(extSysrefWidget);
		connect(this, &JesdFramerWidget::readRequested, extSysrefWidget, &IIOWidget::readAsync);
	}

	// 10-13. SERIALIZER LANES ENABLED - Bitmask switches
	QString baseAttr = QString("adi,jesd204-%1-serializer-lanes-enabled").arg(attrPrefix);

	// Create switches directly
	scopy::MenuOnOffSwitch *lane0 = new scopy::MenuOnOffSwitch("Serializer Lane 0", column);
	scopy::MenuOnOffSwitch *lane1 = new scopy::MenuOnOffSwitch("Serializer Lane 1", column);
	scopy::MenuOnOffSwitch *lane2 = new scopy::MenuOnOffSwitch("Serializer Lane 2", column);
	scopy::MenuOnOffSwitch *lane3 = new scopy::MenuOnOffSwitch("Serializer Lane 3", column);

	column->contentLayout()->addWidget(lane0);
	column->contentLayout()->addWidget(lane1);
	column->contentLayout()->addWidget(lane2);
	column->contentLayout()->addWidget(lane3);

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
	connect(this, &JesdFramerWidget::readRequested, this, readFromHardware);

	// 14. SERIALIZER LANE CROSSBAR - Range Widget [0 1 65535]
	auto serCrossbarWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, QString("adi,jesd204-%1-serializer-lane-crossbar").arg(attrPrefix), "[0 1 65535]",
		"Serializer Lane Crossbar");
	if(serCrossbarWidget) {
		column->contentLayout()->addWidget(serCrossbarWidget);
		connect(this, &JesdFramerWidget::readRequested, serCrossbarWidget, &IIOWidget::readAsync);
	}

	// 15. LMFC OFFSET - Range Widget [0 1 31]
	auto lmfcWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, QString("adi,jesd204-%1-lmfc-offset").arg(attrPrefix), "[0 1 31]", "LMFC Offset");
	if(lmfcWidget) {
		column->contentLayout()->addWidget(lmfcWidget);
		connect(this, &JesdFramerWidget::readRequested, lmfcWidget, &IIOWidget::readAsync);
	}

	// 16. NEW SYSREF ON RELINK - Checkbox
	auto newSysrefWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, QString("adi,jesd204-%1-new-sysref-on-relink").arg(attrPrefix), "New SYSREF on Relink");
	if(newSysrefWidget) {
		column->contentLayout()->addWidget(newSysrefWidget);
		connect(this, &JesdFramerWidget::readRequested, newSysrefWidget, &IIOWidget::readAsync);
	}

	// 17. SYNCB IN SELECT - Checkbox
	auto syncbInWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, QString("adi,jesd204-%1-syncb-in-select").arg(attrPrefix), "SYNCB In Select");
	if(syncbInWidget) {
		column->contentLayout()->addWidget(syncbInWidget);
		connect(this, &JesdFramerWidget::readRequested, syncbInWidget, &IIOWidget::readAsync);
	}

	// 18. OVER SAMPLE - Checkbox
	auto overSampleWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, QString("adi,jesd204-%1-over-sample").arg(attrPrefix), "Over Sample");
	if(overSampleWidget) {
		column->contentLayout()->addWidget(overSampleWidget);
		connect(this, &JesdFramerWidget::readRequested, overSampleWidget, &IIOWidget::readAsync);
	}

	// 19. SYNCB IN LVDS MODE - Checkbox
	auto syncbLvdsModeWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, QString("adi,jesd204-%1-syncb-in-lvds-mode").arg(attrPrefix), "SYNCB In LVDS Mode");
	if(syncbLvdsModeWidget) {
		column->contentLayout()->addWidget(syncbLvdsModeWidget);
		connect(this, &JesdFramerWidget::readRequested, syncbLvdsModeWidget, &IIOWidget::readAsync);
	}

	// 20. SYNCB IN LVDS PN INVERT - Checkbox
	auto syncbLvdsPnWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, QString("adi,jesd204-%1-syncb-in-lvds-pn-invert").arg(attrPrefix), "SYNCB In LVDS PN Invert");
	if(syncbLvdsPnWidget) {
		column->contentLayout()->addWidget(syncbLvdsPnWidget);
		connect(this, &JesdFramerWidget::readRequested, syncbLvdsPnWidget, &IIOWidget::readAsync);
	}

	// 21. ENABLE MANUAL LANE XBAR - Checkbox
	auto enableManualXbarWidget = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, QString("adi,jesd204-%1-enable-manual-lane-xbar").arg(attrPrefix), "Enable Manual Lane XBAR");
	if(enableManualXbarWidget) {
		column->contentLayout()->addWidget(enableManualXbarWidget);
		connect(this, &JesdFramerWidget::readRequested, enableManualXbarWidget, &IIOWidget::readAsync);
	}

	// Add spacer to push content to top
	column->contentLayout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	return column;
}
