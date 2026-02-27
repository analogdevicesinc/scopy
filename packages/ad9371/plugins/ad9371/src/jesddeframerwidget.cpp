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

#include "jesddeframerwidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

JesdDeframerWidget::JesdDeframerWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
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

	QLabel *title = new QLabel("JESD Deframer (TX)", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	QList<IIOWidget *> allWidgets;

	auto addRange = [&](const char *attr, const QString &title) {
		IIOWidget *w = makeDebugRange(widget, m_device, attr, title, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};

	addRange("adi,jesd204-deframer-bank-id", "Bank ID");
	addRange("adi,jesd204-deframer-device-id", "Device ID");
	addRange("adi,jesd204-deframer-lane0-id", "Lane0 ID");
	addRange("adi,jesd204-deframer-m", "M");
	addRange("adi,jesd204-deframer-k", "K");
	addRange("adi,jesd204-deframer-scramble", "Scramble");
	addRange("adi,jesd204-deframer-external-sysref", "External Sysref");
	addRange("adi,jesd204-deframer-deserializer-lane-crossbar", "Deserializer Lane Crossbar");
	addRange("adi,jesd204-deframer-eq-setting", "EQ Setting");
	addRange("adi,jesd204-deframer-lmfc-offset", "LMFC Offset");
	addRange("adi,jesd204-deframer-new-sysref-on-relink", "New Sysref On Relink");
	addRange("adi,jesd204-deframer-enable-auto-chan-xbar", "Enable Auto Chan Xbar");

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &JesdDeframerWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

JesdDeframerWidget::~JesdDeframerWidget() {}
