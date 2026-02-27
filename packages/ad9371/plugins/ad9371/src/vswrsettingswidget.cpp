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

#include "vswrsettingswidget.h"

#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

VswrSettingsWidget::VswrSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
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

	QLabel *title = new QLabel("VSWR Settings", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	QList<IIOWidget *> allWidgets;

	auto addRange = [&](const char *attr, const QString &t) {
		IIOWidget *w = makeDebugRange(widget, m_device, attr, t, m_group);
		layout->addWidget(w);
		allWidgets.append(w);
	};

	addRange("adi,vswr-additional-delay-offset", "Additional Delay Offset");
	addRange("adi,vswr-path-delay-pn-seq-level", "Path Delay PN Seq Level");
	addRange("adi,vswr-tx1-vswr-switch-gpio3p3-pin", "TX1 VSWR Switch GPIO3.3 Pin");
	addRange("adi,vswr-tx1-vswr-switch-delay-us", "TX1 Switch Delay (us)");
	addRange("adi,vswr-tx2-vswr-switch-gpio3p3-pin", "TX2 VSWR Switch GPIO3.3 Pin");
	addRange("adi,vswr-tx2-vswr-switch-delay-us", "TX2 Switch Delay (us)");

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &VswrSettingsWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

VswrSettingsWidget::~VswrSettingsWidget() {}
