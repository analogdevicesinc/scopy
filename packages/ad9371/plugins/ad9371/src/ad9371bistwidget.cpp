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

#include "ad9371bistwidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

Ad9371BistWidget::Ad9371BistWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
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

	QLabel *title = new QLabel("BIST", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	QList<IIOWidget *> allWidgets;

	auto addRange = [&](const char *attr, const QString &t) {
		IIOWidget *w = makeDebugRange(widget, m_device, attr, t, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};
	auto addCheck = [&](const char *attr, const QString &t) {
		IIOWidget *w = makeDebugCheck(widget, m_device, attr, t, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};

	QLabel *ncoTitle = new QLabel("TX NCO", widget);
	Style::setStyle(ncoTitle, style::properties::label::menuBig);
	layout->addWidget(ncoTitle);

	addCheck("adi,bist-tone-enable", "TX NCO Enable");
	addRange("adi,bist-tone-tx1-mhz", "TX1 Tone (MHz)");
	addRange("adi,bist-tone-tx2-mhz", "TX2 Tone (MHz)");

	QLabel *loopbackTitle = new QLabel("Loopback", widget);
	Style::setStyle(loopbackTitle, style::properties::label::menuBig);
	layout->addWidget(loopbackTitle);

	addCheck("adi,bist-tx-rx-loopback-en", "TX -> RX Loopback Enable");
	addCheck("adi,bist-tx-obs-loopback-en", "TX -> OBS Loopback Enable");

	QLabel *prbsTitle = new QLabel("PRBS", widget);
	Style::setStyle(prbsTitle, style::properties::label::menuBig);
	layout->addWidget(prbsTitle);

	addRange("adi,bist-framer-prbs-rx", "Framer PRBS RX");
	addRange("adi,bist-framer-prbs-obs", "Framer PRBS OBS");
	addRange("adi,bist-deframer-prbs", "Deframer PRBS");

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &Ad9371BistWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

Ad9371BistWidget::~Ad9371BistWidget() {}
