/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "measurementsettings.h"

#include <widgets/menuonoffswitch.h>
#include <widgets/menusectionwidget.h>
#include <style.h>
#include <QLabel>
#include <QHBoxLayout>

using namespace scopy;

MeasurementSettings::MeasurementSettings(QWidget *parent)
	: QWidget(parent)
{

	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);
	//		setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	lay->setMargin(0);

	measureSection = new MenuSectionWidget(this);
	Style::setStyle(measureSection, style::properties::widget::border);
	measurePanelSwitch = new MenuOnOffSwitch("Measure Panel", this);
	Style::setStyle(measurePanelSwitch->label(), style::properties::label::subtle, false);
	Style::setStyle(measurePanelSwitch->label(), style::properties::label::defaultLabel);
	measurePanelSwitch->onOffswitch()->setChecked(true);
	measureSection->contentLayout()->addWidget(measurePanelSwitch);

	connect(measurePanelSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { Q_EMIT enableMeasurementPanel(b); });

	statsSection = new MenuSectionWidget(this);
	Style::setStyle(statsSection, style::properties::widget::border);
	statsPanelSwitch = new MenuOnOffSwitch("Stats Panel", this);
	Style::setStyle(statsPanelSwitch->label(), style::properties::label::subtle, false);
	Style::setStyle(statsPanelSwitch->label(), style::properties::label::defaultLabel);
	connect(statsPanelSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { Q_EMIT enableStatsPanel(b); });
	statsSection->contentLayout()->addWidget(statsPanelSwitch);

	statsPanelSwitch->onOffswitch()->setChecked(false);

	markerSection = new MenuSectionWidget(this);
	Style::setStyle(markerSection, style::properties::widget::border);
	markerPanelSwitch = new MenuOnOffSwitch("Marker Panel", this);
	Style::setStyle(markerPanelSwitch->label(), style::properties::label::subtle, false);
	Style::setStyle(markerPanelSwitch->label(), style::properties::label::defaultLabel, true, true);

	connect(markerPanelSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { Q_EMIT enableMarkerPanel(b); });
	markerSection->contentLayout()->addWidget(markerPanelSwitch);

	markerPanelSwitch->onOffswitch()->setChecked(false);

	genalyzerSection = new MenuSectionWidget(this);
	Style::setStyle(genalyzerSection, style::properties::widget::border);
	genalyzerPanelSwitch = new MenuOnOffSwitch("Genalyzer analysis", this);
	Style::setStyle(genalyzerPanelSwitch->label(), style::properties::label::subtle, false);
	Style::setStyle(genalyzerPanelSwitch->label(), style::properties::label::defaultLabel);

	connect(genalyzerPanelSwitch->onOffswitch(), &QAbstractButton::toggled, this,
		[=](bool b) { Q_EMIT enableGenalyzerPanel(b); });
	genalyzerSection->contentLayout()->addWidget(genalyzerPanelSwitch);

	genalyzerPanelSwitch->onOffswitch()->setChecked(false);

	lay->addWidget(measureSection);
	lay->addWidget(statsSection);
	lay->addWidget(markerSection);
	lay->addWidget(genalyzerSection);
}

MeasurementSettings::~MeasurementSettings() {}

bool MeasurementSettings::measurementEnabled() { return measurePanelSwitch->onOffswitch()->isChecked(); }
bool MeasurementSettings::statsEnabled() { return statsPanelSwitch->onOffswitch()->isChecked(); }
bool MeasurementSettings::markerEnabled() { return markerPanelSwitch->onOffswitch()->isChecked(); }
bool MeasurementSettings::genalyzerEnabled() { return genalyzerPanelSwitch->onOffswitch()->isChecked(); }

void MeasurementSettings::addGenalyzerWidget(QWidget *widget)
{
	if(widget && genalyzerSection) {
		genalyzerSection->contentLayout()->addWidget(widget);
	}
}

MenuSectionWidget *MeasurementSettings::getMarkerSection() const { return markerSection; }

MenuSectionWidget *MeasurementSettings::getStatsSection() const { return statsSection; }

MenuSectionWidget *MeasurementSettings::getMeasureSection() const { return measureSection; }

MenuSectionWidget *MeasurementSettings::getGenalyzerSection() const { return genalyzerSection; }

#include "moc_measurementsettings.cpp"
