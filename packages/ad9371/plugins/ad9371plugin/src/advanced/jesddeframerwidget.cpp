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

#include "advanced/jesddeframerwidget.h"
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QSpacerItem>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>
#include <style.h>
#include <iio.h>

Q_LOGGING_CATEGORY(CAT_AD9371_JESD_DEFRAMER, "AD9371_JESD_DEFRAMER")

using namespace scopy;
using namespace scopy::ad9371;

JesdDeframerWidget::JesdDeframerWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_JESD_DEFRAMER) << "No device provided to JESD Deframer";
		return;
	}

	setupUi();

	Style::setStyle(this, style::properties::widget::border_interactive);
}

JesdDeframerWidget::~JesdDeframerWidget() {}

void JesdDeframerWidget::setupUi()
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

	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"Deframer", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, contentWidget);

	// 1. Bank ID - Range Widget [0,15,1]
	auto bankIdWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,jesd204-deframer-bank-id",
									"[0 1 15]", "BANK ID");
	if(bankIdWidget) {
		if(m_widgetGroup) m_widgetGroup->add(bankIdWidget);
		section->contentLayout()->addWidget(bankIdWidget);
		connect(this, &JesdDeframerWidget::readRequested, bankIdWidget, &IIOWidget::readAsync);
	}

	// 2. Device ID - Range Widget [0,255,1]
	auto deviceIdWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,jesd204-deframer-device-id",
									  "[0 1 255]", "DEVICE ID");
	if(deviceIdWidget) {
		if(m_widgetGroup) m_widgetGroup->add(deviceIdWidget);
		section->contentLayout()->addWidget(deviceIdWidget);
		connect(this, &JesdDeframerWidget::readRequested, deviceIdWidget, &IIOWidget::readAsync);
	}

	// 3. Lane0 ID - Range Widget [0,31,1]
	auto lane0IdWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,jesd204-deframer-lane0-id",
									 "[0 1 31]", "LANE0 ID");
	if(lane0IdWidget) {
		if(m_widgetGroup) m_widgetGroup->add(lane0IdWidget);
		section->contentLayout()->addWidget(lane0IdWidget);
		connect(this, &JesdDeframerWidget::readRequested, lane0IdWidget, &IIOWidget::readAsync);
	}

	// 4. M (Converters) - Range Widget [0,255,1]
	auto mWidget =
		Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,jesd204-deframer-m", "[0 1 255]", "M");
	if(mWidget) {
		if(m_widgetGroup) m_widgetGroup->add(mWidget);
		section->contentLayout()->addWidget(mWidget);
		connect(this, &JesdDeframerWidget::readRequested, mWidget, &IIOWidget::readAsync);
	}

	// 5. K (Frames/Multiframe) - Range Widget [0,32,1]
	auto kWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,jesd204-deframer-k", "[0 1 32]", "K");
	if(kWidget) {
		if(m_widgetGroup) m_widgetGroup->add(kWidget);
		section->contentLayout()->addWidget(kWidget);
		connect(this, &JesdDeframerWidget::readRequested, kWidget, &IIOWidget::readAsync);
	}

	// 6. Scramble - Checkbox
	auto scrambleWidget =
		Ad9371WidgetFactory::createDebugCheckboxWidget(m_device, "adi,jesd204-deframer-scramble", "SCRAMBLE");
	if(scrambleWidget) {
		if(m_widgetGroup) m_widgetGroup->add(scrambleWidget);
		section->contentLayout()->addWidget(scrambleWidget);
		scrambleWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdDeframerWidget::readRequested, scrambleWidget, &IIOWidget::readAsync);
	}

	// 7. External SYSREF - Checkbox
	auto extSysrefWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,jesd204-deframer-external-sysref", "EXTERNAL SYSREF");
	if(extSysrefWidget) {
		if(m_widgetGroup) m_widgetGroup->add(extSysrefWidget);
		section->contentLayout()->addWidget(extSysrefWidget);
		extSysrefWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdDeframerWidget::readRequested, extSysrefWidget, &IIOWidget::readAsync);
	}

	// 8-11. Deserializer Lanes Enabled - Bitmask switches (bits 0-3)
	QString desLanesAttr = "adi,jesd204-deframer-deserializer-lanes-enabled";

	scopy::MenuOnOffSwitch *desLane0 = new scopy::MenuOnOffSwitch("DESERIALIZER LANE 0", section);
	scopy::MenuOnOffSwitch *desLane1 = new scopy::MenuOnOffSwitch("DESERIALIZER LANE 1", section);
	scopy::MenuOnOffSwitch *desLane2 = new scopy::MenuOnOffSwitch("DESERIALIZER LANE 2", section);
	scopy::MenuOnOffSwitch *desLane3 = new scopy::MenuOnOffSwitch("DESERIALIZER LANE 3", section);

	section->contentLayout()->addWidget(desLane0);
	desLane0->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	section->contentLayout()->addWidget(desLane1);
	desLane1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	section->contentLayout()->addWidget(desLane2);
	desLane2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	section->contentLayout()->addWidget(desLane3);
	desLane3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);

	auto updateDesLanes = [this, desLanesAttr, desLane0, desLane1, desLane2, desLane3]() {
		int bitmask = 0;
		if(desLane0->onOffswitch()->isChecked())
			bitmask |= (1 << 0);
		if(desLane1->onOffswitch()->isChecked())
			bitmask |= (1 << 1);
		if(desLane2->onOffswitch()->isChecked())
			bitmask |= (1 << 2);
		if(desLane3->onOffswitch()->isChecked())
			bitmask |= (1 << 3);
		QString value = QString::number(bitmask);
		iio_device_debug_attr_write(m_device, desLanesAttr.toUtf8().constData(), value.toUtf8().constData());
	};

	connect(desLane0->onOffswitch(), &QAbstractButton::toggled, this, updateDesLanes);
	connect(desLane1->onOffswitch(), &QAbstractButton::toggled, this, updateDesLanes);
	connect(desLane2->onOffswitch(), &QAbstractButton::toggled, this, updateDesLanes);
	connect(desLane3->onOffswitch(), &QAbstractButton::toggled, this, updateDesLanes);

	auto readDesLanes = [this, desLanesAttr, desLane0, desLane1, desLane2, desLane3]() {
		char value[16];
		int ret = iio_device_debug_attr_read(m_device, desLanesAttr.toUtf8().constData(), value, sizeof(value));
		if(ret < 0)
			return;
		int bitmask = QString(value).toInt();
		desLane0->onOffswitch()->setChecked((bitmask & (1 << 0)) != 0);
		desLane1->onOffswitch()->setChecked((bitmask & (1 << 1)) != 0);
		desLane2->onOffswitch()->setChecked((bitmask & (1 << 2)) != 0);
		desLane3->onOffswitch()->setChecked((bitmask & (1 << 3)) != 0);
	};

	readDesLanes();
	connect(this, &JesdDeframerWidget::readRequested, this, readDesLanes);

	// 12. Lane Crossbar - Range Widget [0,255,1]
	auto crossbarWidget = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,jesd204-deframer-deserializer-lane-crossbar", "[0 1 255]", "DESERIALIZER LANE CROSSBAR");
	if(crossbarWidget) {
		if(m_widgetGroup) m_widgetGroup->add(crossbarWidget);
		section->contentLayout()->addWidget(crossbarWidget);
		connect(this, &JesdDeframerWidget::readRequested, crossbarWidget, &IIOWidget::readAsync);
	}

	// 13. EQ Setting - Range Widget [0,3,1]
	auto eqSettingWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,jesd204-deframer-eq-setting",
									   "[0 1 3]", "EQ SETTING");
	if(eqSettingWidget) {
		if(m_widgetGroup) m_widgetGroup->add(eqSettingWidget);
		section->contentLayout()->addWidget(eqSettingWidget);
		connect(this, &JesdDeframerWidget::readRequested, eqSettingWidget, &IIOWidget::readAsync);
	}

	// 14-17. Invert Lane Polarity - Bitmask switches (bits 0-3)
	QString invertPolarityAttr = "adi,jesd204-deframer-invert-lane-polarity";

	scopy::MenuOnOffSwitch *invLane0 = new scopy::MenuOnOffSwitch("INVERT LANE 0 POLARITY", section);
	scopy::MenuOnOffSwitch *invLane1 = new scopy::MenuOnOffSwitch("INVERT LANE 1 POLARITY", section);
	scopy::MenuOnOffSwitch *invLane2 = new scopy::MenuOnOffSwitch("INVERT LANE 2 POLARITY", section);
	scopy::MenuOnOffSwitch *invLane3 = new scopy::MenuOnOffSwitch("INVERT LANE 3 POLARITY", section);

	section->contentLayout()->addWidget(invLane0);
	invLane0->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	section->contentLayout()->addWidget(invLane1);
	invLane1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	section->contentLayout()->addWidget(invLane2);
	invLane2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	section->contentLayout()->addWidget(invLane3);
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
	connect(this, &JesdDeframerWidget::readRequested, this, readInvPolarity);

	// 18. LMFC Offset - Range Widget [0,31,1]
	auto lmfcWidget = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,jesd204-deframer-lmfc-offset",
								      "[0 1 31]", "LMFC OFFSET");
	if(lmfcWidget) {
		if(m_widgetGroup) m_widgetGroup->add(lmfcWidget);
		section->contentLayout()->addWidget(lmfcWidget);
		connect(this, &JesdDeframerWidget::readRequested, lmfcWidget, &IIOWidget::readAsync);
	}

	// 19. New SYSREF on Relink - Checkbox
	auto newSysrefWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,jesd204-deframer-new-sysref-on-relink", "NEW SYSREF ON RELINK");
	if(newSysrefWidget) {
		if(m_widgetGroup) m_widgetGroup->add(newSysrefWidget);
		section->contentLayout()->addWidget(newSysrefWidget);
		newSysrefWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdDeframerWidget::readRequested, newSysrefWidget, &IIOWidget::readAsync);
	}

	// 20. Enable Auto Channel Crossbar - Checkbox
	auto enableAutoXbarWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,jesd204-deframer-enable-auto-chan-xbar", "ENABLE AUTO CHAN XBAR");
	if(enableAutoXbarWidget) {
		if(m_widgetGroup) m_widgetGroup->add(enableAutoXbarWidget);
		section->contentLayout()->addWidget(enableAutoXbarWidget);
		enableAutoXbarWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdDeframerWidget::readRequested, enableAutoXbarWidget, &IIOWidget::readAsync);
	}

	// 21. TX SYNCB Mode - Checkbox
	auto txSyncbModeWidget = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,jesd204-deframer-tx-syncb-mode", "TX SYNCB MODE");
	if(txSyncbModeWidget) {
		if(m_widgetGroup) m_widgetGroup->add(txSyncbModeWidget);
		section->contentLayout()->addWidget(txSyncbModeWidget);
		txSyncbModeWidget->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &JesdDeframerWidget::readRequested, txSyncbModeWidget, &IIOWidget::readAsync);
	}

	// Add spacer to push content to top
	section->contentLayout()->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	contentLayout->addWidget(section);
	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AD9371_JESD_DEFRAMER) << "JESD Deframer widget created with 21 attributes";
}
