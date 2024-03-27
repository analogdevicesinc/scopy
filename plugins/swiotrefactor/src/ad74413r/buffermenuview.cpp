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

#include <cfloat>
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
}

BufferMenuView::~BufferMenuView() {}

void BufferMenuView::init(QString title, QString function, QPen color, QString unit, double yMin, double yMax)
{
	m_swiotAdvMenu = BufferMenuBuilder::newAdvMenu(this, function, m_connection, m_chnls);

	QScrollArea *scrollArea = new QScrollArea(this);
	QWidget *scrollWidget = new QWidget(scrollArea);
	QVBoxLayout *layScroll = new QVBoxLayout(scrollWidget);
	layScroll->setSpacing(10);
	layScroll->setMargin(0);
	scrollWidget->setLayout(layScroll);

	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setWidgetResizable(true);
	scrollArea->setWidget(scrollWidget);

	MenuHeaderWidget *header = new MenuHeaderWidget(title, color, scrollWidget);
	QWidget *descriptionSection = createDescriptionSection(scrollWidget);
	QWidget *yAxisMenu = createVerticalSettingsMenu(unit, yMin, yMax, scrollWidget);
	QWidget *attrSection = createAttrSection(scrollWidget);

	layScroll->addWidget(header);
	layScroll->addWidget(descriptionSection);
	layScroll->addWidget(yAxisMenu);
	layScroll->addWidget(attrSection);
	layScroll->addItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

	layout()->addWidget(scrollArea);

	createConnections();
}

void BufferMenuView::createConnections()
{
	connect(m_swiotAdvMenu, &BufferMenu::diagnosticFunctionUpdated, this,
		&BufferMenuView::diagnosticFunctionUpdated);
	connect(m_swiotAdvMenu, &BufferMenu::samplingFrequencyUpdated, this, &BufferMenuView::samplingFrequencyUpdated);
	connect(m_swiotAdvMenu, &BufferMenu::thresholdChangeStart, this, [&]() { Q_EMIT thresholdWritten(false); });
	connect(m_swiotAdvMenu, &BufferMenu::thresholdChangeEnd, this, [&]() { Q_EMIT thresholdWritten(true); });
	connect(this, &BufferMenuView::broadcastThreshold, m_swiotAdvMenu, &BufferMenu::onBroadcastThreshold);
	connect(m_swiotAdvMenu, &BufferMenu::diagSamplingFreqChange, this, &BufferMenuView::diagSamplingFreqChange);
	connect(this, &BufferMenuView::updateDiagSamplingFreq, m_swiotAdvMenu, &BufferMenu::onDiagSamplingChange);
	connect(m_swiotAdvMenu, &BufferMenu::freqChangeStart, this, [&]() { Q_EMIT samplingFreqWritten(false); });
	connect(m_swiotAdvMenu, &BufferMenu::freqChangeEnd, this, [&]() { Q_EMIT samplingFreqWritten(true); });

	connect(this, &BufferMenuView::runBtnsPressed, m_swiotAdvMenu, &BufferMenu::onRunBtnsPressed);
}

QWidget *BufferMenuView::createDescriptionSection(QWidget *parent)
{
	MenuSectionWidget *descriptionContainer = new MenuSectionWidget(parent);
	MenuCollapseSection *descriptionSection =
		new MenuCollapseSection("FUNCTION DESCRIPTION", MenuCollapseSection::MHCW_NONE, descriptionContainer);

	QLabel *description = new QLabel(descriptionContainer);
	description->setText(m_swiotAdvMenu->getInfoMessage());
	description->setTextFormat(Qt::TextFormat::RichText);
	description->setWordWrap(true);

	descriptionSection->contentLayout()->addWidget(description);
	descriptionContainer->contentLayout()->addWidget(descriptionSection);

	return descriptionContainer;
}

QWidget *BufferMenuView::createAttrSection(QWidget *parent)
{
	MenuSectionWidget *attrContainer = new MenuSectionWidget(parent);
	auto layout = new QVBoxLayout(attrContainer);
	layout->setSpacing(10);
	layout->setMargin(0);

	MenuCollapseSection *attrSection =
		new MenuCollapseSection("ATTRIBUTES", MenuCollapseSection::MHCW_NONE, attrContainer);

	QList<QWidget *> widgets = m_swiotAdvMenu->getWidgetsList();
	for(QWidget *w : qAsConst(widgets)) {
		layout->addWidget(w);
	}

	attrSection->contentLayout()->addLayout(layout);
	attrContainer->contentLayout()->addWidget(attrSection);

	return attrContainer;
}

QWidget *BufferMenuView::createVerticalSettingsMenu(QString unit, double yMin, double yMax, QWidget *parent)
{
	MenuSectionWidget *verticalContainer = new MenuSectionWidget(this);
	auto layout = new QHBoxLayout(verticalContainer);
	layout->setSpacing(10);
	layout->setMargin(0);

	MenuCollapseSection *verticalSettings =
		new MenuCollapseSection("Y-AXIS", MenuCollapseSection::MHCW_NONE, verticalContainer);

	auto m_yMin = new PositionSpinButton(
		{
			{"" + unit, 1e0},
			{"k" + unit, 1e3},
			{"M" + unit, 1e6},
			{"G" + unit, 1e9},
		},
		"YMin", -DBL_MAX, DBL_MAX, false, false, verticalContainer);
	m_yMin->setValue(yMin);

	auto m_yMax = new PositionSpinButton(
		{
			{"" + unit, 1e0},
			{"k" + unit, 1e3},
			{"M" + unit, 1e6},
			{"G" + unit, 1e9},
		},
		"YMax", -DBL_MAX, DBL_MAX, false, false, verticalContainer);
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
