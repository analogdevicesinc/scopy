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

#include "adalm2000voltmeter_api.h"
#include "m2kcalibration.h"
#include "m2kcomponentbuilder.h"
#include "m2kcontext.h"
#include "m2kvoltmetercsvlogger.h"
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

#include <gui/plotautoscaler.h>
#include <gui/plotaxis.h>
#include <gui/plotchannel.h>
#include <gui/plotnavigator.hpp>
#include <gui/plotwidget.h>
#include <gui/widgets/filebrowserwidget.h>
#include <gui/widgets/menucombo.h>
#include <gui/widgets/menuonoffswitch.h>
#include <gui/widgets/menuplotaxisrangecontrol.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuspinbox.h>
#include <gui/widgets/menuwidget.h>
#include <gui/widgets/valuebarwidget.h>
#include <gui/widgets/valuemonitorwidget.h>
#include <pluginbase/scopyjs.h>
#include <pluginbase/statusbarmanager.h>

#include <component/attribute.h>

#include <qwt_thermo.h>
#include <component/channel.h>
#include <component/context.h>
#include <component/device.h>
#include <component/navigation.h>

Q_LOGGING_CATEGORY(CAT_ADALM2000VOLTMETER, "Adalm2000VoltmeterTool")

constexpr int PLOT_MIN_HEIGHT_PX = 120;

using namespace scopy;
using namespace scopy::adalm2000;

Adalm2000VoltmeterTool::Adalm2000VoltmeterTool(ToolMenuEntry *tme, component::Context *ctx, const QString &uri,
					       QWidget *parent)
	: QWidget(parent)
	, m_tme(tme)
	, m_ctx(ctx)
	, m_uri(uri)
{
	setupUi();

	m_logger = new M2kVoltmeterCsvLogger(this);

	m_api = new Adalm2000Voltmeter_API(this);
	m_api->setObjectName("dmm");
	ScopyJS::GetInstance()->registerApi(m_api);
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

	QWidget *sectionWidget = new QWidget(section);
	QHBoxLayout *sectionLayout = new QHBoxLayout(sectionWidget);
	sectionLayout->setContentsMargins(0, 0, 0, 0);
	sectionWidget->setLayout(sectionLayout);

	section->contentLayout()->addWidget(sectionWidget);

	const QColor color = railColor(channel);

	QVBoxLayout *valuesLayout = new QVBoxLayout();
	valuesLayout->setContentsMargins(0, 0, 0, 0);

	m_monitor[channel] = new ValueMonitorWidget(QStringLiteral("Channel %1").arg(channel + 1), "VDC", QString(), 3,
						    color, true, section);
	valuesLayout->addWidget(m_monitor[channel]);

	m_scale[channel] = new ValueBarWidget(section);
	m_scale[channel]->setOrientation(Qt::Horizontal);
	m_scale[channel]->setBarColor(color);
	m_autoScaler[channel].setPresets(vm::presets());
	m_autoScaler[channel].setFloorAtZero(m_acMode[channel]);
	applyScalePreset(channel, m_autoScaler[channel].index());
	valuesLayout->addWidget(m_scale[channel]);

	createChannelPlot(channel, section);
	sectionLayout->addWidget(m_plot[channel]);
	sectionLayout->addLayout(valuesLayout);

	section->menuSection()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	section->collapseSection()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

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

void Adalm2000VoltmeterTool::createChannelPlot(int channel, QWidget *parent)
{
	const QColor color = railColor(channel);

	m_plot[channel] = new PlotWidget(parent);
	m_plot[channel]->navigator()->setEnabled(false);
	m_plot[channel]->setShowXAxisLabels(true);
	m_plot[channel]->setShowYAxisLabels(true);
	m_plot[channel]->setMinimumHeight(PLOT_MIN_HEIGHT_PX);
	m_plot[channel]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_plot[channel]->xAxis()->setUnits("s");
	m_plot[channel]->xAxis()->setInterval(-vm::PLOT_HISTORY_DEFAULT_S, 0);
	m_plot[channel]->yAxis()->setUnits("V");
	m_plot[channel]->yAxis()->scaleDraw()->setFloatPrecision(2);
	m_plot[channel]->yAxis()->setInterval(-vm::PLOT_MIN_SPAN_V, vm::PLOT_MIN_SPAN_V);
	m_plot[channel]->yAxis()->setDivs(5);
	m_plot[channel]->yAxis()->scaleEngine()->setMajorTicksCount(5);

	m_plotCh[channel] = new PlotChannel(QStringLiteral("Channel %1").arg(channel + 1), QPen(color, 1),
					    m_plot[channel]->xAxis(), m_plot[channel]->yAxis(), m_plot[channel]);
	m_plot[channel]->addPlotChannel(m_plotCh[channel]);
	m_plotCh[channel]->setEnabled(true);
	m_plot[channel]->setUnitsVisible(true);

	m_plotAutoscaler[channel] = new gui::PlotAutoscaler(this);
	m_plotAutoscaler[channel]->addChannels(m_plotCh[channel]);
	connect(m_plotAutoscaler[channel], &gui::PlotAutoscaler::newMin, this,
		[this, channel](double v) { m_pendingYMin[channel] = v; });
	connect(m_plotAutoscaler[channel], &gui::PlotAutoscaler::newMax, this,
		[this, channel](double v) { applyPlotYRange(channel, m_pendingYMin[channel], v); });

	setPlotHistory(channel, vm::PLOT_HISTORY_DEFAULT_S);
}

QWidget *Adalm2000VoltmeterTool::createPlotSettings(int channel, QWidget *parent)
{
	MenuCollapseSection *plotSection = new MenuCollapseSection("Plot", MenuCollapseSection::MHCW_ARROW,
								   MenuCollapseSection::MHW_BASEWIDGET, parent);
	plotSection->contentLayout()->setSpacing(10);

	m_plotVisibleSwitch[channel] = new MenuOnOffSwitch("Show plot", plotSection);
	m_plotVisibleSwitch[channel]->onOffswitch()->setChecked(true);
	connect(m_plotVisibleSwitch[channel]->onOffswitch(), &QAbstractButton::toggled, this,
		[this, channel](bool on) { setPlotVisible(channel, on); });

	m_historySpin[channel] =
		new gui::MenuSpinbox("History", vm::PLOT_HISTORY_DEFAULT_S, "s", vm::PLOT_HISTORY_MIN_S,
				     vm::PLOT_HISTORY_MAX_S, true, false, true, plotSection);
	m_historySpin[channel]->setIncrementMode(gui::MenuSpinbox::IS_FIXED);
	m_historySpin[channel]->setScalingEnabled(false);
	connect(m_historySpin[channel], &gui::MenuSpinbox::valueChanged, this,
		[this, channel](double v) { setPlotHistory(channel, v); });

	m_plotYRange[channel] = new gui::MenuPlotAxisRangeControl(m_plot[channel]->yAxis(), plotSection);
	m_plotYRange[channel]->minSpinbox()->setUnit("V");
	m_plotYRange[channel]->maxSpinbox()->setUnit("V");
	m_plotYRange[channel]->setMin(-vm::PLOT_MIN_SPAN_V);
	m_plotYRange[channel]->setMax(vm::PLOT_MIN_SPAN_V);

	m_autoscaleSwitch[channel] = new MenuOnOffSwitch("Autoscale", plotSection);
	connect(m_autoscaleSwitch[channel]->onOffswitch(), &QAbstractButton::toggled, this,
		[this, channel](bool on) { setPlotAutoscaleEnabled(channel, on); });

	plotSection->contentLayout()->addWidget(m_plotVisibleSwitch[channel]);
	plotSection->contentLayout()->addWidget(m_historySpin[channel]);
	plotSection->contentLayout()->addWidget(m_autoscaleSwitch[channel]);
	plotSection->contentLayout()->addWidget(m_plotYRange[channel]);

	m_autoscaleSwitch[channel]->onOffswitch()->setChecked(true);
	plotSection->header()->setChecked(false);

	return plotSection;
}

void Adalm2000VoltmeterTool::repointPlotCurve(int channel)
{
	if(!m_plotCh[channel] || !m_plotCh[channel]->curve()) {
		return;
	}
	const QVector<double> &x = m_histX[channel];
	const QVector<double> &y = m_histY[channel];
	m_plotCh[channel]->curve()->setRawSamples(x.constData() + (x.size() - y.size()), y.constData(),
						  static_cast<int>(y.size()));
}

void Adalm2000VoltmeterTool::setPlotHistory(int channel, double seconds)
{
	if(channel < 0 || channel > 1 || !m_plot[channel]) {
		return;
	}

	const int cap = vm::historyCapacity(seconds);

	m_histY[channel].clear();
	m_histY[channel].reserve(cap);
	m_histX[channel].clear();
	m_histX[channel].reserve(cap);
	for(int i = cap - 1; i >= 0; --i) {
		m_histX[channel].append(-i / dsp::UPDATE_RATE_HZ);
	}
	m_histCap[channel] = cap;

	repointPlotCurve(channel);

	m_plot[channel]->xAxis()->setInterval(m_histX[channel].first(), 0);
	m_plot[channel]->replot();
}

void Adalm2000VoltmeterTool::clearPlotHistory(int channel)
{
	if(channel < 0 || channel > 1 || !m_plot[channel]) {
		return;
	}
	m_histY[channel].clear();
	repointPlotCurve(channel);
	m_plot[channel]->replot();
}

void Adalm2000VoltmeterTool::pushPlotSample(int channel, double value)
{
	if(channel < 0 || channel > 1 || !m_plot[channel]) {
		return;
	}

	QVector<double> &y = m_histY[channel];
	y.append(value);
	while(y.size() > m_histCap[channel]) {
		y.removeFirst();
	}

	repointPlotCurve(channel);
	if(m_plot[channel]->isVisible()) {
		m_plot[channel]->replot();
	}
}

void Adalm2000VoltmeterTool::applyPlotYRange(int channel, double min, double max)
{
	if(channel < 0 || channel > 1 || !m_plot[channel]) {
		return;
	}
	vm::clampAutoRange(min, max, m_acMode[channel]);
	m_plot[channel]->yAxis()->setInterval(min, max);
}

void Adalm2000VoltmeterTool::setPlotAutoscaleEnabled(int channel, bool on)
{
	if(channel < 0 || channel > 1 || !m_plotAutoscaler[channel] || !m_plotYRange[channel]) {
		return;
	}
	m_plotYRange[channel]->setEnabled(!on);
	if(on && m_runBtn->isChecked()) {
		m_plotAutoscaler[channel]->start();
	} else {
		m_plotAutoscaler[channel]->stop();
	}
}

void Adalm2000VoltmeterTool::setPlotVisible(int channel, bool on)
{
	if(channel < 0 || channel > 1 || !m_plot[channel] || !m_centralLayout) {
		return;
	}
	m_plot[channel]->setVisible(on);
	m_centralLayout->setStretch(channel, on ? 1 : 0);
	if(on) {
		m_plot[channel]->replot();
	}
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

	if(auto *calibration = m_ctx->findChild<M2kCalibration *>()) {
		connect(calibration, &M2kCalibration::started, this, [this]() {
			m_runBtn->setChecked(false);
			m_runBtn->setEnabled(false);
		});
		connect(calibration, &M2kCalibration::finished, this, [this](bool) { m_runBtn->setEnabled(true); });
		m_runBtn->setEnabled(!calibration->isRunning());
	}

	m_gearBtn = new GearBtn(this);
	m_gearBtn->setCheckable(true);
	m_gearBtn->setChecked(true);
	m_tool->addWidgetToTopContainerHelper(m_gearBtn, TTA_RIGHT);

	QWidget *central = new QWidget(this);
	m_centralLayout = new QVBoxLayout(central);
	m_centralLayout->setContentsMargins(0, 0, 0, 0);
	for(int c = 0; c < 2; ++c) {
		m_centralLayout->addWidget(createChannelSection(c, central), 1);
	}
	m_centralLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));
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

		chSection->contentLayout()->addWidget(createPlotSettings(c, chSection));

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

	MenuSectionCollapseWidget *logSection = new MenuSectionCollapseWidget(
		"Data Logging", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, settingsMenu);

	m_logFilePath = new FileBrowserWidget(FileBrowserWidget::SAVE_FILE, logSection);
	m_logFilePath->setFilter("CSV files (*.csv)");
	logSection->contentLayout()->addWidget(m_logFilePath);

	m_logSwitch = new MenuOnOffSwitch("Enable logging", logSection);
	logSection->contentLayout()->addWidget(m_logSwitch);

	connect(m_logFilePath->lineEdit(), &QLineEdit::textChanged, this, [this]() {
		QSignalBlocker blocker(m_logSwitch->onOffswitch());
		m_logSwitch->onOffswitch()->setChecked(false);
		if(m_logger->isOpen()) {
			m_logger->close();
		}
	});

	connect(m_logSwitch->onOffswitch(), &QAbstractButton::toggled, this, [this](bool on) {
		if(on) {
			const QString path = m_logFilePath->lineEdit()->text();
			if(path.isEmpty()) {
				QSignalBlocker blocker(m_logSwitch->onOffswitch());
				m_logSwitch->onOffswitch()->setChecked(false);
				StatusBarManager::pushMessage("Voltmeter: set a log file path first", 3000);
				return;
			}
			if(!m_logger->open(path)) {
				QSignalBlocker blocker(m_logSwitch->onOffswitch());
				m_logSwitch->onOffswitch()->setChecked(false);
			}
		} else {
			m_logger->close();
		}
	});

	settingsMenu->add(logSection, "datalogger");

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

	if(auto *analogIn = m_ctx->findChild<component::Device *>("analog-in", Qt::FindDirectChildrenOnly)) {
		if(component::Attribute *lowRangeMax = component::attributeByName(analogIn, "low_range_max")) {
			connect(lowRangeMax, &component::Attribute::valueChanged, this, [this](const QString &v) {
				bool ok = false;
				const double limit = v.toDouble(&ok);
				if(ok) {
					m_reader->setLowRangeLimit(limit);
				}
			});
			if(lowRangeMax->readCapability()) {
				lowRangeMax->readCapability()->readAsync();
			}
		}
	}

	connect(m_controller, &M2kVoltmeterController::resourceLost, this, [this]() {
		m_reader->requestStop();
		m_runBtn->setChecked(false);
	});

	connect(m_runBtn, &QPushButton::toggled, this, [this](bool on) {
		if(!on) {
			m_scaleTimer->stop();
			for(int c = 0; c < 2; ++c) {
				m_plotAutoscaler[c]->stop();
			}
			m_reader->requestStop();
			m_controller->release();
			if(m_logger && m_logger->isOpen()) {
				m_logger->close();
				QSignalBlocker blocker(m_logSwitch->onOffswitch());
				m_logSwitch->onOffswitch()->setChecked(false);
			}
			return;
		}
		for(int c = 0; c < 2; ++c) {
			m_autoScaler[c].resetWindow();
			clearPlotHistory(c);
			setPlotAutoscaleEnabled(c, m_autoscaleSwitch[c]->onOffswitch()->isChecked());
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
	m_lastReading[channel] = shown;

	applyScalePreset(channel, m_autoScaler[channel].push(shown));
	m_scale[channel]->setValue(shown);
	pushPlotSample(channel, shown);

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

	if(channel == 1 && m_logger && m_logger->isOpen()) {
		m_logger->append(m_lastReading[0], m_lastReading[1]);
	}
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

	clearPlotHistory(channel);
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
