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

#include "jesdframerwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <style.h>
#include <ad9371advutils.h>

using namespace scopy;
using namespace ad9371;

JesdFramerWidget::JesdFramerWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
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

	QLabel *title = new QLabel("JESD Framer", widget);
	Style::setStyle(title, style::properties::label::menuBig);
	layout->addWidget(title);

	QList<IIOWidget *> allWidgets;

	auto addFramerRow = [&](QGridLayout *grid, int r, const QString &label, const char *rxAttr,
				const char *obsAttr) {
		grid->addWidget(new QLabel(label, widget), r, 0);
		IIOWidget *rxW = makeDebugRange(widget, m_device, rxAttr, "", m_group);
		grid->addWidget(rxW, r, 1);
		allWidgets.append(rxW);
		IIOWidget *obsW = makeDebugRange(widget, m_device, obsAttr, "", m_group);
		grid->addWidget(obsW, r, 2);
		allWidgets.append(obsW);
	};

	QGridLayout *grid = new QGridLayout();
	grid->addWidget(new QLabel("", widget), 0, 0);
	grid->addWidget(new QLabel("RX", widget), 0, 1);
	grid->addWidget(new QLabel("Observation RX", widget), 0, 2);
	int row = 1;

	addFramerRow(grid, row++, "Bank ID", "adi,jesd204-rx-framer-bank-id", "adi,jesd204-obs-framer-bank-id");
	addFramerRow(grid, row++, "Device ID", "adi,jesd204-rx-framer-device-id", "adi,jesd204-obs-framer-device-id");
	addFramerRow(grid, row++, "Lane0 ID", "adi,jesd204-rx-framer-lane0-id", "adi,jesd204-obs-framer-lane0-id");
	addFramerRow(grid, row++, "M", "adi,jesd204-rx-framer-m", "adi,jesd204-obs-framer-m");
	addFramerRow(grid, row++, "K", "adi,jesd204-rx-framer-k", "adi,jesd204-obs-framer-k");
	addFramerRow(grid, row++, "Scramble", "adi,jesd204-rx-framer-scramble", "adi,jesd204-obs-framer-scramble");
	addFramerRow(grid, row++, "External Sysref", "adi,jesd204-rx-framer-external-sysref",
		     "adi,jesd204-obs-framer-external-sysref");
	addFramerRow(grid, row++, "Serializer Lane Crossbar", "adi,jesd204-rx-framer-serializer-lane-crossbar",
		     "adi,jesd204-obs-framer-serializer-lane-crossbar");
	addFramerRow(grid, row++, "Serializer Amplitude", "adi,jesd204-rx-framer-serializer-amplitude",
		     "adi,jesd204-obs-framer-serializer-amplitude");
	addFramerRow(grid, row++, "Pre-Emphasis", "adi,jesd204-rx-framer-pre-emphasis",
		     "adi,jesd204-obs-framer-pre-emphasis");
	addFramerRow(grid, row++, "LMFC Offset", "adi,jesd204-rx-framer-lmfc-offset",
		     "adi,jesd204-obs-framer-lmfc-offset");
	addFramerRow(grid, row++, "New Sysref On Relink", "adi,jesd204-rx-framer-new-sysref-on-relink",
		     "adi,jesd204-obs-framer-new-sysref-on-relink");
	addFramerRow(grid, row++, "Enable Auto Chan Xbar", "adi,jesd204-rx-framer-enable-auto-chan-xbar",
		     "adi,jesd204-obs-framer-enable-auto-chan-xbar");
	layout->addLayout(grid);

	m_layout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	connect(this, &JesdFramerWidget::readRequested, this, [=, this]() {
		for(auto *w : allWidgets) {
			w->readAsync();
		}
	});
}

JesdFramerWidget::~JesdFramerWidget() {}
