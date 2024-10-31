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

#include "menus/channelattributesmenu.hpp"

#include <iiowidgetbuilder.h>
#include <menuheader.h>

using namespace scopy;
using namespace datamonitor;

ChannelAttributesMenu::ChannelAttributesMenu(DataMonitorModel *model, QWidget *parent)
	: QWidget{parent}
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setMargin(0);
	mainLayout->setSpacing(10);
	setLayout(mainLayout);

	MenuHeaderWidget *header = new MenuHeaderWidget(model->getName(), model->getColor(), this);
	mainLayout->addWidget(header);

	QWidget *settingsBody = new QWidget(this);
	QVBoxLayout *layout = new QVBoxLayout();
	layout->setMargin(0);
	layout->setSpacing(10);
	settingsBody->setLayout(layout);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(settingsBody);
	mainLayout->addWidget(scrollArea);

	MenuSectionWidget *attrcontainer = new MenuSectionWidget(parent);
	MenuCollapseSection *attr = new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE,
							    MenuCollapseSection::MHW_BASEWIDGET, attrcontainer);

	QVBoxLayout *attrLayout = new QVBoxLayout();
	attrLayout->setSpacing(10);
	attrLayout->setMargin(0);
	attrLayout->setContentsMargins(0, 0, 0, 10); // bottom margin

	if(qobject_cast<DmmDataMonitorModel *>(model)) {
		QList<IIOWidget *> attrWidgets =
			IIOWidgetBuilder(attr)
				.channel(dynamic_cast<DmmDataMonitorModel *>(model)->iioChannel())
				.buildAll();

		for(auto w : attrWidgets) {
			attrLayout->addWidget(w);
		}

	} else {
		attrLayout->addWidget(new QLabel("NO ATTRIBUTE FOUND !"));
	}

	attr->contentLayout()->addLayout(attrLayout);
	attrcontainer->contentLayout()->addWidget(attr);

	layout->addWidget(attrcontainer);

	QSpacerItem *spacer = new QSpacerItem(10, 10, QSizePolicy::Preferred, QSizePolicy::Expanding);
	layout->addItem(spacer);
}

#include "moc_channelattributesmenu.cpp"
