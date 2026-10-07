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

#include "adalm2000powersupplytool.h"

#include "m2kpowersupplycontroller.h"

#include <QComboBox>
#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QPen>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSlider>
#include <QSpacerItem>
#include <QVBoxLayout>
#include <QtGlobal>

#include <gui/customSwitch.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/menuwidget.h>
#include <gui/widgets/valuebarwidget.h>
#include <gui/widgets/valuemonitorwidget.h>
#include <iio-widgets/iiowidgetbuilder.h>
#include <pluginbase/statusbarmanager.h>
#include <style.h>

#include <component/attribute.h>

#include <qwt_thermo.h>

Q_LOGGING_CATEGORY(CAT_ADALM2000POWERSUPPLY, "Adalm2000PowerSupplyTool")

using namespace scopy;
using namespace scopy::adalm2000;

namespace {
QString railName(int channel)
{
	return channel == 0 ? QStringLiteral("Positive output") : QStringLiteral("Negative output");
}
QColor railColor(int channel) { return Style::getColor(channel == 0 ? json::global::ch0 : json::global::ch1); }
} // namespace

Adalm2000PowerSupplyTool::Adalm2000PowerSupplyTool(component::Context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
{
	m_controller = new M2kPowerSupplyController(m_ctx, this);
	setupUi();

	connect(m_controller, &M2kPowerSupplyController::railMeasured, this, &Adalm2000PowerSupplyTool::onMeasured);
	connect(m_controller, &M2kPowerSupplyController::failed, this,
		[](const QString &msg) { StatusBarManager::pushMessage(msg, 4000); });

	if(!m_controller->isUsable()) {
		StatusBarManager::pushUrgentMessage("Power Supply: this device exposes no usable rails");
		for(int ch = 0; ch < 2; ++ch) {
			m_enableBtn[ch]->setEnabled(false);
		}
		return;
	}

	m_controller->initialize();
}

Adalm2000PowerSupplyTool::~Adalm2000PowerSupplyTool()
{
	if(m_controller) {
		m_controller->shutdown();
	}
}

void Adalm2000PowerSupplyTool::showEvent(QShowEvent *event)
{
	QWidget::showEvent(event);
	if(m_controller) {
		m_controller->startPolling();
	}
}

void Adalm2000PowerSupplyTool::hideEvent(QHideEvent *event)
{
	if(m_controller) {
		m_controller->stopPolling();
	}
	QWidget::hideEvent(event);
}

QWidget *Adalm2000PowerSupplyTool::createRailReadout(int channel, QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		railName(channel), MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *row = new QWidget(section);
	QVBoxLayout *rowLayout = new QVBoxLayout(row);
	rowLayout->setContentsMargins(0, 0, 0, 0);

	const QColor color = railColor(channel);

	m_setMonitor[channel] = new ValueMonitorWidget(QString(), "VDC", "Set", 3, color, false, row);
	rowLayout->addWidget(m_setMonitor[channel]);

	m_measuredMonitor[channel] = new ValueMonitorWidget(QString(), "VDC", "Measure", 3, color, false, row);
	rowLayout->addWidget(m_measuredMonitor[channel]);

	section->contentLayout()->addWidget(row);

	const bool positive = (channel == 0);
	m_scale[channel] = new ValueBarWidget(section);
	m_scale[channel]->setOrientation(Qt::Horizontal);
	m_scale[channel]->setTickCounts(m_scale[channel]->scaleMaxMajor(), 10);
	m_scale[channel]->setBarColor(color);
	m_scale[channel]->setRange(positive ? ps::POSITIVE_MIN : ps::NEGATIVE_MIN,
				   positive ? ps::POSITIVE_MAX : ps::NEGATIVE_MAX);
	section->contentLayout()->addWidget(m_scale[channel]);

	return section;
}

void Adalm2000PowerSupplyTool::applyVoltageScale(IIOWidget *widget, int channel)
{
	auto *spin = widget->findChild<gui::MenuSpinbox *>();
	if(!spin) {
		qWarning(CAT_ADALM2000POWERSUPPLY) << "no spinbox inside the voltage IIOWidget for rail" << channel;
		return;
	}

	spin->setScalingEnabled(true);
	// Must stay ascending by scale: the prefix scan walks the combo backwards.
	spin->setScaleList({{QStringLiteral("mVolts"), 1e-3}, {QStringLiteral("Volts"), 1e0}});

	// These are prefix SCALES, not value bounds, despite setScaleRange's parameter names.
	spin->setScaleRange(1e-3, 1e0);

	// The refill leaves mVolts selected, so a bare "5" would mean 5 mV.
	if(auto *scaleCombo = spin->findChild<QComboBox *>()) {
		const int voltsIndex = scaleCombo->findData(1e0);
		if(voltsIndex >= 0) {
			scaleCombo->setCurrentIndex(voltsIndex);
		}
	}

	const bool positive = (channel == 0);
	spin->setMinValue(positive ? ps::POSITIVE_MIN : ps::NEGATIVE_MIN);
	spin->setMaxValue(positive ? ps::POSITIVE_MAX : ps::NEGATIVE_MAX);
	spin->enableRangeLimits(true);
}

void Adalm2000PowerSupplyTool::createRailSettings(int channel, QWidget *parent)
{
	auto *menu = qobject_cast<gui::MenuWidget *>(parent);
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		railName(channel), MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	component::Attribute *attr = m_controller->voltageAttribute(channel);
	if(!attr) {
		section->contentLayout()->addWidget(new QLabel("Rail unavailable", section));
	} else {
		m_voltageWidget[channel] =
			IIOWidgetBuilder(section).attribute(attr).title(railName(channel)).buildSingle();
		if(m_voltageWidget[channel]) {
			if(m_group) {
				m_group->add(m_voltageWidget[channel]);
			}
			applyVoltageScale(m_voltageWidget[channel], channel);
			section->contentLayout()->addWidget(m_voltageWidget[channel]);
		}

		connect(attr, &component::Attribute::valueChanged, this,
			[this, channel](const QString &v) { onSetpointChanged(channel, v.toDouble()); });
		if(attr->readCapability()) {
			attr->readCapability()->readAsync();
		}
	}

	m_enableBtn[channel] = new QPushButton("Enable", section);
	m_enableBtn[channel]->setCheckable(true);
	Style::setStyle(m_enableBtn[channel], style::properties::button::runButton);
	section->contentLayout()->addWidget(m_enableBtn[channel]);
	connect(m_enableBtn[channel], &QPushButton::toggled, this,
		[this, channel](bool on) { onEnableToggled(channel, on); });

	if(menu) {
		menu->add(section, QStringLiteral("rail%1").arg(channel));
	}
}

void Adalm2000PowerSupplyTool::setupUi()
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

	m_gearBtn = new GearBtn(this);
	m_gearBtn->setCheckable(true);
	m_gearBtn->setChecked(true);
	m_tool->addWidgetToTopContainerHelper(m_gearBtn, TTA_RIGHT);

	QWidget *central = new QWidget(this);
	QVBoxLayout *centralLayout = new QVBoxLayout(central);
	centralLayout->setContentsMargins(0, 0, 0, 0);
	for(int ch = 0; ch < 2; ++ch) {
		centralLayout->addWidget(createRailReadout(ch, central));
	}
	centralLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
	m_tool->addWidgetToCentralContainerHelper(central);

	gui::MenuWidget *settingsMenu =
		new gui::MenuWidget("SETTINGS", QPen(Style::getAttribute(json::theme::interactive_primary_idle)), this);

	MenuSectionCollapseWidget *trackingSection = new MenuSectionCollapseWidget(
		"Tracking", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, settingsMenu);

	m_modeSwitch = new CustomSwitch("Independent", "Tracking", trackingSection);
	m_modeSwitch->setChecked(true);
	trackingSection->contentLayout()->addWidget(m_modeSwitch);
	connect(m_modeSwitch, &QAbstractButton::toggled, this,
		[this](bool independent) { onTrackingModeChanged(!independent); });

	m_ratioLabel = new QLabel(QStringLiteral("Ratio: %1%").arg(ps::RATIO_DEFAULT), trackingSection);
	trackingSection->contentLayout()->addWidget(m_ratioLabel);

	m_ratioSlider = new QSlider(Qt::Horizontal, trackingSection);
	m_ratioSlider->setRange(ps::RATIO_MIN, ps::RATIO_MAX);
	m_ratioSlider->setValue(ps::RATIO_DEFAULT);
	m_ratioSlider->setEnabled(false);
	trackingSection->contentLayout()->addWidget(m_ratioSlider);
	connect(m_ratioSlider, &QSlider::valueChanged, this, &Adalm2000PowerSupplyTool::onRatioChanged);

	settingsMenu->add(trackingSection, "tracking");

	for(int ch = 0; ch < 2; ++ch) {
		createRailSettings(ch, settingsMenu);
	}

	m_tool->rightStack()->add(m_settingsMenuId, settingsMenu);

	connect(m_gearBtn, &QPushButton::toggled, this, [this](bool toggled) {
		if(toggled) {
			m_tool->requestMenu(m_settingsMenuId);
		}
		m_tool->openRightContainerHelper(toggled);
	});
	m_tool->requestMenu(m_settingsMenuId);
	m_tool->openRightContainerHelper(true);
}

void Adalm2000PowerSupplyTool::onMeasured(int channel, double volts)
{
	if(channel < 0 || channel > 1) {
		return;
	}
	const double average = m_average[channel].push(volts);
	m_measuredMonitor[channel]->setValue(average);
	m_scale[channel]->setValue(average);
}

void Adalm2000PowerSupplyTool::onSetpointChanged(int channel, double volts)
{
	if(channel < 0 || channel > 1) {
		return;
	}
	m_setpoint[channel] = volts;
	m_setMonitor[channel]->setValue(volts);
	m_average[channel].clear();

	if(channel == 0 && m_tracking) {
		applyTracking();
	}
}

void Adalm2000PowerSupplyTool::onEnableToggled(int channel, bool on)
{
	if(channel < 0 || channel > 1) {
		return;
	}
	m_railOn[channel] = on;
	m_enableBtn[channel]->setText(on ? "Disable" : "Enable");
	m_average[channel].clear();
	m_controller->setRailEnabled(channel, on);

	if(channel == 0 && m_tracking) {
		QSignalBlocker blocker(m_enableBtn[1]);
		m_enableBtn[1]->setChecked(on);
		m_enableBtn[1]->setText(on ? "Disable" : "Enable");
		m_railOn[1] = on;
		m_average[1].clear();
		m_controller->setRailEnabled(1, on);
	}
}

void Adalm2000PowerSupplyTool::onTrackingModeChanged(bool tracking)
{
	m_tracking = tracking;

	m_ratioSlider->setEnabled(m_tracking);
	if(m_voltageWidget[1]) {
		m_voltageWidget[1]->setEnabled(!m_tracking);
	}
	m_enableBtn[1]->setEnabled(!m_tracking);

	if(m_railOn[0]) {
		const bool want = m_tracking;
		if(m_railOn[1] != want) {
			QSignalBlocker blocker(m_enableBtn[1]);
			m_enableBtn[1]->setChecked(want);
			m_enableBtn[1]->setText(want ? "Disable" : "Enable");
			m_railOn[1] = want;
			m_average[1].clear();
			m_controller->setRailEnabled(1, want);
		}
	}

	if(m_tracking) {
		applyTracking();
	}
}

void Adalm2000PowerSupplyTool::onRatioChanged(int percent)
{
	m_ratioLabel->setText(QStringLiteral("Ratio: %1%").arg(percent));
	if(m_tracking) {
		applyTracking();
	}
}

void Adalm2000PowerSupplyTool::applyTracking()
{
	const double negative = ps::trackingNegative(m_setpoint[0], m_ratioSlider->value());
	m_controller->setRailVoltage(1, negative);
}

// Must stay out of a shared moc TU with the m2k* mocs: iioutil/command.h and
// core/command.h each declare scopy::Command behind independent include guards.
#include "moc_adalm2000powersupplytool.cpp"
