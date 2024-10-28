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

#include "plotmanagercombobox.h"
#include "plotmanager.h"
#include <menusectionwidget.h>

using namespace scopy;

PlotManagerCombobox::PlotManagerCombobox(PlotManager *man, ChannelComponent *c, QWidget *parent)
	: QWidget(parent)
{
	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);
	lay->setSpacing(0);
	lay->setMargin(0);

	MenuSectionWidget *sec = new MenuSectionWidget(this);
	m_mcombo = new MenuCombo("PLOT", sec);
	m_combo = m_mcombo->combo();
	m_man = man;
	m_ch = c;

	// add all plots from manager
	for(PlotComponent *plt : man->plots()) {
		addPlot(plt);
	}

	// select current plot in combo
	uint32_t uuid = c->plotChannelCmpt()->plotComponent()->uuid();
	m_combo->setCurrentIndex(findIndexFromUuid(uuid));

	connect(m_combo, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		uint32_t uuid = m_combo->itemData(idx).toULongLong();
		man->moveChannel(m_ch, uuid);
		man->replot();
	});
	lay->addWidget(sec);
	sec->contentLayout()->addWidget(m_mcombo);
}

PlotManagerCombobox::~PlotManagerCombobox() {}

void PlotManagerCombobox::renamePlotSlot()
{
	PlotComponent *plt = dynamic_cast<PlotComponent *>(QObject::sender());
	renamePlot(plt);
}

void PlotManagerCombobox::addPlot(PlotComponent *p)
{
	m_combo->addItem(p->name(), p->uuid());
	connect(p, &PlotComponent ::nameChanged, this, &PlotManagerCombobox::renamePlotSlot);
}

void PlotManagerCombobox::removePlot(PlotComponent *p)
{
	int idx = findIndexFromUuid(p->uuid());
	m_combo->removeItem(idx);
	disconnect(p, &PlotComponent ::nameChanged, this, &PlotManagerCombobox::renamePlotSlot);
}

void PlotManagerCombobox::renamePlot(PlotComponent *p)
{
	int idx = findIndexFromUuid(p->uuid());
	m_combo->setItemText(idx, p->name());
}

int PlotManagerCombobox::findIndexFromUuid(uint32_t uuid)
{
	for(int i = 0; i < m_combo->count(); i++) {
		if(uuid == m_combo->itemData(i)) {
			return i;
		}
	}
	return -1;
}

#include "moc_plotmanagercombobox.cpp"
