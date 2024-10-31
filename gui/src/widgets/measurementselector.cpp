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
#include <QVBoxLayout>

#include <stylehelper.h>

using namespace scopy;
MeasurementSelector::MeasurementSelector(QWidget *parent)
	: QWidget(parent)
{
	lay = new QVBoxLayout(this);
	setLayout(lay);
	lay->setMargin(0);
	lay->setSpacing(6);
}

MeasurementSelector::~MeasurementSelector() {}

void MeasurementSelector::addMeasurement(QString name, QString icon)
{
	MeasurementSelectorItem *item = new MeasurementSelectorItem(name, icon, this);
	lay->addWidget(item);
	m_map.insert(name, item);
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
		if(!item)
			continue;
		item->measureCheckbox()->setChecked(b);
	}
}

void MeasurementSelector::toggleAllStats(bool b)
{
	for(int i = 0; i < children().count(); i++) {
		MeasurementSelectorItem *item = dynamic_cast<MeasurementSelectorItem *>(children()[i]);
		if(!item)
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

#include "moc_measurementselector.cpp"
