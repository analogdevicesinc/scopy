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
 *
 */

#include "pingview.h"

#include <QAbstractButton>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <cmath>
#include <component/ping.h>
#include <menuonoffswitch.h>
#include <menusectionwidget.h>
#include <menuspinbox.h>
#include <qcoro/qcorotask.h>
#include <style.h>

using namespace scopy;
using namespace scopy::dcdebugger;

namespace {
const char *const MONITORING_HINT = "Stopping the ping disables automatic disconnect detection.";
const char *const NO_RESULT = "—";

QLabel *sectionTitle(const QString &text, QWidget *parent)
{
	auto *label = new QLabel(text, parent);
	Style::setStyle(label, style::properties::label::menuSmall);
	Style::setStyle(label, style::properties::widget::basicBackground);
	return label;
}

// The global sheet paints plain widgets in the window colour; inside a section every container
// and label takes the section background explicitly.
template <typename T>
T *onSection(T *w)
{
	Style::setStyle(w, style::properties::widget::basicBackground);
	return w;
}

QHBoxLayout *row(QWidget *parent, QVBoxLayout *into, const QString &title, QWidget **container)
{
	auto *w = onSection(new QWidget(parent));
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);
	if(!title.isEmpty()) {
		auto *label = onSection(new QLabel(title, w));
		Style::setStyle(label, style::properties::label::subtle);
		label->setMinimumWidth(90);
		lay->addWidget(label);
	}
	into->addWidget(w);
	if(container) {
		*container = w;
	}
	return lay;
}
} // namespace

PingView::PingView(component::Ping *ping, QWidget *parent)
	: QWidget(parent)
	, m_ping(ping)
{
	auto *viewLay = new QVBoxLayout(this);
	viewLay->setContentsMargins(0, 0, 0, 0);

	QWidget *wrapper = new QWidget(parent);
	auto *lay = new QVBoxLayout(wrapper);
	lay->setContentsMargins(0, 0, 0, 0);
	lay->setSpacing(10);
	wrapper->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_unavailable = new QLabel("The ping is no longer available", wrapper);
	Style::setStyle(m_unavailable, style::properties::label::warning);
	m_unavailable->setVisible(wrapper);
	lay->addWidget(m_unavailable);
	lay->addWidget(buildControlSection(wrapper));
	lay->addWidget(buildStatusSection(wrapper));
	lay->addStretch();
	viewLay->addWidget(wrapper);

	if(m_ping) {
		connect(m_ping, &component::Ping::monitoringChanged, this, &PingView::updateControls);
		connect(m_ping, &component::Ping::intervalChanged, this, &PingView::updateControls);
		connect(m_ping, &component::Ping::resultChanged, this, &PingView::updateStatus);
		connect(m_ping, &QObject::destroyed, this, &PingView::refresh, Qt::QueuedConnection);
	}

	// Both follow the Ping without signal blockers (the switch animates from toggled): a change
	// is only written back when it differs from the Ping, so following it never echoes.
	connect(m_monitoring->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		if(m_ping && m_ping->isMonitoring() != on) {
			m_ping->setMonitoring(on);
		}
		updateControls(); // the Ping may refuse or clamp: show what it did
	});
	// MenuSpinbox commits on Enter / focus out / its +- buttons, not on every keystroke, and
	// clamps to its range.
	connect(m_period, &gui::MenuSpinbox::valueChanged, this, [this](double value) {
		const int ms = int(std::lround(value));
		if(m_ping && m_ping->intervalMs() != ms) {
			m_ping->setIntervalMs(ms);
		}
		updateControls();
	});
	connect(m_pingNowBtn, &QPushButton::clicked, this, &PingView::pingNow);

	refresh();
}

QWidget *PingView::buildControlSection(QWidget *parent)
{
	auto *section = new MenuSectionWidget(parent);
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);
	lay->addWidget(sectionTitle("Control", section));

	QHBoxLayout *monRow = row(section, lay, QString(), nullptr);
	m_monitoring = onSection(new MenuOnOffSwitch("Monitoring", section));
	m_monitoring->setObjectName("pingMonitoring");
	m_monitoring->setToolTip(MONITORING_HINT);
	m_monitoring->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	auto *hint = onSection(new QLabel(MONITORING_HINT, section));
	hint->setObjectName("pingHint");
	Style::setStyle(hint, style::properties::label::subtle);
	monRow->addWidget(m_monitoring);
	monRow->addWidget(hint, 1);

	// Like the monitor's period, but in ms with a fixed step: one "ms" scale (factor 1), so the
	// value is the interval itself and the +- buttons move it by PERIOD_STEP_MS.
	m_period = onSection(new gui::MenuSpinbox("Period", component::Ping::DEFAULT_INTERVAL_MS, "ms", MIN_PERIOD_MS,
						  MAX_PERIOD_MS, true, false, false, section));
	m_period->setObjectName("pingPeriod");
	m_period->setScaleList(QList<gui::UnitPrefix>{{QString(), 1.0}});
	m_period->setScaleRange(1, 1); // setScaleList keeps the scales inside [min, max]: none here
	m_period->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	if(auto *fixed = dynamic_cast<gui::IncrementStrategyFixed *>(m_period->incrementStrategy())) {
		fixed->setK(PERIOD_STEP_MS);
	}
	m_period->setValueSilent(m_ping ? m_ping->intervalMs() : component::Ping::DEFAULT_INTERVAL_MS);
	// Its own tooltip shows the value; the explanation goes on the info icon.
	m_period->setInfoMessage("Period of the reachability check while monitoring");
	lay->addWidget(m_period);

	QHBoxLayout *btnRow = row(section, lay, QString(), nullptr);
	m_pingNowBtn = new QPushButton("Ping now", section);
	m_pingNowBtn->setObjectName("pingNow");
	m_pingNowBtn->setToolTip("Run one reachability check");
	m_pingNowBtn->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	Style::setStyle(m_pingNowBtn, style::properties::button::basicButton);
	btnRow->addWidget(m_pingNowBtn);
	btnRow->addStretch();
	return section;
}

QWidget *PingView::buildStatusSection(QWidget *parent)
{
	auto *section = new MenuSectionWidget(parent);
	QVBoxLayout *lay = section->contentLayout();
	lay->setSpacing(8);
	lay->addWidget(sectionTitle("Status", section));

	QHBoxLayout *resultRow = row(section, lay, "Last result", nullptr);
	m_result = onSection(new QLabel(NO_RESULT, section));
	m_result->setObjectName("pingResult");
	resultRow->addWidget(m_result, 1);

	QHBoxLayout *timeRow = row(section, lay, "Last check", nullptr);
	m_checkedAt = onSection(new QLabel(NO_RESULT, section));
	m_checkedAt->setObjectName("pingCheckedAt");
	timeRow->addWidget(m_checkedAt, 1);

	m_lost = onSection(new QLabel("Connection lost", section));
	m_lost->setObjectName("pingLost");
	Style::setStyle(m_lost, style::properties::label::warning);
	m_lost->setVisible(false);
	lay->addWidget(m_lost);
	return section;
}

void PingView::refresh()
{
	updateControls();
	updateStatus();
}

void PingView::updateControls()
{
	const bool alive = m_ping;
	m_unavailable->setVisible(!alive);
	m_monitoring->setEnabled(alive);
	m_period->setEnabled(alive);
	m_pingNowBtn->setEnabled(alive && !m_checking);
	if(!alive) {
		return;
	}
	m_monitoring->onOffswitch()->setChecked(m_ping->isMonitoring()); // no echo: see the toggled handler
	if(int(std::lround(m_period->value())) != m_ping->intervalMs()) {
		m_period->setValueSilent(m_ping->intervalMs());
	}
}

void PingView::updateStatus()
{
	if(!m_ping || !m_ping->hasResult()) {
		m_result->setText(NO_RESULT);
		m_checkedAt->setText(NO_RESULT);
		Style::setStyle(m_result, style::properties::label::warning, false);
		m_lost->setVisible(m_ping && m_ping->isLost());
		return;
	}
	const bool ok = m_ping->lastReachable();
	m_result->setText(ok ? "Reachable" : "Unreachable");
	Style::setStyle(m_result, style::properties::label::warning, !ok);
	m_checkedAt->setText(m_ping->lastCheckedAt().toString("yyyy-MM-dd HH:mm:ss.zzz"));
	m_lost->setVisible(m_ping->isLost());
}

void PingView::pingNow()
{
	if(!m_ping || m_checking) {
		return;
	}
	// The outcome arrives through resultChanged; the task only gates the button.
	m_checking = true;
	m_pingNowBtn->setEnabled(false);
	QCoro::connect(m_ping->checkReachableAsync(), this, [this](const CommandResponse<void> &) {
		m_checking = false;
		m_pingNowBtn->setEnabled(m_ping);
	});
}

#include "moc_pingview.cpp"
