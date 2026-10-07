/*
 * Copyright (c) 2026 Analog Devices Inc.
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

#include "adalm2000digitaliotool.h"

#include "diogroupwidget.h"
#include "digitalio_api.h"
#include "m2kdigitaliocontroller.h"

#include <QDesktopServices>
#include <QLoggingCategory>
#include <QSignalBlocker>
#include <QUrl>
#include <QVBoxLayout>

#include <gui/style.h>
#include <pluginbase/scopyjs.h>
#include <pluginbase/statusbarmanager.h>

Q_LOGGING_CATEGORY(CAT_ADALM2000DIGITALIO, "Adalm2000DigitalIoTool")

using namespace scopy;
using namespace scopy::adalm2000;

Adalm2000DigitalIoTool::Adalm2000DigitalIoTool(ToolMenuEntry *tme, component::Context *ctx, IIOWidgetGroup *group,
					       QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
	, m_ctx(ctx)
	, m_group(group)
{
	// Must precede setupUi(): the group and pin widgets ask the controller for the
	// attributes they bind to.
	m_controller = new M2kDigitalIoController(ctx, this);

	setupUi();

	connect(m_controller, &M2kDigitalIoController::pinStatesUpdated, this, [this](quint16 gpi, quint16 shorted) {
		for(auto *g : m_groups) {
			if(g) {
				g->setInputState(gpi, shorted);
			}
		}
	});

	connect(m_controller, &M2kDigitalIoController::failed, this,
		[](const QString &msg) { StatusBarManager::pushMessage(msg, 4000); });

	if(!m_controller->isUsable()) {
		qWarning(CAT_ADALM2000DIGITALIO) << "the digital device is incomplete; the tool will be idle";
		StatusBarManager::pushMessage(QStringLiteral("Digital I/O: the digital device is unavailable"), 4000);
		if(m_runBtn) {
			m_runBtn->setEnabled(false);
		}
		return;
	}

	// Unawaited: never QCoro::waitFor() in a constructor or a UI slot.
	m_controller->initialize();

	// `dio` is the global name existing scripts are written against.
	m_api = new DigitalIO_API(this);
	m_api->setObjectName(QStringLiteral("dio"));
	ScopyJS::GetInstance()->registerApi(m_api);
}

Adalm2000DigitalIoTool::~Adalm2000DigitalIoTool()
{
	if(m_api) {
		ScopyJS::GetInstance()->unregisterApi(m_api);
		delete m_api;
		m_api = nullptr;
	}
	if(m_controller) {
		m_controller->shutdown();
	}
}

void Adalm2000DigitalIoTool::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->bottomContainer()->setVisible(false);
	m_tool->leftContainer()->setVisible(false);
	m_tool->rightContainer()->setVisible(false);
	m_tool->centralContainer()->setVisible(true);
	mainLayout->addWidget(m_tool);

	InfoBtn *infoBtn = new InfoBtn(this);
	connect(infoBtn, &QAbstractButton::clicked, this, []() {
		QDesktopServices::openUrl(
			QUrl(QStringLiteral("https://analogdevicesinc.github.io/scopy/plugins/m2k/digitalio.html")));
	});
	m_tool->addWidgetToTopContainerHelper(infoBtn, TTA_LEFT);

	m_runBtn = new RunBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		applyOutputEnabled(on); // unawaited: never waitFor() in a UI slot
	});
	connect(m_runBtn, &QAbstractButton::toggled, m_tme, &ToolMenuEntry::setRunning);
	connect(m_tme, &ToolMenuEntry::runToggled, m_runBtn, &QAbstractButton::setChecked);

	QWidget *central = new QWidget(this);
	QVBoxLayout *centralLayout = new QVBoxLayout(central);
	centralLayout->setContentsMargins(0, 0, 0, 0);
	centralLayout->setSpacing(Style::getDimension(json::global::unit_2));
	for(int g = 0; g < dio::GROUP_COUNT; ++g) {
		m_groups[g] = new DioGroupWidget(m_controller, m_group, g, central);
		centralLayout->addWidget(m_groups[g]);
	}
	centralLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	m_tool->addWidgetToCentralContainerHelper(central);
}

QCoro::Task<void> Adalm2000DigitalIoTool::applyOutputEnabled(bool on)
{
	if(!m_controller) {
		co_return;
	}
	const bool ok = co_await m_controller->setOutputEnabled(on);
	if(!ok && on && m_runBtn) {
		m_runBtn->setChecked(false);
	}
}

void Adalm2000DigitalIoTool::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	if(m_controller) {
		m_controller->startPolling();
	}
}

void Adalm2000DigitalIoTool::hideEvent(QHideEvent *event)
{
	if(m_controller) {
		m_controller->stopPolling();
	}
	QWidget::hideEvent(event);
}

DioGroupWidget *Adalm2000DigitalIoTool::groupWidget(int group) const
{
	return dio::validGroup(group) ? m_groups[group] : nullptr;
}

// The hardware's state, not the button's: the two differ while a write is in flight.
bool Adalm2000DigitalIoTool::isRunning() const { return m_controller && m_controller->outputEnabled(); }

void Adalm2000DigitalIoTool::setRunningBlocking(bool running)
{
	if(!m_controller) {
		return;
	}
	const bool ok = QCoro::waitFor(m_controller->setOutputEnabled(running));
	if(m_runBtn) {
		// Blocked so the button does not fire the async path over this write. That
		// also suppresses the menu-entry sync, so set it explicitly below.
		QSignalBlocker blocker(m_runBtn);
		const bool on = ok && running;
		m_runBtn->setChecked(on);
		m_runBtn->setText(on ? "Stop" : "Run");
		m_tme->setRunning(on);
	}
}

#include "moc_adalm2000digitaliotool.cpp"
