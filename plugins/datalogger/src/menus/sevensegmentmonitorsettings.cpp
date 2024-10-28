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

#include "menus/sevensegmentmonitorsettings.hpp"

#include <QBoxLayout>
#include <datamonitorstylehelper.hpp>
#include <datamonitorutils.hpp>
#include <menucollapsesection.h>
#include <menusectionwidget.h>

using namespace scopy;
using namespace datamonitor;

SevenSegmentMonitorSettings::SevenSegmentMonitorSettings(QWidget *parent)
	: QWidget{parent}
{
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setMargin(0);
	layout->setSpacing(10);
	this->setLayout(layout);

	MenuSectionWidget *sevenSegmentSettingsContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *sevenSegmentSettingsSection =
		new MenuCollapseSection("7 SEGMENT", MenuCollapseSection::MHCW_NONE,
					MenuCollapseSection::MHW_BASEWIDGET, sevenSegmentSettingsContainer);

	sevenSegmentSettingsContainer->contentLayout()->addWidget(sevenSegmentSettingsSection);
	sevenSegmentSettingsSection->contentLayout()->setSpacing(10);

	QHBoxLayout *precisionLayout = new QHBoxLayout();

	precision = new QLineEdit(sevenSegmentSettingsSection);
	precision->setText(QString::number(DataMonitorUtils::getDefaultPrecision()));

	precisionLayout->addWidget(new QLabel("Precision: ", this));
	precisionLayout->addWidget(precision);

	connect(precision, &QLineEdit::returnPressed, this, &SevenSegmentMonitorSettings::changePrecision);
	connect(precision, &QLineEdit::textChanged, this, &SevenSegmentMonitorSettings::changePrecision);

	peakHolderToggle = new MenuOnOffSwitch(tr("Min/Max"), sevenSegmentSettingsSection, false);
	peakHolderToggle->onOffswitch()->setChecked(true);

	connect(peakHolderToggle->onOffswitch(), &QAbstractButton::toggled, this,
		&SevenSegmentMonitorSettings::peakHolderToggled);

	sevenSegmentSettingsSection->contentLayout()->addLayout(precisionLayout);
	sevenSegmentSettingsSection->contentLayout()->addWidget(peakHolderToggle);

	layout->addWidget(sevenSegmentSettingsContainer);

	DataMonitorStyleHelper::SevenSegmentMonitorMenuStyle(this);
}

void SevenSegmentMonitorSettings::changePrecision()
{
	// precision value can be between 0 and 9
	auto value = precision->text().toInt();
	if(value < 0) {
		precision->setText("0");
	}
	if(value >= 10) {
		precision->setText("9");
	}

	Q_EMIT precisionChanged(precision->text().toInt());
}

#include "moc_sevensegmentmonitorsettings.cpp"
