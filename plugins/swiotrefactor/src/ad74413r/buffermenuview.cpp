/*
 * Copyright (c) 2023 Analog Devices Inc.
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
 */

#include "ad74413r/buffermenuview.h"

#include <QList>

#include <gui/widgets/menuheader.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>

using namespace scopy::swiotrefactor;

BufferMenuView::BufferMenuView(QMap<QString, iio_channel *> chnls, Connection *conn, QWidget *parent)
	: QWidget(parent)
	, m_swiotAdvMenu(nullptr)
	, m_chnls(chnls)
	, m_connection(conn)
{
	setLayout(new QVBoxLayout());
	layout()->setMargin(0);
	layout()->setSpacing(10);
}

BufferMenuView::~BufferMenuView() {}

void BufferMenuView::init(QString title, QString function, QPen color, QString unit, double yMin, double yMax)
{
	m_swiotAdvMenu = BufferMenuBuilder::newAdvMenu(this, function, m_connection, m_chnls);

	MenuHeaderWidget *header = new MenuHeaderWidget(title, color, this);

	MenuSectionWidget *attrContainer = new MenuSectionWidget(this);
	auto layout = new QVBoxLayout(attrContainer);
	layout->setSpacing(10);
	layout->setContentsMargins(0, 0, 0, 10);
	layout->setMargin(0);
	MenuCollapseSection *attrSection =
		new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, attrContainer);

	QList<QWidget *> widgets = m_swiotAdvMenu->getWidgetsList();
	for(QWidget *w : qAsConst(widgets)) {
		layout->addWidget(w);
	}

	attrSection->contentLayout()->addLayout(layout);
	attrContainer->contentLayout()->addWidget(attrSection);

	QWidget *yAxisMenu = createVerticalSettingsMenu(unit, yMin, yMax, this);

	this->layout()->addWidget(header);
	this->layout()->addWidget(yAxisMenu);
	this->layout()->addWidget(attrContainer);
	this->layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));
	createConnections();
}

void BufferMenuView::createConnections()
{
	connect(m_swiotAdvMenu, &BufferMenu::diagnosticFunctionUpdated, this,
		&BufferMenuView::diagnosticFunctionUpdated);
	connect(m_swiotAdvMenu, &BufferMenu::samplingFrequencyUpdated, this, &BufferMenuView::samplingFrequencyUpdated);
	connect(m_swiotAdvMenu, &BufferMenu::broadcastThreshold, this, &BufferMenuView::broadcastThresholdForward);
	connect(this, &BufferMenuView::broadcastThresholdBackward, m_swiotAdvMenu, &BufferMenu::onBroadcastThreshold);
}

QWidget *BufferMenuView::createVerticalSettingsMenu(QString unit, double yMin, double yMax, QWidget *parent)
{
	MenuSectionWidget *verticalContainer = new MenuSectionWidget(this);
	auto layout = new QHBoxLayout(verticalContainer);
	layout->setSpacing(10);
	layout->setContentsMargins(0, 0, 0, 10);
	layout->setMargin(0);

	MenuCollapseSection *verticalSettings =
		new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, verticalContainer);
	auto m_yMin = new PositionSpinButton(
		{
			{unit, 1e0},
		},
		"YMin", yMin, yMax, false, false, verticalContainer);
	m_yMin->setValue(yMin);

	auto m_yMax = new PositionSpinButton(
		{
			{unit, 1e0},
		},
		"YMax", yMin, yMax, false, false, verticalContainer);
	m_yMax->setValue(yMax);

	layout->addWidget(m_yMin);
	layout->addWidget(m_yMax);

	verticalSettings->contentLayout()->addLayout(layout);
	verticalContainer->contentLayout()->addWidget(verticalSettings);

	// Connects
	connect(m_yMin, &PositionSpinButton::valueChanged, this, &BufferMenuView::setYMin);
	connect(this, &BufferMenuView::minChanged, this, [=](double min) {
		QSignalBlocker b(m_yMin);
		m_yMin->setValue(min);
	});

	connect(m_yMax, &PositionSpinButton::valueChanged, this, &BufferMenuView::setYMax);
	connect(this, &BufferMenuView::maxChanged, this, [=](double max) {
		QSignalBlocker b(m_yMax);
		m_yMax->setValue(max);
	});
	return verticalContainer;
}

BufferMenu *BufferMenuView::getAdvMenu() { return m_swiotAdvMenu; }
