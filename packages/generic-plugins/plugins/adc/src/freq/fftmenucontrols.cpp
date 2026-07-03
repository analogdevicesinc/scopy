/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "fftmenucontrols.h"
#include <gui/plotmarkercontroller.h>
#include <gui/smallOnOffSwitch.h>
#include <gui/widgets/menucollapsesection.h>
#include <style.h>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

namespace scopy::adc {

MarkerMenuControls buildMarkerMenu(QWidget *parent, bool withImage)
{
	MarkerMenuControls c;
	c.section = new MenuSectionCollapseWidget("MARKER", MenuCollapseSection::MHCW_ONOFF,
						  MenuCollapseSection::MHW_BASEWIDGET, parent);

	auto layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(0, 0, 0, 0);

	c.typeCombo = new MenuCombo("Marker Type", c.section);
	c.typeCombo->combo()->addItem("Peak", PlotMarkerController::MC_PEAK);
	c.typeCombo->combo()->addItem("Fixed", PlotMarkerController::MC_FIXED);
	c.typeCombo->combo()->addItem("Single Tone", PlotMarkerController::MC_SINGLETONE);
	if(withImage) {
		c.typeCombo->combo()->addItem("Image", PlotMarkerController::MC_IMAGE);
	}
	c.typeCombo->combo()->setCurrentIndex(0);

	c.fixedEditSwitch = new MenuOnOffSwitch("Marker editable", c.section);
	c.fixedEditSwitch->onOffswitch()->setChecked(true);
	c.fixedEditSwitch->setVisible(false);

	c.countSpin = new gui::MenuSpinbox("Marker count", 5, "markers", 0, 9, true, false, c.section);
	c.countSpin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	c.countSpin->setScaleRange(1, 10);
	c.countSpin->setValue(5);

	layout->addWidget(c.typeCombo);
	layout->addWidget(c.countSpin);
	layout->addWidget(c.fixedEditSwitch);

	c.section->contentLayout()->addLayout(layout);
	c.section->setCollapsed(true);
	return c;
}

AveragingMenuControls buildAveragingMenu(QWidget *parent)
{
	AveragingMenuControls c;
	c.section = new MenuSectionCollapseWidget("AVERAGING", MenuCollapseSection::MHCW_ONOFF,
						  MenuCollapseSection::MHW_BASEWIDGET, parent);

	auto layout = new QVBoxLayout();
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);

	QLabel *avgLabel = new QLabel("Size", c.section);
	Style::setStyle(avgLabel, style::properties::label::subtle);
	c.sizeSpin = new QSpinBox(c.section);
	c.sizeSpin->setRange(2, 1000);
	c.sizeSpin->setValue(2);

	layout->addWidget(avgLabel);
	layout->addWidget(c.sizeSpin);

	c.section->contentLayout()->addLayout(layout);
	c.section->setCollapsed(true);
	return c;
}

MinMaxHoldMenuControls buildMinMaxHoldMenu(QWidget *parent)
{
	MinMaxHoldMenuControls c;
	c.section = new MenuSectionCollapseWidget("MIN/MAX HOLD", MenuCollapseSection::MHCW_ONOFF,
						  MenuCollapseSection::MHW_BASEWIDGET, parent);

	auto layout = new QVBoxLayout();
	layout->setSpacing(5);
	layout->setContentsMargins(0, 0, 0, 0);
	int btnSize = Style::getDimension(json::global::unit_2);

	auto buildRow = [&](const QString &title, SmallOnOffSwitch *&sw, QPushButton *&reset) {
		auto row = new QHBoxLayout();
		row->setSpacing(10);
		row->setContentsMargins(0, 0, 0, 0);
		QLabel *lbl = new QLabel(title, c.section);
		Style::setStyle(lbl, style::properties::label::subtle);
		sw = new SmallOnOffSwitch(c.section);
		reset = new QPushButton("Reset", c.section);
		reset->setIcon(
			Style::getPixmap(":/gui/icons/refresh.svg", Style::getColor(json::theme::content_inverse)));
		reset->setFixedHeight(btnSize);
		reset->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		Style::setStyle(reset, style::properties::button::grayButton);
		row->addWidget(lbl);
		row->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));
		row->addWidget(reset);
		row->addWidget(sw);
		return row;
	};

	layout->addLayout(buildRow("Min curve", c.minSwitch, c.minReset));
	layout->addLayout(buildRow("Max curve", c.maxSwitch, c.maxReset));

	c.section->contentLayout()->addLayout(layout);
	c.section->setCollapsed(true);
	return c;
}

} // namespace scopy::adc
