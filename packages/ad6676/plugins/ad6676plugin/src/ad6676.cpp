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

#include "ad6676.h"

#include <cmath>

#include <QLoggingCategory>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QSpacerItem>
#include <style.h>

#include <iio-widgets/iiowidgetgroup.h>
#include <iio-widgets/guistrategy/rangeguistrategy.h>
#include <gui/widgets/menusectionwidget.h>
#include <gui/widgets/menuspinbox.h>

Q_LOGGING_CATEGORY(CAT_AD6676, "AD6676")

using namespace scopy::ad6676;
using namespace scopy;

Ad6676::Ad6676(iio_context *ctx, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_ctx(ctx)
	, m_group(group)
{
	setupUi();
	connectSignals();
}

Ad6676::~Ad6676() {}

// ---------------------------------------------------------------------------
// Widget helper methods
// ---------------------------------------------------------------------------

IIOWidget *Ad6676::createRangeWidget(iio_channel *ch, const QString &attr, const QString &range, const QString &title)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_dev)
				    .channel(ch)
				    .attribute(attr)
				    .optionsValues(range)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::RangeUi)
				    .group(m_group)
				    .buildSingle();

	if(widget) {
		connect(this, &Ad6676::readRequested, widget, &IIOWidget::readAsync);
	}
	return widget;
}

IIOWidget *Ad6676::createComboWidget(iio_channel *ch, const QString &attr, const QString &availableAttr,
				     const QString &title)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_dev)
				    .channel(ch)
				    .attribute(attr)
				    .optionsAttribute(availableAttr)
				    .title(title)
				    .uiStrategy(IIOWidgetBuilder::ComboUi)
				    .group(m_group)
				    .buildSingle();

	if(widget) {
		connect(this, &Ad6676::readRequested, widget, &IIOWidget::readAsync);
	}
	return widget;
}

IIOWidget *Ad6676::createReadOnlyWidget(iio_channel *ch, const QString &attr, const QString &title)
{
	IIOWidget *widget = IIOWidgetBuilder(m_centralWidget)
				    .device(m_dev)
				    .channel(ch)
				    .attribute(attr)
				    .title(title)
				    .compactMode(true)
				    .group(m_group)
				    .buildSingle();

	if(widget) {
		widget->setEnabled(false);
		widget->showProgressBar(false);
		connect(this, &Ad6676::readRequested, widget, &IIOWidget::readAsync);
	}
	return widget;
}

// ---------------------------------------------------------------------------
// Main UI setup
// ---------------------------------------------------------------------------

void Ad6676::setupUi()
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

	if(!m_ctx) {
		qWarning(CAT_AD6676) << "No IIO context provided";
		return;
	}

	// Find the single IIO device: "axi-ad6676-hpc"
	m_dev = iio_context_find_device(m_ctx, "axi-ad6676-hpc");
	if(!m_dev) {
		qWarning(CAT_AD6676) << "Device axi-ad6676-hpc not found";
		return;
	}

	// All attributes use voltage0 (input, false)
	m_chn = iio_device_find_channel(m_dev, "voltage0", false);
	if(!m_chn) {
		qWarning(CAT_AD6676) << "Channel voltage0 not found";
		return;
	}

	// Build scrollable central widget
	m_centralWidget = new QWidget();
	m_scrollArea = new QScrollArea(this);
	m_scrollArea->setWidget(m_centralWidget);
	m_scrollArea->setWidgetResizable(true);
	m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	m_scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	QVBoxLayout *centralLayout = new QVBoxLayout(m_centralWidget);
	centralLayout->setMargin(0);
	centralLayout->setContentsMargins(0, 0, 0, 0);
	centralLayout->setSpacing(10);

	QWidget *contentWidget = new QWidget(m_centralWidget);

	Style::setBackgroundColor(contentWidget, json::theme::background_primary);
	Style::setStyle(contentWidget, style::properties::widget::border_interactive);

	QVBoxLayout *contentWidgetLayout = new QVBoxLayout(contentWidget);
	contentWidgetLayout->setMargin(0);
	contentWidgetLayout->setContentsMargins(0, 0, 0, 0);
	contentWidgetLayout->setSpacing(10);

	MenuCollapseSection *collapseSection = new MenuCollapseSection(
		"AD6676", MenuCollapseSection::MHCW_NONE, MenuCollapseSection::MHW_BASEWIDGET, m_centralWidget);

	collapseSection->contentLayout()->addWidget(createAdcSettingsSection(collapseSection));
	collapseSection->contentLayout()->addWidget(createBandwidthSettingsSection(collapseSection));
	collapseSection->contentLayout()->addWidget(createReceiveSettingsSection(collapseSection));
	collapseSection->contentLayout()->addWidget(createShufflerSettingsSection(collapseSection));
	collapseSection->contentLayout()->addWidget(createTestSettingsSection(collapseSection));
	contentWidgetLayout->addWidget(collapseSection);

	centralLayout->addWidget(contentWidget);
	centralLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	m_tool->addWidgetToCentralContainerHelper(m_scrollArea);
}

void Ad6676::connectSignals()
{
	// Refresh button — run readRequested() on a worker thread
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

	// Dynamic bandwidth range: update min/max whenever adc_frequency changes
	if(m_adcFreqWidget && m_bwWidget) {
		auto *adcUiStrategy = dynamic_cast<RangeAttrUi *>(m_adcFreqWidget->getUiStrategy());
		if(adcUiStrategy) {
			connect(adcUiStrategy, &RangeAttrUi::displayedNewData, this, &Ad6676::onAdcFreqDisplayed);
		}
	}
}

void Ad6676::updateBandwidthRange(QString adcFreqMhz)
{
	if(!m_bwWidget)
		return;

	double freqMhz = adcFreqMhz.toDouble();
	if(qFuzzyIsNull(freqMhz))
		return;

	double minBw = 0.005 * freqMhz;
	double maxBw = 0.05 * freqMhz;
	QString newRange = QString("[%1 0.001 %2]").arg(minBw).arg(maxBw);

	auto *ds = m_bwWidget->getDataStrategy();
	ds->setConstDataOptions(newRange);

	auto *bwSpinbox = m_bwWidget->findChild<gui::MenuSpinbox *>();
	if (bwSpinbox) {
		bwSpinbox->setMinValue(minBw);
		bwSpinbox->setMaxValue(maxBw);
		bwSpinbox->setValueForce(bwSpinbox->value(), true);
	}
}

void Ad6676::onAdcFreqDisplayed(QString data, QString) { updateBandwidthRange(data); }

// ---------------------------------------------------------------------------
// Section creation — attributes ported from iio-oscilloscope ad6676.c
// Device: axi-ad6676-hpc / Channel: voltage0 (input, false)
// ---------------------------------------------------------------------------

QWidget *Ad6676::createAdcSettingsSection(QWidget *parent)
{
	QWidget *section = new QWidget(parent);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(5, 5, 5, 5);
	section->setLayout(layout);

	QLabel *label = new QLabel("ADC Settings");
	layout->addWidget(label);

	// adc_frequency: IIO in Hz, UI in MHz. Range 2925–3200 MHz.
	m_adcFreqWidget = createRangeWidget(m_chn, "adc_frequency", "[2925 1 3200]", "ADC Frequency (MHz)");
	if(m_adcFreqWidget) {
		m_adcFreqWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 3); });
		m_adcFreqWidget->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
		layout->addWidget(m_adcFreqWidget);
	}

	return section;
}

QWidget *Ad6676::createBandwidthSettingsSection(QWidget *parent)
{
	QWidget *section = new QWidget(parent);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(5, 5, 5, 5);
	section->setLayout(layout);

	QLabel *label = new QLabel("Bandwidth Settings");
	layout->addWidget(label);

	// bandwidth: IIO in Hz, UI in MHz. Initial range 14.625–160 MHz (= 0.5%–5% of 2925 MHz).
	// Range is dynamically updated when adc_frequency changes (see updateBandwidthRange).
	m_bwWidget = createRangeWidget(m_chn, "bandwidth", "[14.625 0.001 160]", "Bandwidth (MHz)");
	if(m_bwWidget) {
		m_bwWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 3); });
		m_bwWidget->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	}

	// bw_margin_low: integer, no scaling, range 0–30
	IIOWidget *bwMarginLow = createRangeWidget(m_chn, "bw_margin_low", "[0 1 30]", "BW Margin Low");

	// bw_margin_high: integer, no scaling, range 0–30
	IIOWidget *bwMarginHigh = createRangeWidget(m_chn, "bw_margin_high", "[0 1 30]", "BW Margin High");

	// bw_margin_if: integer, no scaling, range -30–30
	IIOWidget *bwMarginIf = createRangeWidget(m_chn, "bw_margin_if", "[-30 1 30]", "BW Margin IF");

	QHBoxLayout *bwLayout = new QHBoxLayout();
	bwLayout->setContentsMargins(0, 0, 0, 0);
	if(m_bwWidget)
		bwLayout->addWidget(m_bwWidget);
	if(bwMarginLow)
		bwLayout->addWidget(bwMarginLow);
	if(bwMarginHigh)
		bwLayout->addWidget(bwMarginHigh);
	if(bwMarginIf)
		bwLayout->addWidget(bwMarginIf);
	layout->addLayout(bwLayout);

	return section;
}

QWidget *Ad6676::createReceiveSettingsSection(QWidget *parent)
{
	QWidget *section = new QWidget(parent);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(5, 5, 5, 5);
	section->setLayout(layout);

	QLabel *label = new QLabel("Receive Settings");
	layout->addWidget(label);

	// intermediate_frequency: IIO in Hz, UI in MHz. Range 70–450 MHz.
	IIOWidget *ifWidget =
		createRangeWidget(m_chn, "intermediate_frequency", "[70 1 450]", "Intermediate Frequency (MHz)");
	if(ifWidget) {
		ifWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 6); });
		ifWidget->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	}

	// sampling_frequency: IIO in Hz, UI in MHz. Display-only (editable=False in glade).
	IIOWidget *sampFreqWidget = createReadOnlyWidget(m_chn, "sampling_frequency", "Data Rate (MSPS)");
	if(sampFreqWidget) {
		sampFreqWidget->setDataToUIConversion(
			[](QString data) { return QString::number(data.toDouble() / 1e6, 'f', 3); });
		sampFreqWidget->setUItoDataConversion(
			[](QString data) { return QString::number(data.toDouble() * 1e6, 'f', 0); });
	}

	// hardwaregain: float, no scaling, range -27–0. Label: "Attenuation(dB)".
	IIOWidget *gainWidget = createRangeWidget(m_chn, "hardwaregain", "[-27 1 0]", "Attenuation (dB)");

	// scale: IIO stores linear (0.25–1.0), UI shows dBFS (0–12.04 positive).
	// db_full_scale_convert: linear→dBFS = (int)(20*log10(1/linear)+0.5)
	//                        dBFS→linear = pow(10, -dbfs/20.0)
	IIOWidget *scaleWidget = createRangeWidget(m_chn, "scale", "[0 1 12]", "Input Signal Power (-dBFS)");
	if(scaleWidget) {
		scaleWidget->setDataToUIConversion([](QString data) {
			double linear = data.toDouble();
			if(qFuzzyIsNull(linear))
				return QString("0");
			return QString::number((int)(20.0 * log10(1.0 / linear) + 0.5));
		});
		scaleWidget->setUItoDataConversion([](QString data) {
			double dbfs = data.toDouble();
			return QString::number(pow(10.0, -dbfs / 20.0), 'g', 10);
		});
	}

	QHBoxLayout *rxLayout = new QHBoxLayout();
	rxLayout->setContentsMargins(0, 0, 0, 0);
	if(ifWidget)
		rxLayout->addWidget(ifWidget);
	if(sampFreqWidget)
		rxLayout->addWidget(sampFreqWidget);
	if(gainWidget)
		rxLayout->addWidget(gainWidget);
	if(scaleWidget)
		rxLayout->addWidget(scaleWidget);
	layout->addLayout(rxLayout);

	return section;
}

QWidget *Ad6676::createShufflerSettingsSection(QWidget *parent)
{
	QWidget *section = new QWidget(parent);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(5, 5, 5, 5);
	section->setLayout(layout);

	QLabel *label = new QLabel("Shuffler Settings");
	layout->addWidget(label);

	// shuffler_control: options from shuffler_control_available
	IIOWidget *shufflerCtrl =
		createComboWidget(m_chn, "shuffler_control", "shuffler_control_available", "Shuffler Control");

	// shuffler_thresh: options from shuffler_thresh_available
	IIOWidget *shufflerThresh =
		createComboWidget(m_chn, "shuffler_thresh", "shuffler_thresh_available", "Shuffler Threshold");

	QHBoxLayout *shufflerLayout = new QHBoxLayout();
	shufflerLayout->setContentsMargins(0, 0, 0, 0);
	if(shufflerCtrl)
		shufflerLayout->addWidget(shufflerCtrl);
	if(shufflerThresh)
		shufflerLayout->addWidget(shufflerThresh);
	layout->addLayout(shufflerLayout);

	return section;
}

QWidget *Ad6676::createTestSettingsSection(QWidget *parent)
{
	QWidget *section = new QWidget(parent);

	QVBoxLayout *layout = new QVBoxLayout();
	layout->setSpacing(10);
	layout->setContentsMargins(5, 5, 5, 5);
	section->setLayout(layout);

	QLabel *label = new QLabel("Test Settings");
	layout->addWidget(label);

	// test_mode: options from test_mode_available
	// Note: sr_attribs in original uses "stest_mode" — may be a legacy alias; "test_mode" used here.
	IIOWidget *testMode = createComboWidget(m_chn, "test_mode", "test_mode_available", "Test Mode");
	if(testMode)
		layout->addWidget(testMode);

	return section;
}
