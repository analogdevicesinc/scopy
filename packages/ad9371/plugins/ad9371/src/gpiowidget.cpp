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

#include "gpiowidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

GpioWidget::GpioWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
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

	QLabel *gpio3v3Title = new QLabel("GPIO 3.3V", widget);
	Style::setStyle(gpio3v3Title, style::properties::label::menuBig);
	layout->addWidget(gpio3v3Title);

	// GPIO 3.3V groups: [0..3], [4..7], [8..11]
	const int gpio3v3Groups[][2] = {{0, 3}, {4, 7}, {8, 11}};
	for(const auto &grp : gpio3v3Groups) {
		QString dirAttr = QString("adi,gpio-3v3-%1-%2-oe-mask").arg(grp[0]).arg(grp[1]);
		addRange(dirAttr.toUtf8().constData(), QString("GPIO 3.3V [%1..%2] OE Mask").arg(grp[0]).arg(grp[1]));
		for(int pin = grp[0]; pin <= grp[1]; pin++) {
			QString enableAttr = QString("adi,gpio-3v3-%1-level").arg(pin);
			addRange(enableAttr.toUtf8().constData(), QString("GPIO 3.3V %1 Level").arg(pin));
		}
	}

	QLabel *gpioLvTitle = new QLabel("GPIO Low Voltage", widget);
	Style::setStyle(gpioLvTitle, style::properties::label::menuBig);
	layout->addWidget(gpioLvTitle);

	// GPIO LV groups: [0..3], [4..7], [8..11], [12..15], [16..18]
	const int gpioLvGroups[][2] = {{0, 3}, {4, 7}, {8, 11}, {12, 15}, {16, 18}};
	for(const auto &grp : gpioLvGroups) {
		QString dirAttr = QString("adi,gpio-%1-%2-oe-mask").arg(grp[0]).arg(grp[1]);
		addRange(dirAttr.toUtf8().constData(), QString("GPIO [%1..%2] OE Mask").arg(grp[0]).arg(grp[1]));
		for(int pin = grp[0]; pin <= grp[1]; pin++) {
			QString enableAttr = QString("adi,gpio-%1-level").arg(pin);
			addRange(enableAttr.toUtf8().constData(), QString("GPIO %1 Level").arg(pin));
		}
	}

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &GpioWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

GpioWidget::~GpioWidget() {}
