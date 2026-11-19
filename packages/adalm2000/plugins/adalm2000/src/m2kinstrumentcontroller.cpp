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

#include "m2kinstrumentcontroller.h"

#include "m2kcalibration.h"
#include "m2kcontext.h"

#include <component/ping.h>
#include <core/acq_engine/acquisitionengine.h>
#include <core/acq_ui/acqinstrument.h>
#include <gui/instrumenttemplate.h>
#include <gui/semiexclusivebuttongroup.h>
#include <gui/style.h>
#include <gui/widgets/hoverwidget.h>
#include <gui/widgets/toolbuttons.h>
#include <pluginbase/statusbarmanager.h>

#include <QPushButton>

using namespace scopy;
using namespace scopy::adalm2000;

namespace {
constexpr int kMaxFps = 60;
} // namespace

M2kInstrumentController::M2kInstrumentController(ToolMenuEntry *tme, M2kContext *ctx, const QString &resourceKey,
						 const QString &title, QObject *parent)
	: AcqInstrumentController(tme, parent)
	, m_ctx(ctx)
	, m_resourceKey(resourceKey)
	, m_title(title)
{
	m_ui->setRunGuard([this]() { return claim(); });
	m_ui->setBufferControlVisible(false);
	m_ui->engine()->setMaxFPS(kMaxFps);
	m_ui->shell()->setTargetFps(kMaxFps);
	connect(m_ui, &scopy::adc::AcqInstrument::stopped, this, [this]() {
		if(!m_ui->engine()->isRunning()) {
			release();
		}
	});
}

M2kInstrumentController::~M2kInstrumentController()
{
	AcqInstrumentController::stop();
	release();
}

void M2kInstrumentController::stop()
{
	m_claimed = false;
	AcqInstrumentController::stop();
}

void M2kInstrumentController::setupCalibrationGate()
{
	auto *calibration = m_ctx ? m_ctx->findChild<M2kCalibration *>() : nullptr;
	if(!calibration) {
		return;
	}
	auto gate = [this](bool calibrating) {
		m_calibrating = calibrating;
		if(calibrating) {
			AcqInstrumentController::stop();
		}
		updateRunEnabled();
	};
	connect(calibration, &M2kCalibration::started, this, [gate]() { gate(true); });
	connect(calibration, &M2kCalibration::finished, this, [gate](bool) { gate(false); });
	gate(calibration->isRunning());
}

void M2kInstrumentController::updateRunEnabled()
{
	m_ui->shell()->runBtn()->setEnabled(!m_calibrating);
	m_ui->shell()->singleBtn()->setEnabled(!m_calibrating);
	if(m_tme) {
		m_tme->setRunEnabled(!m_calibrating);
	}
}

void M2kInstrumentController::addHoverButton(const QString &label, QWidget *content)
{
	InstrumentTemplate *it = m_ui->shell();
	auto *btn = new QPushButton(label, it);
	btn->setCheckable(true);
	Style::setStyle(btn, style::properties::button::blueGrayButton);
	Style::setStyle(btn, style::properties::label::menuMedium);
	it->tool()->addWidgetToBottomContainerHelper(btn, TTA_RIGHT);
	if(!m_hoverGroup) {
		m_hoverGroup = new SemiExclusiveButtonGroup(this);
	}
	m_hoverGroup->addButton(btn);

	content->setMinimumWidth(Style::getDimension(json::global::unit_6) * 2 +
				 Style::getDimension(json::global::unit_4));
	Style::setBackgroundColor(content, json::theme::background_primary);
	auto *hover = new HoverWidget(content, btn, it);
	hover->setStyleSheet(QStringLiteral("scopy--HoverWidget { background-color: transparent; }"));
	hover->setAnchorPos(HP_TOPRIGHT);
	hover->setContentPos(HP_TOPLEFT);
	hover->setVisible(false);
	connect(btn, &QAbstractButton::toggled, hover, [hover](bool on) {
		hover->setVisible(on);
		hover->raise();
	});
}

bool M2kInstrumentController::claim()
{
	if(m_calibrating) {
		StatusBarManager::pushMessage(tr("%1: wait for calibration to finish").arg(m_title));
		return false;
	}
	if(!m_claimed) {
		ResourceManager::open(m_resourceKey, this, true);
		m_claimed = true;
	}
	if(auto *ping = m_ctx ? m_ctx->findChild<component::Ping *>(QString(), Qt::FindDirectChildrenOnly) : nullptr) {
		if(ping->isMonitoring()) {
			ping->stopMonitoring();
			m_pingPaused = true;
		}
	}
	return true;
}

void M2kInstrumentController::release()
{
	if(m_claimed) {
		m_claimed = false;
		ResourceManager::close(m_resourceKey);
	}
	if(m_pingPaused && m_ctx) {
		m_pingPaused = false;
		if(auto *ping = m_ctx->findChild<component::Ping *>(QString(), Qt::FindDirectChildrenOnly)) {
			ping->startMonitoring();
		}
	}
}

#include "moc_m2kinstrumentcontroller.cpp"
