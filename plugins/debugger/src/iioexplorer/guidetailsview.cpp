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

#include "guidetailsview.h"

using namespace scopy::debugger;

GuiDetailsView::GuiDetailsView(QWidget *parent)
	: QWidget(parent)
	, m_scrollArea(new QScrollArea(this))
	, m_scrollAreaContents(new QWidget(this))
	, m_spacer(new QSpacerItem(0, 0, QSizePolicy::Preferred, QSizePolicy::Maximum))
	, m_wheelGuard(new MouseWheelWidgetGuard(this))
{
	setupUi();
}

void GuiDetailsView::setupUi()
{
	setContentsMargins(0, 0, 0, 0);
	setLayout(new QVBoxLayout(this));

	m_scrollAreaContents->setLayout(new QVBoxLayout(m_scrollAreaContents));
	m_scrollAreaContents->setObjectName("DetailsViewScrollAreaContents");
	m_scrollAreaContents->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setWidget(m_scrollAreaContents);

	MenuCollapseSection *attrSection = new MenuCollapseSection("Attributes", MenuCollapseSection::MHCW_ARROW,
								   MenuCollapseSection::MHW_BASEWIDGET, this);
	attrSection->contentLayout()->setSpacing(10);
	attrSection->contentLayout()->setMargin(0);
	attrSection->contentLayout()->addWidget(m_scrollArea);
	attrSection->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
	connect(attrSection->header(), &QAbstractButton::clicked, this, [this, attrSection](bool checked) {
		attrSection->setSizePolicy(QSizePolicy::Preferred, checked ? QSizePolicy::Minimum : QSizePolicy::Fixed);
		m_spacer->changeSize(0, 0, QSizePolicy::Preferred,
				     checked ? QSizePolicy::Minimum : QSizePolicy::Expanding);
	});

	m_detailsSeparator = new MenuCollapseSection("General info", MenuCollapseSection::MHCW_ARROW,
						     MenuCollapseSection::MHW_BASEWIDGET, this);
	m_detailsSeparator->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	layout()->addWidget(attrSection);
	layout()->addWidget(m_detailsSeparator);
	layout()->addItem(m_spacer);
}

void GuiDetailsView::setIIOStandardItem(IIOStandardItem *item)
{
	m_currentItem = item;
	clearWidgets();
	QList<IIOWidget *> iioWidgets = item->getIIOWidgets();
	for(IIOWidget *widget : iioWidgets) {
		widget->show();
		m_currentWidgets.append(widget);
		widget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
		m_scrollAreaContents->layout()->addWidget(widget);
	}

	QStringList details = item->details();
	for(const QString &detail : details) {
		auto label = new QLabel(detail, m_detailsSeparator);
		m_detailsList.append(label);
		m_detailsSeparator->contentLayout()->addWidget(label);
		label->show();
	}

	QLabel *detailsLabel = dynamic_cast<QLabel *>(m_detailsSeparator->header()->layout()->itemAt(0)->widget());
	if(detailsLabel) {
		detailsLabel->setText(m_currentItem->typeString() + " info");
	}

	m_wheelGuard->installEventRecursively(this);
}

void GuiDetailsView::clearWidgets()
{
	QLayoutItem *child;
	while((child = m_scrollAreaContents->layout()->takeAt(0)) != nullptr) {
		child->widget()->hide();
		m_scrollAreaContents->layout()->removeWidget(child->widget());
	}
	m_currentWidgets.clear();

	while((child = m_detailsSeparator->contentLayout()->takeAt(0)) != nullptr) {
		child->widget()->hide();
		m_detailsSeparator->contentLayout()->removeWidget(child->widget());
	}
	m_detailsList.clear();
}

#include "moc_guidetailsview.cpp"
