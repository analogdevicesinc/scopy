/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "ad963xadvanced.h"
#include "auxadcdaciowidget.h"
#include "elnawidget.h"
#include "ensmmodeclockswidget.h"
#include "gainwidget.h"
#include "rssiwidget.h"
#include "txmonitorwidget.h"
#include "miscwidget.h"

#include <QFutureWatcher>
#include <QtConcurrent>
#include <bistwidget.h>
#include <iiowidgetbuilder.h>
#include <menuonoffswitch.h>
#include <style.h>
#include <toolbuttons.h>
#include <iioutil/connectionprovider.h>

using namespace scopy;
using namespace pluto;

AD936XAdvanced::AD936XAdvanced(QString uri, QWidget *parent)
	: m_uri(uri)
	, QWidget{parent}
{
	m_mainLayout = new QVBoxLayout(this);
	m_mainLayout->setMargin(0);
	m_mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);

	m_mainLayout->addWidget(m_tool);

	m_refreshButton = new AnimatedRefreshBtn(false, this);
	m_tool->addWidgetToTopContainerHelper(m_refreshButton, TTA_RIGHT);

	connect(m_refreshButton, &QPushButton::clicked, this, [this]() {
		m_refreshButton->startAnimation();

		QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
		connect(
			watcher, &QFutureWatcher<void>::finished, this,
			[this, watcher]() {
				m_refreshButton->stopAnimation();
				watcher->deleteLater();
			},
			Qt::QueuedConnection);

		QFuture<void> future = QtConcurrent::run([this]() { Q_EMIT readRequested(); });

		watcher->setFuture(future);
	});

	// main widget body

	QStackedWidget *centralWidget = new QStackedWidget(this);
	m_tool->addWidgetToCentralContainerHelper(centralWidget);

	QButtonGroup *navigationButtons = new QButtonGroup(this);
	navigationButtons->setExclusive(true);

	QPushButton *ensmModeClocksBtn = new QPushButton("ENSM/Mode/Clocks", this);
	Style::setStyle(ensmModeClocksBtn, style::properties::button::blueGrayButton);
	ensmModeClocksBtn->setCheckable(true);
	ensmModeClocksBtn->setChecked(true);

	EnsmModeClocksWidget *ensmModeClocks = new EnsmModeClocksWidget(m_uri, centralWidget);
	centralWidget->addWidget(ensmModeClocks);

	connect(this, &AD936XAdvanced::readRequested, ensmModeClocks, &EnsmModeClocksWidget::readRequested);
	connect(ensmModeClocksBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(ensmModeClocks); });

	QPushButton *eLnaBtn = new QPushButton("eLNA", this);
	Style::setStyle(eLnaBtn, style::properties::button::blueGrayButton);
	eLnaBtn->setCheckable(true);

	ElnaWidget *elna = new ElnaWidget(m_uri, centralWidget);
	centralWidget->addWidget(elna);

	connect(this, &AD936XAdvanced::readRequested, elna, &ElnaWidget::readRequested);
	connect(eLnaBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(elna); });

	QPushButton *rssiBtn = new QPushButton("RSSI", this);
	Style::setStyle(rssiBtn, style::properties::button::blueGrayButton);
	rssiBtn->setCheckable(true);

	RssiWidget *rssi = new RssiWidget(m_uri, centralWidget);
	centralWidget->addWidget(rssi);

	connect(this, &AD936XAdvanced::readRequested, rssi, &RssiWidget::readRequested);
	connect(rssiBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(rssi); });

	QPushButton *gainBtn = new QPushButton("GAIN", this);
	Style::setStyle(gainBtn, style::properties::button::blueGrayButton);
	gainBtn->setCheckable(true);

	GainWidget *gainWidget = new GainWidget(m_uri, centralWidget);
	centralWidget->addWidget(gainWidget);

	connect(this, &AD936XAdvanced::readRequested, gainWidget, &GainWidget::readRequested);
	connect(gainBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(gainWidget); });

	QPushButton *txMonitorBtn = new QPushButton("TX MONITOR", this);
	Style::setStyle(txMonitorBtn, style::properties::button::blueGrayButton);
	txMonitorBtn->setCheckable(true);

	TxMonitorWidget *txMonitor = new TxMonitorWidget(m_uri, centralWidget);
	centralWidget->addWidget(txMonitor);

	connect(this, &AD936XAdvanced::readRequested, txMonitor, &TxMonitorWidget::readRequested);

	connect(txMonitorBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(txMonitor); });

	QPushButton *auxAdcDacIioBtn = new QPushButton("Aux ADC/DAC/IIO", this);
	Style::setStyle(auxAdcDacIioBtn, style::properties::button::blueGrayButton);
	auxAdcDacIioBtn->setCheckable(true);

	AuxAdcDacIoWidget *auxAdcDacIo = new AuxAdcDacIoWidget(m_uri, centralWidget);
	centralWidget->addWidget(auxAdcDacIo);

	connect(this, &AD936XAdvanced::readRequested, auxAdcDacIo, &AuxAdcDacIoWidget::readRequested);

	connect(auxAdcDacIioBtn, &QPushButton::clicked, this,
		[=, this]() { centralWidget->setCurrentWidget(auxAdcDacIo); });

	QPushButton *miscBtn = new QPushButton("MISC", this);
	Style::setStyle(miscBtn, style::properties::button::blueGrayButton);
	miscBtn->setCheckable(true);

	MiscWidget *misc = new MiscWidget(m_uri, centralWidget);
	centralWidget->addWidget(misc);

	connect(this, &AD936XAdvanced::readRequested, misc, &MiscWidget::readRequested);

	connect(miscBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(misc); });

	QPushButton *bistBtn = new QPushButton("BIST", this);
	Style::setStyle(bistBtn, style::properties::button::blueGrayButton);
	bistBtn->setCheckable(true);

	BistWidget *bist = new BistWidget(m_uri, centralWidget);
	centralWidget->addWidget(bist);

	connect(this, &AD936XAdvanced::readRequested, bist, &BistWidget::readRequested);

	connect(bistBtn, &QPushButton::clicked, this, [=, this]() { centralWidget->setCurrentWidget(bist); });

	navigationButtons->addButton(ensmModeClocksBtn);
	navigationButtons->addButton(eLnaBtn);
	navigationButtons->addButton(rssiBtn);
	navigationButtons->addButton(gainBtn);
	navigationButtons->addButton(txMonitorBtn);
	navigationButtons->addButton(auxAdcDacIioBtn);
	navigationButtons->addButton(miscBtn);
	navigationButtons->addButton(bistBtn);

	m_tool->addWidgetToTopContainerHelper(ensmModeClocksBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(eLnaBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(rssiBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(gainBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(txMonitorBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(auxAdcDacIioBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(miscBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(bistBtn, TTA_LEFT);

	// Style::setStyle(scrollWidget, style::properties::widget::border_interactive, true, true);
}

AD936XAdvanced::~AD936XAdvanced()
{
	// close Connection
	ConnectionProvider::close(m_uri);
}
