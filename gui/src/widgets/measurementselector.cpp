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

#include "widgets/measurementselector.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QSignalBlocker>
#include <QVBoxLayout>

#include <stylehelper.h>

using namespace scopy;
MeasurementSelector::MeasurementSelector(QWidget *parent)
	: QWidget(parent)
{
	lay = new QVBoxLayout(this);
	setLayout(lay);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(6);

	setupToggleAllRow();
}

void MeasurementSelector::setupToggleAllRow()
{
	m_toggleAllItem = new MeasurementSelectorItem("Toggle All", QString(), this);

	connect(m_toggleAllItem->measureCheckbox(), &QCheckBox::toggled, this,
		&MeasurementSelector::toggleAllMeasurement);
	connect(m_toggleAllItem->statsCheckbox(), &QCheckBox::toggled, this, &MeasurementSelector::toggleAllStats);

	lay->addWidget(m_toggleAllItem);
}

MeasurementSelector::~MeasurementSelector() {}

void MeasurementSelector::addMeasurement(QString name, QString icon)
{
	MeasurementSelectorItem *item = new MeasurementSelectorItem(name, icon, this);
	lay->addWidget(item);
	m_map.insert(name, item);

	connect(item->measureCheckbox(), &QCheckBox::toggled, this, &MeasurementSelector::updateToggleAllMeasure);
	connect(item->statsCheckbox(), &QCheckBox::toggled, this, &MeasurementSelector::updateToggleAllStats);
}

void MeasurementSelector::removeMeasurement(QString name)
{
	lay->removeWidget(m_map.value(name));
	m_map.remove(name);
}

MeasurementSelectorItem *MeasurementSelector::measurement(QString name) { return m_map.value(name); }

void MeasurementSelector::toggleAllMeasurement(bool b)
{
	for(int i = 0; i < children().count(); i++) {
		MeasurementSelectorItem *item = dynamic_cast<MeasurementSelectorItem *>(children()[i]);
		if(!item || item == m_toggleAllItem)
			continue;
		item->measureCheckbox()->setChecked(b);
	}
}

void MeasurementSelector::toggleAllStats(bool b)
{
	for(int i = 0; i < children().count(); i++) {
		MeasurementSelectorItem *item = dynamic_cast<MeasurementSelectorItem *>(children()[i]);
		if(!item || item == m_toggleAllItem)
			continue;
		item->statsCheckbox()->setChecked(b);
	}
}

MeasurementSelectorItem::MeasurementSelectorItem(QString name, QString iconPath, QWidget *parent)
	: QWidget(parent)
{
	QHBoxLayout *lay = new QHBoxLayout(this);
	setLayout(lay);

	m_icon = new QLabel("", this);
	m_name = new QLabel(name, this);

	m_measureCheckbox = new QCheckBox(this);
	m_statsCheckbox = new QCheckBox(this);

	lay->addWidget(m_icon);
	lay->addWidget(m_name);
	lay->addWidget(m_measureCheckbox);
	lay->addWidget(m_statsCheckbox);

	lay->setStretch(0, 1);
	lay->setStretch(1, 5);
	lay->setStretch(2, 1);
	lay->setStretch(3, 1);
	StyleHelper::MeasurementSelectorItemWidget(iconPath, this, "MeasurementSelectorItem" + name);
}

MeasurementSelectorItem::~MeasurementSelectorItem() {}

QCheckBox *MeasurementSelectorItem::measureCheckbox() const { return m_measureCheckbox; }

QCheckBox *MeasurementSelectorItem::statsCheckbox() const { return m_statsCheckbox; }

void MeasurementSelector::updateToggleAllMeasure()
{
	bool allChecked = true;
	for(auto *item : std::as_const(m_map)) {
		if(!item->measureCheckbox()->isChecked()) {
			allChecked = false;
			break;
		}
	}
	QSignalBlocker blocker(m_toggleAllItem->measureCheckbox());
	m_toggleAllItem->measureCheckbox()->setChecked(allChecked);
}

void MeasurementSelector::updateToggleAllStats()
{
	bool allChecked = true;
	for(auto *item : std::as_const(m_map)) {
		if(!item->statsCheckbox()->isChecked()) {
			allChecked = false;
			break;
		}
	}
	QSignalBlocker blocker(m_toggleAllItem->statsCheckbox());
	m_toggleAllItem->statsCheckbox()->setChecked(allChecked);
}

#include "moc_measurementselector.cpp"
