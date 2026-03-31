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
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>
#include <style.h>
#include <gui/widgets/menusectionwidget.h>
#include <iio.h>

Q_LOGGING_CATEGORY(CAT_AD9371_JESD_FRAMER, "AD9371_JESD_FRAMER")

using namespace scopy;
using namespace scopy::ad9371;

JesdFramerWidget::JesdFramerWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_JESD_FRAMER) << "No device provided to JESD Framer";
		return;
	}

	setupUi();

	Style::setStyle(this, style::properties::widget::border_interactive);
}

JesdFramerWidget::~JesdFramerWidget() {}

void JesdFramerWidget::setupUi()
{
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(true);

	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	// Create horizontal layout for RX Framer | OBS Framer columns
	QHBoxLayout *columnsLayout = new QHBoxLayout();
	columnsLayout->setSpacing(15);

	columnsLayout->addWidget(createFramerColumn("RX", "rx-framer", contentWidget));
	columnsLayout->addWidget(createFramerColumn("Observation RX", "obs-framer", contentWidget));

	contentLayout->addLayout(columnsLayout);
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_JESD_FRAMER) << "JESD Framer widget created with 48 attributes in RX|OBS column layout";
}

QWidget *JesdFramerWidget::createFramerColumn(const QString &columnTitle, const QString &attrPrefix, QWidget *parent)
{
	MenuSectionCollapseWidget *column = new MenuSectionCollapseWidget(columnTitle, MenuCollapseSection::MHCW_ARROW,
									  MenuCollapseSection::MHW_BASEWIDGET, parent);

	// 1. Bank ID - Range Widget [0,15,1]
	auto bankIdWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, QString("adi,jesd204-%1-bank-id").arg(attrPrefix), "[0 1 15]", "BANK ID");
	if(bankIdWidget) {
		if(m_widgetGroup) m_widgetGroup->add(bankIdWidget);
		column->contentLayout()->addWidget(bankIdWidget);
		connect(this, &JesdFramerWidget::readRequested, bankIdWidget, &IIOWidget::readAsync);
	}

	// 2. Device ID - Range Widget [0,255,1]
	auto deviceIdWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, QString("adi,jesd204-%1-device-id").arg(attrPrefix), "[0 1 255]", "DEVICE ID");
	if(deviceIdWidget) {
		if(m_widgetGroup) m_widgetGroup->add(deviceIdWidget);
		column->contentLayout()->addWidget(deviceIdWidget);
		connect(this, &JesdFramerWidget::readRequested, deviceIdWidget, &IIOWidget::readAsync);
	}

	// 3. Lane0 ID - Range Widget [0,31,1]
	auto lane0IdWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, QString("adi,jesd204-%1-lane0-id").arg(attrPrefix), "[0 1 31]", "LANE0 ID");
	if(lane0IdWidget) {
		if(m_widgetGroup) m_widgetGroup->add(lane0IdWidget);
		column->contentLayout()->addWidget(lane0IdWidget);
		connect(this, &JesdFramerWidget::readRequested, lane0IdWidget, &IIOWidget::readAsync);
	}

	// 4. M - Range Widget [0,255,1]
	auto mWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, QString("adi,jesd204-%1-m").arg(attrPrefix), "[0 1 255]", "M");
	if(mWidget) {
		if(m_widgetGroup) m_widgetGroup->add(mWidget);
		column->contentLayout()->addWidget(mWidget);
		connect(this, &JesdFramerWidget::readRequested, mWidget, &IIOWidget::readAsync);
	}

	// 5. K - Range Widget [0,32,1]
	auto kWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, QString("adi,jesd204-%1-k").arg(attrPrefix), "[0 1 32]", "K");
	if(kWidget) {
		if(m_widgetGroup) m_widgetGroup->add(kWidget);
		column->contentLayout()->addWidget(kWidget);
		connect(this, &JesdFramerWidget::readRequested, kWidget, &IIOWidget::readAsync);
	}

	// 6. Scramble - Checkbox
	auto scrambleWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, QString("adi,jesd204-%1-scramble").arg(attrPrefix), "SCRAMBLE");
	if(scrambleWidget) {
		if(m_widgetGroup) m_widgetGroup->add(scrambleWidget);
		column->contentLayout()->addWidget(scrambleWidget);
		scrambleWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdFramerWidget::readRequested, scrambleWidget, &IIOWidget::readAsync);
	}

	// 7. External SYSREF - Checkbox
	auto extSysrefWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, QString("adi,jesd204-%1-external-sysref").arg(attrPrefix), "EXTERNAL SYSREF");
	if(extSysrefWidget) {
		if(m_widgetGroup) m_widgetGroup->add(extSysrefWidget);
		column->contentLayout()->addWidget(extSysrefWidget);
		extSysrefWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdFramerWidget::readRequested, extSysrefWidget, &IIOWidget::readAsync);
	}

	// 8-11. Serializer Lanes Enabled - Bitmask switches (bits 0-3)
	QString serLanesAttr = QString("adi,jesd204-%1-serializer-lanes-enabled").arg(attrPrefix);

	scopy::MenuOnOffSwitch *serLane0 = new scopy::MenuOnOffSwitch("SERIALIZER LANE 0", column);
	scopy::MenuOnOffSwitch *serLane1 = new scopy::MenuOnOffSwitch("SERIALIZER LANE 1", column);
	scopy::MenuOnOffSwitch *serLane2 = new scopy::MenuOnOffSwitch("SERIALIZER LANE 2", column);
	scopy::MenuOnOffSwitch *serLane3 = new scopy::MenuOnOffSwitch("SERIALIZER LANE 3", column);

	column->contentLayout()->addWidget(serLane0);
	serLane0->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	column->contentLayout()->addWidget(serLane1);
	serLane1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	column->contentLayout()->addWidget(serLane2);
	serLane2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	column->contentLayout()->addWidget(serLane3);
	serLane3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	auto updateSerLanes = [this, serLanesAttr, serLane0, serLane1, serLane2, serLane3]() {
		int bitmask = 0;
		if(serLane0->onOffswitch()->isChecked())
			bitmask |= (1 << 0);
		if(serLane1->onOffswitch()->isChecked())
			bitmask |= (1 << 1);
		if(serLane2->onOffswitch()->isChecked())
			bitmask |= (1 << 2);
		if(serLane3->onOffswitch()->isChecked())
			bitmask |= (1 << 3);
		QString value = QString::number(bitmask);
		iio_device_debug_attr_write(m_device, serLanesAttr.toUtf8().constData(), value.toUtf8().constData());
	};

	connect(serLane0->onOffswitch(), &QAbstractButton::toggled, this, updateSerLanes);
	connect(serLane1->onOffswitch(), &QAbstractButton::toggled, this, updateSerLanes);
	connect(serLane2->onOffswitch(), &QAbstractButton::toggled, this, updateSerLanes);
	connect(serLane3->onOffswitch(), &QAbstractButton::toggled, this, updateSerLanes);

	auto readSerLanes = [this, serLanesAttr, serLane0, serLane1, serLane2, serLane3]() {
		char value[16];
		int ret = iio_device_debug_attr_read(m_device, serLanesAttr.toUtf8().constData(), value, sizeof(value));
		if(ret < 0)
			return;
		int bitmask = QString(value).toInt();
		serLane0->onOffswitch()->setChecked((bitmask & (1 << 0)) != 0);
		serLane1->onOffswitch()->setChecked((bitmask & (1 << 1)) != 0);
		serLane2->onOffswitch()->setChecked((bitmask & (1 << 2)) != 0);
		serLane3->onOffswitch()->setChecked((bitmask & (1 << 3)) != 0);
	};

	readSerLanes();
	connect(this, &JesdFramerWidget::readRequested, this, readSerLanes);

	// 12. Serializer Lane Crossbar - Range Widget [0,255,1]
	auto serCrossbarWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, QString("adi,jesd204-%1-serializer-lane-crossbar").arg(attrPrefix), "[0 1 255]",
		"SERIALIZER LANE CROSSBAR");
	if(serCrossbarWidget) {
		if(m_widgetGroup) m_widgetGroup->add(serCrossbarWidget);
		column->contentLayout()->addWidget(serCrossbarWidget);
		connect(this, &JesdFramerWidget::readRequested, serCrossbarWidget, &IIOWidget::readAsync);
	}

	// 13. Serializer Amplitude - Range Widget [0,15,1]
	auto serAmplitudeWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, QString("adi,jesd204-%1-serializer-amplitude").arg(attrPrefix), "[0 1 15]",
		"SERIALIZER AMPLITUDE");
	if(serAmplitudeWidget) {
		if(m_widgetGroup) m_widgetGroup->add(serAmplitudeWidget);
		column->contentLayout()->addWidget(serAmplitudeWidget);
		connect(this, &JesdFramerWidget::readRequested, serAmplitudeWidget, &IIOWidget::readAsync);
	}

	// 14. Pre-Emphasis - Range Widget [0,7,1]
	auto preEmphasisWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, QString("adi,jesd204-%1-pre-emphasis").arg(attrPrefix), "[0 1 7]", "PRE EMPHASIS");
	if(preEmphasisWidget) {
		if(m_widgetGroup) m_widgetGroup->add(preEmphasisWidget);
		column->contentLayout()->addWidget(preEmphasisWidget);
		connect(this, &JesdFramerWidget::readRequested, preEmphasisWidget, &IIOWidget::readAsync);
	}

	// 15-18. Invert Lane Polarity - Bitmask switches (bits 0-3)
	QString invertPolarityAttr = QString("adi,jesd204-%1-invert-lane-polarity").arg(attrPrefix);

	scopy::MenuOnOffSwitch *invLane0 = new scopy::MenuOnOffSwitch("INVERT LANE 0 POLARITY", column);
	scopy::MenuOnOffSwitch *invLane1 = new scopy::MenuOnOffSwitch("INVERT LANE 1 POLARITY", column);
	scopy::MenuOnOffSwitch *invLane2 = new scopy::MenuOnOffSwitch("INVERT LANE 2 POLARITY", column);
	scopy::MenuOnOffSwitch *invLane3 = new scopy::MenuOnOffSwitch("INVERT LANE 3 POLARITY", column);

	column->contentLayout()->addWidget(invLane0);
	invLane0->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	column->contentLayout()->addWidget(invLane1);
	invLane1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	column->contentLayout()->addWidget(invLane2);
	invLane2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	column->contentLayout()->addWidget(invLane3);
	invLane3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	auto updateInvPolarity = [this, invertPolarityAttr, invLane0, invLane1, invLane2, invLane3]() {
		int bitmask = 0;
		if(invLane0->onOffswitch()->isChecked())
			bitmask |= (1 << 0);
		if(invLane1->onOffswitch()->isChecked())
			bitmask |= (1 << 1);
		if(invLane2->onOffswitch()->isChecked())
			bitmask |= (1 << 2);
		if(invLane3->onOffswitch()->isChecked())
			bitmask |= (1 << 3);
		QString value = QString::number(bitmask);
		iio_device_debug_attr_write(m_device, invertPolarityAttr.toUtf8().constData(),
					    value.toUtf8().constData());
	};

	connect(invLane0->onOffswitch(), &QAbstractButton::toggled, this, updateInvPolarity);
	connect(invLane1->onOffswitch(), &QAbstractButton::toggled, this, updateInvPolarity);
	connect(invLane2->onOffswitch(), &QAbstractButton::toggled, this, updateInvPolarity);
	connect(invLane3->onOffswitch(), &QAbstractButton::toggled, this, updateInvPolarity);

	auto readInvPolarity = [this, invertPolarityAttr, invLane0, invLane1, invLane2, invLane3]() {
		char value[16];
		int ret = iio_device_debug_attr_read(m_device, invertPolarityAttr.toUtf8().constData(), value,
						     sizeof(value));
		if(ret < 0)
			return;
		int bitmask = QString(value).toInt();
		invLane0->onOffswitch()->setChecked((bitmask & (1 << 0)) != 0);
		invLane1->onOffswitch()->setChecked((bitmask & (1 << 1)) != 0);
		invLane2->onOffswitch()->setChecked((bitmask & (1 << 2)) != 0);
		invLane3->onOffswitch()->setChecked((bitmask & (1 << 3)) != 0);
	};

	readInvPolarity();
	connect(this, &JesdFramerWidget::readRequested, this, readInvPolarity);

	// 19. LMFC Offset - Range Widget [0,31,1]
	auto lmfcWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, QString("adi,jesd204-%1-lmfc-offset").arg(attrPrefix), "[0 1 31]", "LMFC OFFSET");
	if(lmfcWidget) {
		if(m_widgetGroup) m_widgetGroup->add(lmfcWidget);
		column->contentLayout()->addWidget(lmfcWidget);
		connect(this, &JesdFramerWidget::readRequested, lmfcWidget, &IIOWidget::readAsync);
	}

	// 20. New SYSREF on Relink - Checkbox
	auto newSysrefWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, QString("adi,jesd204-%1-new-sysref-on-relink").arg(attrPrefix), "NEW SYSREF ON RELINK");
	if(newSysrefWidget) {
		if(m_widgetGroup) m_widgetGroup->add(newSysrefWidget);
		column->contentLayout()->addWidget(newSysrefWidget);
		newSysrefWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdFramerWidget::readRequested, newSysrefWidget, &IIOWidget::readAsync);
	}

	// 21. Enable Auto Chan XBAR - Checkbox
	auto enableAutoXbarWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, QString("adi,jesd204-%1-enable-auto-chan-xbar").arg(attrPrefix), "ENABLE AUTO CHAN XBAR");
	if(enableAutoXbarWidget) {
		if(m_widgetGroup) m_widgetGroup->add(enableAutoXbarWidget);
		column->contentLayout()->addWidget(enableAutoXbarWidget);
		enableAutoXbarWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdFramerWidget::readRequested, enableAutoXbarWidget, &IIOWidget::readAsync);
	}

	// 22. OBS RX SYNCB Select - Checkbox
	auto obsRxSyncbWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, QString("adi,jesd204-%1-obs-rx-syncb-select").arg(attrPrefix), "OBS RX SYNCB SELECT");
	if(obsRxSyncbWidget) {
		if(m_widgetGroup) m_widgetGroup->add(obsRxSyncbWidget);
		column->contentLayout()->addWidget(obsRxSyncbWidget);
		obsRxSyncbWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdFramerWidget::readRequested, obsRxSyncbWidget, &IIOWidget::readAsync);
	}

	// 23. RX SYNCB Mode - Checkbox
	auto rxSyncbModeWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, QString("adi,jesd204-%1-rx-syncb-mode").arg(attrPrefix), "RX SYNCB MODE");
	if(rxSyncbModeWidget) {
		if(m_widgetGroup) m_widgetGroup->add(rxSyncbModeWidget);
		column->contentLayout()->addWidget(rxSyncbModeWidget);
		rxSyncbModeWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdFramerWidget::readRequested, rxSyncbModeWidget, &IIOWidget::readAsync);
	}

	// 24. Over Sample - Checkbox
	auto overSampleWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, QString("adi,jesd204-%1-over-sample").arg(attrPrefix), "OVER SAMPLE");
	if(overSampleWidget) {
		if(m_widgetGroup) m_widgetGroup->add(overSampleWidget);
		column->contentLayout()->addWidget(overSampleWidget);
		overSampleWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdFramerWidget::readRequested, overSampleWidget, &IIOWidget::readAsync);
	}

	// Add spacer to push content to top
	column->contentLayout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	return column;
}
