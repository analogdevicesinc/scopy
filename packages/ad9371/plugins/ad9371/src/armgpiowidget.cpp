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

#include "armgpiowidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

ArmGpioWidget::ArmGpioWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: m_device(device)
	, m_group(group)
	, QWidget{parent}
{
	Style::setBackgroundColor(this, json::theme::background_primary);
	m_layout = new QVBoxLayout(this);
	m_layout->setMargin(0);
	m_layout->setContentsMargins(0, 0, 0, 0);

	QWidget *widget = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(5, 5, 5, 5);
	layout->setSpacing(10);
	Style::setStyle(widget, style::properties::widget::border_interactive);
	m_layout->addWidget(widget);

	QList<IIOWidget *> allWidgets;

	auto addRange = [&](const char *attr, const QString &title) {
		IIOWidget *w = makeDebugRange(widget, m_device, attr, title, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};
	auto addCheck = [&](const char *attr, const QString &title) {
		IIOWidget *w = makeDebugCheck(widget, m_device, attr, title, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};

	QLabel *modeTitle = new QLabel("Mode", widget);
	Style::setStyle(modeTitle, style::properties::label::menuBig);
	layout->addWidget(modeTitle);

	addCheck("adi,arm-gpio-config-tx-rx-pin-mode", "TX/RX Pin Mode");
	addCheck("adi,arm-gpio-config-use-tx2-enable-pin", "Use TX2 Enable Pin");
	addCheck("adi,arm-gpio-config-use-rx2-enable-pin", "Use RX2 Enable Pin");

	QLabel *orxTitle = new QLabel("ORX", widget);
	Style::setStyle(orxTitle, style::properties::label::menuBig);
	layout->addWidget(orxTitle);

	addCheck("adi,arm-gpio-config-orx-pin-mode", "ORX Pin Mode");
	addRange("adi,arm-gpio-config-orx-trigger-pin", "ORX Trigger Pin");
	addRange("adi,arm-gpio-config-orx-mode2-pin", "ORX Mode2 Pin");
	addRange("adi,arm-gpio-config-orx-mode1-pin", "ORX Mode1 Pin");
	addRange("adi,arm-gpio-config-orx-mode0-pin", "ORX Mode0 Pin");

	QLabel *ackTitle = new QLabel("ACK", widget);
	Style::setStyle(ackTitle, style::properties::label::menuBig);
	layout->addWidget(ackTitle);

	const char *ackAttrs[] = {"rx1", "rx2", "tx1", "tx2", "orx1", "orx2", "srx"};
	const QString ackNames[] = {"RX1", "RX2", "TX1", "TX2", "ORX1", "ORX2", "SRX"};
	for(int i = 0; i < 7; i++) {
		QString enableAckAttr = QString("adi,arm-gpio-config-%1-enable-ack").arg(ackAttrs[i]);
		addRange(enableAckAttr.toUtf8().constData(), ackNames[i] + " Enable ACK");
	}

	addRange("adi,arm-gpio-config-tx-obs-select", "TX OBS Select");

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &ArmGpioWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

ArmGpioWidget::~ArmGpioWidget() {}
