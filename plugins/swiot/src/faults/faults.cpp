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

#include "faults/faults.h"

#include "swiot_logging_categories.h"

#include <QDesktopServices>
#include <QThread>
#include <QTimer>
#include <tutorialbuilder.h>
#include <gui/stylehelper.h>
#include <pluginbase/preferences.h>

using namespace scopy::swiot;

#define POLLING_INTERVAL 1000

Faults::Faults(QString uri, ToolMenuEntry *tme, QWidget *parent)
	: QWidget(parent)
	, m_timer(new QTimer())
	, m_thread(new QThread(this))
	, m_tme(tme)
{
	qInfo(CAT_SWIOT_FAULTS) << "Initialising SWIOT faults page.";
	QHBoxLayout *layout = new QHBoxLayout(this);
	layout->setContentsMargins(0, 0, 0, 0);
	setLayout(layout);

	// tool template configuration
	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->centralContainer()->setVisible(true);
	m_tool->topContainer()->setVisible(true);

	layout->addWidget(m_tool);

	InfoBtn *infoBtn = new InfoBtn(this);
	m_tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);
	connect(infoBtn, &QAbstractButton::clicked, this, [=, this]() {
		QDesktopServices::openUrl(QUrl("https://analogdevicesinc.github.io/scopy/plugins/swiot1l/faults.html"));
	});

	m_configBtn = createConfigBtn(this);
	m_runBtn = new RunBtn(this);
	m_singleBtn = new SingleShotBtn(this);

	m_tool->addWidgetToTopContainerHelper(m_configBtn, TTA_LEFT);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	m_tool->addWidgetToTopContainerHelper(m_singleBtn, TTA_RIGHT);

	m_faultsPage = new FaultsPage(uri, this);
	m_tool->addWidgetToCentralContainerHelper(m_faultsPage);

	connectSignalsAndSlots();
	initTutorialProperties();
}

Faults::~Faults()
{
	if(m_thread) {
		m_thread->quit();
		m_thread->wait();
		delete m_thread;
	}
}

void Faults::connectSignalsAndSlots()
{
	QObject::connect(m_runBtn, &QPushButton::toggled, this, &Faults::runButtonClicked);
	QObject::connect(m_singleBtn, &QPushButton::clicked, this, &Faults::singleButtonClicked);
	QObject::connect(m_configBtn, &QPushButton::clicked, this, &Faults::onBackBtnPressed);

	QObject::connect(m_timer, &QTimer::timeout, this, &Faults::pollFaults);
	QObject::connect(m_thread, &QThread::started, this, [&]() {
		qDebug(CAT_SWIOT_FAULTS) << "Faults reader thread started";
		m_timer->start(POLLING_INTERVAL);
	});

	QObject::connect(m_tme, &ToolMenuEntry::runToggled, m_runBtn, &QPushButton::setChecked);
}

void Faults::onBackBtnPressed()
{

	m_thread->quit();
	m_thread->wait();
	Q_EMIT backBtnPressed();
}

void Faults::runButtonClicked(bool toggled)
{
	m_singleBtn->setChecked(false);
	if(toggled) {
		m_thread->start();
		if(!m_tme->running()) {
			m_tme->setRunning(true);
		}
	} else {
		if(m_thread->isRunning()) {
			m_thread->quit();
		}
		if(m_tme->running()) {
			m_tme->setRunning(false);
		}
		m_timer->stop();
	}
}

void Faults::singleButtonClicked()
{
	qDebug(CAT_SWIOT_FAULTS) << "Single button clicked";
	m_runBtn->setChecked(false);
	m_timer->stop();
	pollFaults();
	m_singleBtn->setChecked(false);
}

void Faults::startTutorial()
{
	qInfo(CAT_SWIOT) << "Starting faults tutorial.";
	QWidget *parent = Util::findContainingWindow(this);
	gui::TutorialBuilder *m_faultsTutorial =
		new gui::TutorialBuilder(this, ":/swiot/tutorial_chapters.json", "faults", parent);
	m_faultsTutorial->setTitle("FAULTS");
	m_faultsTutorial->start();
}

void Faults::pollFaults()
{
	qDebug(CAT_SWIOT_FAULTS) << "Polling faults...";
	m_faultsPage->update();
}

QPushButton *Faults::createConfigBtn(QWidget *parent)
{
	QPushButton *configBtn = new QPushButton(parent);
	StyleHelper::BlueGrayButton(configBtn, "back_btn");
	configBtn->setFixedWidth(128);
	configBtn->setCheckable(false);
	configBtn->setText("Config");
	return configBtn;
}

void Faults::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);

	if(Preferences::get("faults_start_tutorial").toBool()) {
		startTutorial();
		Preferences::set("faults_start_tutorial", false);
	}
}

void Faults::initTutorialProperties()
{
	// initialize components that might be used for the Faults tutorial
	m_singleBtn->setProperty("tutorial_name", "SINGLE_BUTTON");
	m_runBtn->setProperty("tutorial_name", "RUN_BUTTON");
	m_configBtn->setProperty("tutorial_name", "CONFIG_BUTTON");
}

#include "moc_faults.cpp"
