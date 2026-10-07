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

#include "adalm2000voltmetertool.h"

#include "m2kcomponentbuilder.h"
#include "m2kcontext.h"
#include "m2kvoltmetercontroller.h"
#include "m2kvoltmeterreader.h"

#include <QLabel>
#include <QLoggingCategory>
#include <QPen>
#include <QAbstractButton>
#include <QComboBox>
#include <QTimer>
#include <QPushButton>
#include <QSpacerItem>
#include <QVBoxLayout>

#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuwidget.h>
#include <gui/widgets/valuebarwidget.h>
#include <gui/widgets/valuemonitorwidget.h>
#include <pluginbase/statusbarmanager.h>

#include <component/attribute.h>

#include <qwt_thermo.h>
#include <component/channel.h>
#include <component/context.h>
#include <component/device.h>
#include <component/navigation.h>

Q_LOGGING_CATEGORY(CAT_ADALM2000VOLTMETER, "Adalm2000VoltmeterTool")

using namespace scopy;
using namespace scopy::adalm2000;

Adalm2000VoltmeterTool::Adalm2000VoltmeterTool(ToolMenuEntry *tme, component::Context *ctx, IIOWidgetGroup *group,
					       const QString &uri, QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
	, m_ctx(ctx)
	, m_group(group)
	, m_uri(uri)
{
	setupUi();
}

component::Attribute *Adalm2000VoltmeterTool::findRangeAttribute(int channel) const
{
	if(!m_ctx) {
		qWarning(CAT_ADALM2000VOLTMETER) << "No context provided";
		return nullptr;
	}

	auto *dev = m_ctx->findChild<component::Device *>("analog-in", Qt::FindDirectChildrenOnly);
	if(!dev) {
		qWarning(CAT_ADALM2000VOLTMETER) << "analog-in device not found";
		return nullptr;
	}

	const QString id = QStringLiteral("voltage%1").arg(channel);
	component::Channel *chn = component::channelById(dev, id, /*isOutput=*/false);
	if(!chn) {
		qWarning(CAT_ADALM2000VOLTMETER) << "analog-in input channel" << id << "not found";
		return nullptr;
	}

	component::Attribute *attr = component::attributeByName(chn, "range");
	if(!attr) {
		qWarning(CAT_ADALM2000VOLTMETER) << id << "has no range attribute";
	}
	return attr;
}

QWidget *Adalm2000VoltmeterTool::createChannelSection(int channel, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		QStringLiteral("Channel %1").arg(channel + 1), MenuCollapseSection::MHCW_ARROW,
		MenuCollapseSection::MHW_BASEWIDGET, parent);

	const QColor color = railColor(channel);

	m_monitor[channel] = new ValueMonitorWidget(QStringLiteral("Channel %1").arg(channel + 1), "VDC", QString(), 3,
						    color, true, section);
	section->contentLayout()->addWidget(m_monitor[channel]);

	m_scale[channel] = new ValueBarWidget(section);
	m_scale[channel]->setOrientation(Qt::Horizontal);
	m_scale[channel]->setBarColor(color);
	m_autoScaler[channel].setPresets(vm::presets());
	m_autoScaler[channel].setFloorAtZero(m_acMode[channel]);
	applyScalePreset(channel, m_autoScaler[channel].index());
	section->contentLayout()->addWidget(m_scale[channel]);

	return section;
}

void Adalm2000VoltmeterTool::applyScalePreset(int channel, int index)
{
	if(channel < 0 || channel > 1 || index < 0 || !m_scale[channel]) {
		return;
	}
	const ScalePreset preset = m_autoScaler[channel].current();
	m_scale[channel]->setRange(preset.lower, preset.upper);
	m_scale[channel]->setTickCounts(preset.maxMajor, preset.maxMinor);
}

void Adalm2000VoltmeterTool::setupUi()
{
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	m_tool = new ToolTemplate(this);
	m_tool->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	m_tool->topContainer()->setVisible(true);
	m_tool->topContainerMenuControl()->setVisible(false);
	m_tool->bottomContainer()->setVisible(false);
	m_tool->leftContainer()->setVisible(false);
	m_tool->rightContainer()->setVisible(true);
	m_tool->centralContainer()->setVisible(true);
	m_tool->setRightContainerWidth(300);
	mainLayout->addWidget(m_tool);

	m_runBtn = new RunBtn(this);
	m_tool->addWidgetToTopContainerHelper(m_runBtn, TTA_RIGHT);
	connect(m_runBtn, &QAbstractButton::toggled, m_tme, &ToolMenuEntry::setRunning);
	connect(m_tme, &ToolMenuEntry::runToggled, m_runBtn, &QAbstractButton::setChecked);

	m_gearBtn = new GearBtn(this);
	m_gearBtn->setCheckable(true);
	m_gearBtn->setChecked(true);
	m_tool->addWidgetToTopContainerHelper(m_gearBtn, TTA_RIGHT);

	QWidget *central = new QWidget(this);
	QVBoxLayout *centralLayout = new QVBoxLayout(central);
	centralLayout->setContentsMargins(0, 0, 0, 0);
	for(int c = 0; c < 2; ++c) {
		centralLayout->addWidget(createChannelSection(c, central));
	}
	centralLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	m_tool->addWidgetToCentralContainerHelper(central);

	gui::MenuWidget *settingsMenu =
		new gui::MenuWidget("SETTINGS", QPen(Style::getAttribute(json::theme::interactive_primary_idle)), this);

	for(int c = 0; c < 2; ++c) {
		MenuSectionCollapseWidget *chSection = new MenuSectionCollapseWidget(
			QStringLiteral("Channel %1").arg(c + 1), MenuCollapseSection::MHCW_ARROW,
			MenuCollapseSection::MHW_BASEWIDGET, settingsMenu);

		m_modeCombo[c] = new MenuCombo("Mode", chSection);
		m_modeCombo[c]->combo()->addItem("DC (Direct Current)");
		m_modeCombo[c]->combo()->addItem("AC (20 Hz - 40 kHz)");
		m_modeCombo[c]->combo()->setCurrentIndex(0);
		chSection->contentLayout()->addWidget(m_modeCombo[c]);
		connect(m_modeCombo[c]->combo(), &QComboBox::currentIndexChanged, this,
			[this, c](int idx) { onModeChanged(c, idx); });

		m_gainCombo[c] = new MenuCombo("Gain", chSection);
		m_gainCombo[c]->combo()->addItem("Auto");
		m_gainCombo[c]->combo()->addItem(QString::fromUtf8("\u00b1 25V"));
		m_gainCombo[c]->combo()->addItem(QString::fromUtf8("\u00b1 2.5V"));
		m_gainCombo[c]->combo()->setCurrentIndex(0);
		chSection->contentLayout()->addWidget(m_gainCombo[c]);
		connect(m_gainCombo[c]->combo(), &QComboBox::currentIndexChanged, this,
			[this, c](int idx) { onGainSelected(c, idx); });

		m_activeRangeLabel[c] = new QLabel("Active range: -", chSection);
		chSection->contentLayout()->addWidget(m_activeRangeLabel[c]);

		component::Attribute *rangeAttr = findRangeAttribute(c);
		if(rangeAttr) {
			connect(rangeAttr, &component::Attribute::valueChanged, this,
				[this, c](const QString &v) { m_activeRangeLabel[c]->setText("Active range: " + v); });
			if(rangeAttr->readCapability()) {
				rangeAttr->readCapability()->readAsync();
			}
		} else {
			m_activeRangeLabel[c]->setText("Range attribute unavailable");
		}

		settingsMenu->add(chSection, QStringLiteral("channel%1").arg(c));
	}

	MenuSectionCollapseWidget *peakSection = new MenuSectionCollapseWidget(
		"Peak hold", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, settingsMenu);

	m_peakHoldSwitch = new MenuOnOffSwitch("Show min/max", peakSection);
	m_peakHoldSwitch->onOffswitch()->setChecked(true);
	peakSection->contentLayout()->addWidget(m_peakHoldSwitch);
	connect(m_peakHoldSwitch->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		for(int c = 0; c < 2; ++c) {
			m_monitor[c]->setPeakHoldVisible(on);
		}
	});

	QPushButton *resetPeak = new QPushButton("Reset", peakSection);
	Style::setStyle(resetPeak, style::properties::button::basicButton);
	peakSection->contentLayout()->addWidget(resetPeak);
	connect(resetPeak, &QPushButton::clicked, this, &Adalm2000VoltmeterTool::resetPeakHold);

	settingsMenu->add(peakSection, "peakhold");

	m_tool->rightStack()->add(m_settingsMenuId, settingsMenu);

	connect(m_gearBtn, &QPushButton::toggled, this, [this](bool toggled) {
		if(toggled) {
			m_tool->requestMenu(m_settingsMenuId);
		}
		m_tool->openRightContainerHelper(toggled);
	});

	m_tool->requestMenu(m_settingsMenuId);
	m_tool->openRightContainerHelper(true);

	auto *m2kCtx = qobject_cast<M2kContext *>(m_ctx);
	m_controller = new M2kVoltmeterController(m2kCtx, m_uri, this);
	m_reader = new M2kVoltmeterReader(m2kCtx, this);

	m_scaleTimer = new QTimer(this);
	m_scaleTimer->setInterval(vm::SETTLE_INTERVAL_MS);
	connect(m_scaleTimer, &QTimer::timeout, this, [this]() {
		for(int c = 0; c < 2; ++c) {
			applyScalePreset(c, m_autoScaler[c].settle());
		}
	});

	connect(m_reader, &M2kVoltmeterReader::readingsUpdated, this, &Adalm2000VoltmeterTool::onReadings);
	connect(m_reader, &M2kVoltmeterReader::rangeChangeRequested, this,
		&Adalm2000VoltmeterTool::onRangeChangeRequested);
	connect(m_reader, &M2kVoltmeterReader::finished, this, [this]() { m_runBtn->setChecked(false); });

	connect(m_controller, &M2kVoltmeterController::resourceLost, this, [this]() {
		m_reader->requestStop();
		m_runBtn->setChecked(false);
	});

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		if(!on) {
			m_scaleTimer->stop();
			m_reader->requestStop();
			m_controller->release();
			return;
		}
		for(int c = 0; c < 2; ++c) {
			m_autoScaler[c].resetWindow();
		}
		m_scaleTimer->start();
		startRunning(); // unawaited on purpose -- never waitFor() in a UI slot
	});
}

QCoro::Task<void> Adalm2000VoltmeterTool::startRunning()
{
	if(!co_await m_controller->claimAndConfigure()) {
		qWarning(CAT_ADALM2000VOLTMETER) << "could not claim the ADC";
		StatusBarManager::pushUrgentMessage("Voltmeter: the ADC is in use by another tool");
		m_runBtn->setChecked(false);
		co_return;
	}
	m_reader->resetChannels();
	m_reader->start();
}

void Adalm2000VoltmeterTool::onReadings(int channel, double dcVolts, double acVolts)
{
	if(channel < 0 || channel > 1) {
		return;
	}

	const double shown = m_acMode[channel] ? acVolts : dcVolts;
	m_monitor[channel]->setValue(shown);

	applyScalePreset(channel, m_autoScaler[channel].push(shown));
	m_scale[channel]->setValue(shown);

	if(!m_peakSeen[channel]) {
		m_peakSeen[channel] = true;
		m_peakMin[channel] = shown;
		m_peakMax[channel] = shown;
	} else {
		m_peakMin[channel] = qMin(m_peakMin[channel], shown);
		m_peakMax[channel] = qMax(m_peakMax[channel], shown);
	}
	m_monitor[channel]->setMin(m_peakMin[channel]);
	m_monitor[channel]->setMax(m_peakMax[channel]);
}

void Adalm2000VoltmeterTool::onModeChanged(int channel, int index)
{
	if(channel < 0 || channel > 1) {
		return;
	}
	m_acMode[channel] = (index == 1);
	m_monitor[channel]->setUnit(m_acMode[channel] ? "Vrms" : "VDC");

	m_autoScaler[channel].setFloorAtZero(m_acMode[channel]);
	applyScalePreset(channel, m_autoScaler[channel].index());

	m_peakSeen[channel] = false;
	m_monitor[channel]->setMin(0.0);
	m_monitor[channel]->setMax(0.0);
}

void Adalm2000VoltmeterTool::onGainSelected(int channel, int index)
{
	if(channel < 0 || channel > 1 || !m_reader) {
		return;
	}

	const bool automatic = (index == 0);
	m_reader->setAutoGainEnabled(channel, automatic);
	if(automatic) {
		return; // the DSP's 25-frame unanimity takes it from here
	}
	// index 1 = +/-25V, index 2 = +/-2.5V
	writeRange(channel, index == 2);
}

void Adalm2000VoltmeterTool::writeRange(int channel, bool lowRange)
{
	component::Attribute *attr = findRangeAttribute(channel);
	if(!attr || !attr->writeCapability()) {
		StatusBarManager::pushMessage(
			QStringLiteral("Voltmeter: channel %1 range is not writable").arg(channel + 1));
		return;
	}
	attr->writeCapability()->writeAsync(lowRange ? QLatin1String(PLUS_2_5V) : QLatin1String(PLUS_25V));

	if(m_reader) {
		m_reader->resetChannels();
	}
}

void Adalm2000VoltmeterTool::resetPeakHold()
{
	for(int c = 0; c < 2; ++c) {
		m_peakSeen[c] = false;
		m_peakMin[c] = 0.0;
		m_peakMax[c] = 0.0;
		m_monitor[c]->setMin(0.0);
		m_monitor[c]->setMax(0.0);
	}
}

void Adalm2000VoltmeterTool::onRangeChangeRequested(int channel, bool lowRange)
{
	if(channel < 0 || channel > 1) {
		return;
	}
	const int want = lowRange ? 1 : 0;
	if(m_lastRangeLow[channel] == want) {
		return; // no change, nothing to write
	}
	m_lastRangeLow[channel] = want;

	writeRange(channel, lowRange);
}

#include "moc_adalm2000voltmetertool.cpp"
