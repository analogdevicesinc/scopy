/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#include "timeplotmanagersettings.h"
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menucollapsesection.h>
#include <gui/widgets/menuheader.h>
#include <gui/stylehelper.h>
#include <timeplotcomponentsettings.h>

#include <grtimechannelcomponent.h>
#include <style.h>

namespace scopy {
namespace adc {

TimePlotManagerSettings::TimePlotManagerSettings(TimePlotManager *mgr, QWidget *parent)
	: QWidget(parent)
	, ToolComponent()
	, m_sampleRateAvailable(false)
{
	m_plotManager = mgr;
	auto *w = createMenu(this);
	QVBoxLayout *lay = new QVBoxLayout(parent);
	lay->addWidget(w);
	lay->setSpacing(0);
	lay->setMargin(0);
	setLayout(lay);
}

TimePlotManagerSettings::~TimePlotManagerSettings() {}

QWidget *TimePlotManagerSettings::createMenu(QWidget *parent)
{
	m_pen = QPen(Style::getAttribute(json::theme::interactive_primary_idle));
	m_menu = new MenuWidget("TIME PLOT", m_pen, parent);

	QWidget *xaxismenu = createXAxisMenu(m_menu);

	m_addPlotBtn = new QPushButton("Add Plot", this);
	StyleHelper::BasicButton(m_addPlotBtn, "AddPlotButton");

	connect(m_addPlotBtn, &QPushButton::clicked, this, [=]() {
		uint32_t idx = m_plotManager->addPlot("Time Plot " + QString::number(m_plotManager->plots().count()));
		TimePlotComponent *plt = m_plotManager->plot(idx);
		addPlot(plt);
	});

	connect(m_plotManager, &PlotManager::plotRemoved, this, [=](uint32_t uuid) {
		TimePlotComponent *plt = m_plotManager->plot(uuid);
		removePlot(plt);
	});

	connect(this, &TimePlotManagerSettings::samplingInfoChanged, this, [=](SamplingInfo s) {
		for(auto p : m_plotManager->plots()) {
			auto tpc = dynamic_cast<TimePlotComponent *>(p);
			if(tpc) {
				tpc->timePlotInfo()->setShowSPS(m_xModeCb->combo()->currentData().toInt() !=
								XMODE_SAMPLES);
				tpc->timePlotInfo()->update(s);
			}
		}
	});

	m_plotSection = new MenuSectionWidget(this);
	m_plotCb = new MenuCombo("Plot Settings", m_plotSection);
	m_plotSection->contentLayout()->addWidget(m_plotCb);

	m_plotStack = new MapStackedWidget(this);
	m_plotStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	m_menu->add(xaxismenu, "xaxis");
	m_menu->add(m_plotSection);
	m_menu->add(m_plotStack);

	connect(m_plotCb->combo(), qOverload<int>(&QComboBox::currentIndexChanged), this,
		[=](int idx) { m_plotStack->show(QString(m_plotCb->combo()->currentData().toInt())); });

	m_menu->add(m_addPlotBtn, "add", gui::MenuWidget::MA_BOTTOMLAST);

	return m_menu;
}

QWidget *TimePlotManagerSettings::createXAxisMenu(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("X-AXIS", MenuCollapseSection::MHCW_NONE,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *bufferPlotSize = new QWidget(section);
	QHBoxLayout *bufferPlotSizeLayout = new QHBoxLayout(bufferPlotSize);
	bufferPlotSizeLayout->setMargin(0);
	bufferPlotSizeLayout->setSpacing(10);
	bufferPlotSize->setLayout(bufferPlotSizeLayout);

	m_bufferSizeSpin = new MenuSpinbox("Buffer Size", 16, "samples", 16, 4000000, true, false, bufferPlotSize);
	m_bufferSizeSpin->setScaleRange(1, 1e6);

	connect(m_bufferSizeSpin, &MenuSpinbox::valueChanged, this, [=](double val) {
		if(m_plotSizeSpin->value() < val) {
			m_plotSizeSpin->setValue(val);
		}
		m_plotSizeSpin->setMinValue(val);
		setBufferSize((uint32_t)val);
	});

	connect(this, &TimePlotManagerSettings::bufferSizeChanged, m_bufferSizeSpin, &MenuSpinbox::setValue);

	m_plotSizeSpin = new MenuSpinbox("Plot Size", 16, "samples", 0, 4000000, true, false, bufferPlotSize);
	m_plotSizeSpin->setScaleRange(1, 1e6);

	connect(m_plotSizeSpin, &MenuSpinbox::valueChanged, this, [=](double val) { setPlotSize((uint32_t)val); });

	bufferPlotSizeLayout->addWidget(m_bufferSizeSpin);
	bufferPlotSizeLayout->addWidget(m_plotSizeSpin);

	m_syncBufferPlot = new MenuOnOffSwitch(tr("SYNC BUFFER-PLOT SIZES"), section, false);
	connect(m_syncBufferPlot->onOffswitch(), &QAbstractButton::toggled, this, [=](bool b) {
		m_plotSizeSpin->setEnabled(!b);
		m_rollingModeSw->setEnabled(!b);
		if(b) {
			m_rollingModeSw->onOffswitch()->setChecked(false);
			m_plotSizeSpin->setValue(m_bufferSizeSpin->value());
			connect(m_bufferSizeSpin, &MenuSpinbox::valueChanged, m_plotSizeSpin, &MenuSpinbox::setValue);
		} else {
			disconnect(m_bufferSizeSpin, &MenuSpinbox::valueChanged, m_plotSizeSpin,
				   &MenuSpinbox::setValue);
		}
	});
	m_rollingModeSw = new MenuOnOffSwitch(tr("ROLLING MODE"), section, false);
	InfoIconWidget::addHoveringInfoToWidget(
		m_rollingModeSw->label(),
		"New data is updating from right to left, like a scrolling motionon the plot."
		"\nSync buffer-plot sizes must be disabled",
		section);
	connect(m_rollingModeSw->onOffswitch(), &QAbstractButton::toggled, this,
		&TimePlotManagerSettings::setRollingMode);

	QWidget *xMinMax = new QWidget(section);
	QHBoxLayout *xMinMaxLayout = new QHBoxLayout(xMinMax);
	xMinMaxLayout->setMargin(0);
	xMinMaxLayout->setSpacing(10);
	xMinMax->setLayout(xMinMaxLayout);

	m_xmin = new MenuSpinbox("XMin", -1, "samples", -DBL_MAX, DBL_MAX, true, false, xMinMax);
	m_xmin->setIncrementMode(gui::MenuSpinbox::IS_FIXED);

	m_xmax = new MenuSpinbox("XMax", -1, "samples", -DBL_MAX, DBL_MAX, true, false, xMinMax);
	m_xmax->setIncrementMode(gui::MenuSpinbox::IS_FIXED);

	connect(m_xmin, &MenuSpinbox::valueChanged, this,
		[=](double min) { m_plotManager->setXInterval(m_xmin->value(), m_xmax->value()); });
	connect(m_xmax, &MenuSpinbox::valueChanged, this,
		[=](double max) { m_plotManager->setXInterval(m_xmin->value(), m_xmax->value()); });

	xMinMaxLayout->addWidget(m_xmin);
	xMinMaxLayout->addWidget(m_xmax);

	m_xModeCb = new MenuCombo("XMODE", section);
	InfoIconWidget::addHoveringInfoToWidget(m_xModeCb->label(),
						"Set X axis scaling mode\nThis does not affect the data", m_xModeCb);
	auto xcb = m_xModeCb->combo();

	xcb->addItem("Samples", XMODE_SAMPLES);
	xcb->addItem("Time - override samplerate", XMODE_OVERRIDE);

	connect(xcb, qOverload<int>(&QComboBox::currentIndexChanged), this, [=](int idx) {
		m_sampleRateSpin->setVisible(false);
		if(xcb->itemData(idx) == XMODE_SAMPLES) {
			m_sampleRateSpin->setValue(1);
			m_xmin->setUnit("samples");
			m_xmin->setScaleRange(1, 1e6);
			m_xmax->setUnit("samples");
			m_xmax->setScaleRange(1, 1e6);
			m_plotManager->setXUnit("samples");
			for(PlotComponent *plt : m_plotManager->plots()) {
				auto p = dynamic_cast<TimePlotComponent *>(plt);
				p->timePlot()->xAxis()->scaleDraw()->setFloatPrecision(3);
				p->timePlot()->xAxis()->getFormatter()->setTwoDecimalMode(false);
			}
		}
		if(xcb->itemData(idx) == XMODE_TIME) {
			m_sampleRateSpin->setVisible(true);
			m_sampleRateSpin->setEnabled(false);
			m_sampleRateSpin->setValue(readSampleRate());
			m_xmin->setUnit("s");
			m_xmin->setScaleRange(0, 1);
			m_xmax->setUnit("s");
			m_xmax->setScaleRange(0, 1);
			m_plotManager->setXUnit("s");

			for(PlotComponent *plt : m_plotManager->plots()) {
				auto p = dynamic_cast<TimePlotComponent *>(plt);
				p->timePlot()->xAxis()->scaleDraw()->setFloatPrecision(3);
				p->timePlot()->xAxis()->scaleDraw()->setUnitsEnabled(true);
				p->timePlot()->xAxis()->getFormatter()->setTwoDecimalMode(true);
			}
		}
		if(xcb->itemData(idx) == XMODE_OVERRIDE) {
			m_sampleRateSpin->setVisible(true);
			m_sampleRateSpin->setEnabled(true);

			m_xmin->setUnit("s");
			m_xmin->setScaleRange(0, 1);
			m_xmax->setUnit("s");
			m_xmax->setScaleRange(0, 1);
			m_plotManager->setXUnit("s");
			for(PlotComponent *plt : m_plotManager->plots()) {
				auto p = dynamic_cast<TimePlotComponent *>(plt);
				p->timePlot()->xAxis()->scaleDraw()->setFloatPrecision(3);
				p->timePlot()->xAxis()->scaleDraw()->setUnitsEnabled(true);
				p->timePlot()->xAxis()->getFormatter()->setTwoDecimalMode(true);
			}
		}
		updateXAxis();
		m_plotManager->updateAxisScales();
	});

	m_sampleRateSpin = new MenuSpinbox("Sample rate", 1, "Hz", 1, DBL_MAX, true, false, section);
	m_sampleRateSpin->setIncrementMode(MenuSpinbox::IS_125);
	InfoIconWidget::addHoveringInfoToWidget(
		m_sampleRateSpin->label(),
		"Controls how the X-axis is scaled when in Time XMode\nThis only affects the x scale, not the data",
		section);

	m_sampleRateSpin->setValue(10);
	m_sampleRateSpin->setEnabled(false);
	connect(m_sampleRateSpin, &MenuSpinbox::valueChanged, this, [=](double val) { setSampleRate(val); });

	connect(this, &TimePlotManagerSettings::sampleRateChanged, m_sampleRateSpin, &MenuSpinbox::setValue);

	section->contentLayout()->setSpacing(10);

	section->contentLayout()->addWidget(bufferPlotSize);
	section->contentLayout()->addWidget(m_syncBufferPlot);
	section->contentLayout()->addWidget(m_rollingModeSw);
	section->contentLayout()->addWidget(xMinMax);
	section->contentLayout()->addWidget(m_xModeCb);
	section->contentLayout()->addWidget(m_sampleRateSpin);
	section->contentLayout()->setSpacing(10);

	return section;
}

void TimePlotManagerSettings::onInit()
{
	m_samplingInfo.plotSize = 1024;
	m_samplingInfo.bufferSize = 1024;
	m_samplingInfo.complexMode = 0;
	m_samplingInfo.rollingMode = 0;
	m_samplingInfo.freqOffset = 0;
	m_samplingInfo.sampleRate = 1;

	m_bufferSizeSpin->setValue(1024);
	m_plotSizeSpin->setValue(1024);
	m_sampleRateSpin->setValue(1);
	m_xmin->setValue(0);
	m_xmax->setValue(1024);
	m_syncBufferPlot->onOffswitch()->setChecked(true);
	m_xModeCb->combo()->setCurrentIndex(1);
	m_xModeCb->combo()->setCurrentIndex(0);

	m_plotManager->updateAxisScales();

	updateXAxis();
	//	m_rollingModeSw->onOffswitch()->setChecked(false);
}

double TimePlotManagerSettings::sampleRate() const { return m_samplingInfo.sampleRate; }

void TimePlotManagerSettings::setSampleRate(double newSampleRate)
{
	if(qFuzzyCompare(m_samplingInfo.sampleRate, newSampleRate))
		return;
	m_samplingInfo.sampleRate = newSampleRate;
	Q_EMIT sampleRateChanged(newSampleRate);
	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

bool TimePlotManagerSettings::rollingMode() const { return m_samplingInfo.rollingMode; }

void TimePlotManagerSettings::setRollingMode(bool newRollingMode)
{
	if(m_samplingInfo.rollingMode == newRollingMode)
		return;
	m_samplingInfo.rollingMode = newRollingMode;
	Q_EMIT rollingModeChanged(newRollingMode);
	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

uint32_t TimePlotManagerSettings::plotSize() const { return m_samplingInfo.plotSize; }

void TimePlotManagerSettings::setPlotSize(uint32_t newPlotSize)
{
	if(m_samplingInfo.plotSize == newPlotSize)
		return;
	m_samplingInfo.plotSize = newPlotSize;
	Q_EMIT plotSizeChanged(newPlotSize);
	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

uint32_t TimePlotManagerSettings::bufferSize() const { return m_samplingInfo.bufferSize; }

void TimePlotManagerSettings::setBufferSize(uint32_t newBufferSize)
{
	if(m_samplingInfo.bufferSize == newBufferSize)
		return;
	m_samplingInfo.bufferSize = newBufferSize;
	Q_EMIT bufferSizeChanged(newBufferSize);
	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

void TimePlotManagerSettings::updateXAxis()
{

	double min = 0;
	double max = m_samplingInfo.plotSize;

	min = min / m_samplingInfo.sampleRate;
	max = max / m_samplingInfo.sampleRate;

	if(m_samplingInfo.rollingMode) {
		m_xmin->setValue(max);
		m_xmax->setValue(min);
	} else {
		m_xmin->setValue(min);
		m_xmax->setValue(max);
	}
}

MenuWidget *TimePlotManagerSettings::menu() { return m_menu; }

void TimePlotManagerSettings::onStart()
{
	QComboBox *cb = m_xModeCb->combo();

	if(cb->itemData(cb->currentIndex()) == XMODE_TIME) {
		double sr = m_sampleRateSpin->value();
		setSampleRate(sr);
	} else {
		Q_EMIT sampleRateChanged(m_samplingInfo.sampleRate);
	}

	Q_EMIT samplingInfoChanged(m_samplingInfo);
	updateXAxis();
}

bool TimePlotManagerSettings::syncBufferPlotSize() const { return m_syncBufferPlotSize; }

void TimePlotManagerSettings::setSyncBufferPlotSize(bool newSyncBufferPlotSize)
{
	if(m_syncBufferPlotSize == newSyncBufferPlotSize)
		return;
	m_syncBufferPlotSize = newSyncBufferPlotSize;
	Q_EMIT syncBufferPlotSizeChanged(newSyncBufferPlotSize);
}

void TimePlotManagerSettings::addPlot(TimePlotComponent *p)
{
	QWidget *plotMenu = p->plotMenu();

	connect(p, &TimePlotComponent::nameChanged, this, [=](QString newName) {
		int idx = m_plotCb->combo()->findData(p->uuid());
		m_plotCb->combo()->setItemText(idx, newName);
	});
	m_plotCb->combo()->addItem(p->name(), p->uuid());
	m_plotStack->add(QString(p->uuid()), plotMenu);
	// m_menu->add(plotMenu, p->name() + QString(p->uuid()), gui::MenuWidget::MA_TOPLAST);
	setPlotComboVisible();
	connect(p->plotMenu(), &TimePlotComponentSettings::requestSettings, this, [=]() {
		int idx = m_plotCb->combo()->findData(p->uuid());
		m_plotCb->combo()->setCurrentIndex(idx);
		m_menu->scrollTo(m_plotCb);
		Q_EMIT requestOpenMenu();
	});
}

void TimePlotManagerSettings::setPlotComboVisible()
{
	bool visible = m_plotCb->combo()->count() > 1;
	m_plotSection->setVisible(visible);
}

void TimePlotManagerSettings::removePlot(TimePlotComponent *p)
{
	QWidget *plotMenu = p->plotMenu();
	// m_menu->remove(plotMenu);
	int idx = m_plotCb->combo()->findData(p->uuid());
	m_plotCb->combo()->removeItem(idx);
	m_plotStack->remove(QString(p->uuid()));

	setPlotComboVisible();
}

void TimePlotManagerSettings::addChannel(ChannelComponent *c)
{
	m_channels.append(c);
	SampleRateProvider *srp = dynamic_cast<SampleRateProvider *>(c);
	if(srp) {
		addSampleRateProvider(srp);
	}

	connect(this, &TimePlotManagerSettings::samplingInfoChanged, c, &ChannelComponent::setSamplingInfo);
}

void TimePlotManagerSettings::removeChannel(ChannelComponent *c)
{
	m_channels.removeAll(c);
	SampleRateProvider *srp = dynamic_cast<SampleRateProvider *>(c);
	if(srp) {
		removeSampleRateProvider(srp);
	}
}

void TimePlotManagerSettings::addSampleRateProvider(SampleRateProvider *s)
{
	m_sampleRateProviders.append(s);
	updateXModeCombo();
}

void TimePlotManagerSettings::removeSampleRateProvider(SampleRateProvider *s) { m_sampleRateProviders.removeAll(s); }

double TimePlotManagerSettings::readSampleRate()
{
	double sr = 1;
	for(SampleRateProvider *ch : qAsConst(m_sampleRateProviders)) {
		if(ch->sampleRateAvailable()) {
			sr = ch->sampleRate();
			break;
		}
	}
	return sr;
}

void TimePlotManagerSettings::updateXModeCombo()
{
	if(m_sampleRateAvailable) // already set
		return;
	m_sampleRateAvailable = true;
	if(m_sampleRateAvailable) {
		auto cb = m_xModeCb->combo();
		cb->insertItem(1, "Time", XMODE_TIME);
		QMetaObject::invokeMethod(cb, "setCurrentIndex", Qt::QueuedConnection, Q_ARG(int, 1));
	}
}

void TimePlotManagerSettings::setEnableAddRemovePlot(bool b) { m_addPlotBtn->setVisible(b); }

} // namespace adc
} // namespace scopy
