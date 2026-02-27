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

#include "gainsetupwidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

GainSetupWidget::GainSetupWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
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

	QLabel *rxTitle = new QLabel("RX", widget);
	Style::setStyle(rxTitle, style::properties::label::menuBig);
	layout->addWidget(rxTitle);

	addRange("adi,rx-gain-control-mode", "RX Gain Control Mode");
	addRange("adi,rx1-gain-index", "RX1 Gain Index");
	addRange("adi,rx2-gain-index", "RX2 Gain Index");
	addRange("adi,rx1-max-gain-index", "RX1 Max Gain Index");
	addRange("adi,rx1-min-gain-index", "RX1 Min Gain Index");
	addRange("adi,rx2-max-gain-index", "RX2 Max Gain Index");
	addRange("adi,rx2-min-gain-index", "RX2 Min Gain Index");

	QLabel *obsTitle = new QLabel("Observation", widget);
	Style::setStyle(obsTitle, style::properties::label::menuBig);
	layout->addWidget(obsTitle);

	addRange("adi,obs-rx-gain-control-mode", "OBS RX Gain Control Mode");
	addRange("adi,orx1-gain-index", "ORX1 Gain Index");
	addRange("adi,orx2-gain-index", "ORX2 Gain Index");
	addRange("adi,obs-rx-max-gain-index", "OBS RX Max Gain Index");
	addRange("adi,obs-rx-min-gain-index", "OBS RX Min Gain Index");

	QLabel *snifferTitle = new QLabel("Sniffer", widget);
	Style::setStyle(snifferTitle, style::properties::label::menuBig);
	layout->addWidget(snifferTitle);

	addRange("adi,sniffer-rx-gain-control-mode", "Sniffer RX Gain Control Mode");
	addRange("adi,sniffer-gain-index", "Sniffer Gain Index");
	addRange("adi,sniffer-rx-max-gain-index", "Sniffer RX Max Gain Index");
	addRange("adi,sniffer-rx-min-gain-index", "Sniffer RX Min Gain Index");

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &GainSetupWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

GainSetupWidget::~GainSetupWidget() {}
