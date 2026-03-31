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
 */

#include "advanced/obssettingswidget.h"
#include "ad9371widgetfactory.h"
#include <iio-widgets/iiowidgetgroup.h>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QLabel>
#include <QGroupBox>
#include <QSpacerItem>
#include <iio-widgets/iiowidget.h>
#include <QLoggingCategory>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_AD9371_OBS_SETTINGS, "AD9371_OBS_SETTINGS")

using namespace scopy;
using namespace scopy::ad9371;

ObsSettingsWidget::ObsSettingsWidget(iio_device *device, IIOWidgetGroup *group, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
	, m_widgetGroup(group)
{
	if(!m_device) {
		qWarning(CAT_AD9371_OBS_SETTINGS) << "No device provided to OBS Settings";
		return;
	}

	setupUi();
	readChannelEnableFromDevice();
}

ObsSettingsWidget::~ObsSettingsWidget() {}

void ObsSettingsWidget::setupUi()
{
	auto *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	QScrollArea *scrollArea = new QScrollArea(this);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	scrollArea->setWidgetResizable(true);

	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	contentLayout->addWidget(createObsSettingsSection(contentWidget));
	contentLayout->addWidget(createObsProfileSection(contentWidget));
	contentLayout->addWidget(createSnifferProfileSection(contentWidget));

	contentLayout->addItem(new QSpacerItem(1, 1, QSizePolicy::Preferred, QSizePolicy::Expanding));

	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	connect(this, &ObsSettingsWidget::readRequested, this, &ObsSettingsWidget::readChannelEnableFromDevice);

	qDebug(CAT_AD9371_OBS_SETTINGS) << "OBS Settings widget created with 25 attributes in 3 sections";
}

// --- OBS Settings Section (9 attrs) ---
QWidget *ObsSettingsWidget::createObsSettingsSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("Config", MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);
	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// #1-5: CHECKBOX_MASK bits 0-4 on adi,obs-settings-obs-rx-channels-enable
	QGroupBox *chEnableGroup = new QGroupBox("CHANNEL ENABLE", widget);
	QGridLayout *chEnableLayout = new QGridLayout(chEnableGroup);

	m_chEnable0 = new MenuOnOffSwitch("ORX1", chEnableGroup);
	m_chEnable0->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	chEnableLayout->addWidget(m_chEnable0, 0, 0);

	m_chEnable1 = new MenuOnOffSwitch("ORX2", chEnableGroup);
	m_chEnable1->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	chEnableLayout->addWidget(m_chEnable1, 0, 1);

	m_chEnable2 = new MenuOnOffSwitch("SNRXA", chEnableGroup);
	m_chEnable2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	chEnableLayout->addWidget(m_chEnable2, 1, 0);

	m_chEnable3 = new MenuOnOffSwitch("SNRXB", chEnableGroup);
	m_chEnable3->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	chEnableLayout->addWidget(m_chEnable3, 1, 1);

	m_chEnable4 = new MenuOnOffSwitch("SNRXC", chEnableGroup);
	m_chEnable4->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
	chEnableLayout->addWidget(m_chEnable4, 1, 2);

	connect(m_chEnable0->onOffswitch(), &QAbstractButton::toggled, this,
		&ObsSettingsWidget::writeChannelEnableToDevice);
	connect(m_chEnable1->onOffswitch(), &QAbstractButton::toggled, this,
		&ObsSettingsWidget::writeChannelEnableToDevice);
	connect(m_chEnable2->onOffswitch(), &QAbstractButton::toggled, this,
		&ObsSettingsWidget::writeChannelEnableToDevice);
	connect(m_chEnable3->onOffswitch(), &QAbstractButton::toggled, this,
		&ObsSettingsWidget::writeChannelEnableToDevice);
	connect(m_chEnable4->onOffswitch(), &QAbstractButton::toggled, this,
		&ObsSettingsWidget::writeChannelEnableToDevice);

	layout->addWidget(chEnableGroup);

	// #6: adi,obs-settings-obs-rx-lo-source - Combo {0:"Internal", 1:"External"}
	QMap<QString, QString> loSourceOptions;
	loSourceOptions.insert("0", "OBSLO_TX_PLL");
	loSourceOptions.insert("1", "OBSLO_SNIFFER_PLL");
	IIOWidget *loSource = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,obs-settings-obs-rx-lo-source", loSourceOptions, "LO SOURCE");
	if(loSource) {
		if(m_widgetGroup)
			m_widgetGroup->add(loSource);
		layout->addWidget(loSource);
		connect(this, &ObsSettingsWidget::readRequested, loSource, &IIOWidget::readAsync);
	}

	// #7: adi,obs-settings-sniffer-pll-lo-frequency_hz
	QString snifferLoRange = "[300000000 1 4000000000]";
	IIOWidget *snifferLoFreq =
		Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,obs-settings-sniffer-pll-lo-frequency_hz",
							    snifferLoRange, "SNIFFER PLL LO FREQUENCY HZ");
	if(snifferLoFreq) {
		if(m_widgetGroup)
			m_widgetGroup->add(snifferLoFreq);
		layout->addWidget(snifferLoFreq);
		connect(this, &ObsSettingsWidget::readRequested, snifferLoFreq, &IIOWidget::readAsync);
	}

	// #8: adi,obs-settings-real-if-data - Checkbox
	IIOWidget *realIfData = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,obs-settings-real-if-data", "REAL IF DATA");
	if(realIfData) {
		if(m_widgetGroup)
			m_widgetGroup->add(realIfData);
		layout->addWidget(realIfData);
		connect(this, &ObsSettingsWidget::readRequested, realIfData, &IIOWidget::readAsync);
	}

	// #9: adi,obs-settings-default-obs-rx-channel - Combo with non-sequential LUT
	QMap<QString, QString> defaultChOptions;
	defaultChOptions.insert("0", "RXOFF");
	defaultChOptions.insert("1", "RX1_TXLO");
	defaultChOptions.insert("2", "RX2_TXLO");
	defaultChOptions.insert("3", "INTERNALCALS");
	defaultChOptions.insert("4", "SNIFFER");
	defaultChOptions.insert("5", "RX1_SNIFFERLO");
	defaultChOptions.insert("6", "RX2_SNIFFERLO");
	defaultChOptions.insert("20", "SNIFFER_A");
	defaultChOptions.insert("36", "SNIFFER_B");
	defaultChOptions.insert("52", "SNIFFER_C");
	IIOWidget *defaultCh = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,obs-settings-default-obs-rx-channel", defaultChOptions, "DEFAULT CHANNEL");
	if(defaultCh) {
		if(m_widgetGroup)
			m_widgetGroup->add(defaultCh);
		layout->addWidget(defaultCh);
		connect(this, &ObsSettingsWidget::readRequested, defaultCh, &IIOWidget::readAsync);
	}

	return section;
}

// --- OBS Profile Section (8 attrs) ---
QWidget *ObsSettingsWidget::createObsProfileSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget(
		"Observation", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);
	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// #10: adi,obs-profile-adc-div - Range [1 1 2]
	IIOWidget *adcDiv =
		Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,obs-profile-adc-div", "[1 1 2]", "ADC DIV");
	if(adcDiv) {
		if(m_widgetGroup)
			m_widgetGroup->add(adcDiv);
		layout->addWidget(adcDiv);
		connect(this, &ObsSettingsWidget::readRequested, adcDiv, &IIOWidget::readAsync);
	}

	// #11: adi,obs-profile-rx-fir-decimation - Combo {1:"DECIMATE by 1", 2:"DECIMATE by 2", 4:"DECIMATE by 4"}
	QMap<QString, QString> firDecOptions;
	firDecOptions.insert("1", "DECIMATE by 1");
	firDecOptions.insert("2", "DECIMATE by 2");
	firDecOptions.insert("4", "DECIMATE by 4");
	IIOWidget *firDec = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,obs-profile-rx-fir-decimation", firDecOptions, "RX FIR");
	if(firDec) {
		if(m_widgetGroup)
			m_widgetGroup->add(firDec);
		layout->addWidget(firDec);
		connect(this, &ObsSettingsWidget::readRequested, firDec, &IIOWidget::readAsync);
	}

	// #12: adi,obs-profile-rx-dec5-decimation - Range [4 1 5]
	IIOWidget *dec5Dec = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,obs-profile-rx-dec5-decimation",
									 "[4 1 5]", "RX DEC5 DECIMATION");
	if(dec5Dec) {
		if(m_widgetGroup)
			m_widgetGroup->add(dec5Dec);
		layout->addWidget(dec5Dec);
		connect(this, &ObsSettingsWidget::readRequested, dec5Dec, &IIOWidget::readAsync);
	}

	// #13: adi,obs-profile-en-high-rej-dec5 - Checkbox
	IIOWidget *highRejDec5 = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,obs-profile-en-high-rej-dec5", "EN HIGH REJ DEC5");
	if(highRejDec5) {
		if(m_widgetGroup)
			m_widgetGroup->add(highRejDec5);
		layout->addWidget(highRejDec5);
		connect(this, &ObsSettingsWidget::readRequested, highRejDec5, &IIOWidget::readAsync);
	}

	// #14: adi,obs-profile-rhb1-decimation - Range [1 1 2]
	IIOWidget *rhb1Dec = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,obs-profile-rhb1-decimation",
									 "[1 1 2]", "RHB1 DECIMATION");
	if(rhb1Dec) {
		if(m_widgetGroup)
			m_widgetGroup->add(rhb1Dec);
		layout->addWidget(rhb1Dec);
		connect(this, &ObsSettingsWidget::readRequested, rhb1Dec, &IIOWidget::readAsync);
	}

	// #15: adi,obs-profile-iq-rate_khz - Range [20000 1 320000]
	IIOWidget *iqRate = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,obs-profile-iq-rate_khz",
									"[20000 1 320000]", "IQ RATE KHZ");
	if(iqRate) {
		if(m_widgetGroup)
			m_widgetGroup->add(iqRate);
		layout->addWidget(iqRate);
		connect(this, &ObsSettingsWidget::readRequested, iqRate, &IIOWidget::readAsync);
	}

	// #16: adi,obs-profile-rf-bandwidth_hz - Range [5000000 1 240000000]
	IIOWidget *rfBw = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,obs-profile-rf-bandwidth_hz",
								      "[5000000 1 240000000]", "RF BANDWIDTH HZ");
	if(rfBw) {
		if(m_widgetGroup)
			m_widgetGroup->add(rfBw);
		layout->addWidget(rfBw);
		connect(this, &ObsSettingsWidget::readRequested, rfBw, &IIOWidget::readAsync);
	}

	// #17: adi,obs-profile-rx-bbf-3db-corner_khz - Range [0 1 250000]
	IIOWidget *bbfCorner = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,obs-profile-rx-bbf-3db-corner_khz", "[0 1 250000]", "RX BBF 3DB CORNER KHZ");
	if(bbfCorner) {
		if(m_widgetGroup)
			m_widgetGroup->add(bbfCorner);
		layout->addWidget(bbfCorner);
		connect(this, &ObsSettingsWidget::readRequested, bbfCorner, &IIOWidget::readAsync);
	}

	return section;
}

// --- Sniffer Profile Section (8 attrs) ---
QWidget *ObsSettingsWidget::createSnifferProfileSection(QWidget *parent)
{
	MenuSectionCollapseWidget *section = new MenuSectionCollapseWidget("Sniffer", MenuCollapseSection::MHCW_ARROW,
									   MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);
	section->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// #18: adi,sniffer-profile-adc-div - Range [1 1 2]
	IIOWidget *adcDiv = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,sniffer-profile-adc-div",
									"[1 1 2]", "ADC DIV");
	if(adcDiv) {
		if(m_widgetGroup)
			m_widgetGroup->add(adcDiv);
		layout->addWidget(adcDiv);
		connect(this, &ObsSettingsWidget::readRequested, adcDiv, &IIOWidget::readAsync);
	}

	// #19: adi,sniffer-profile-rx-fir-decimation - Combo {1:"DECIMATE by 1", 2:"DECIMATE by 2", 4:"DECIMATE by 4"}
	QMap<QString, QString> firDecOptions;
	firDecOptions.insert("1", "DECIMATE by 1");
	firDecOptions.insert("2", "DECIMATE by 2");
	firDecOptions.insert("4", "DECIMATE by 4");
	IIOWidget *firDec = Ad9371WidgetFactory::createDebugCustomComboWidget(
		m_device, "adi,sniffer-profile-rx-fir-decimation", firDecOptions, "RX FIR");
	if(firDec) {
		if(m_widgetGroup)
			m_widgetGroup->add(firDec);
		layout->addWidget(firDec);
		connect(this, &ObsSettingsWidget::readRequested, firDec, &IIOWidget::readAsync);
	}

	// #20: adi,sniffer-profile-rx-dec5-decimation - Range [4 1 5]
	IIOWidget *dec5Dec = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,sniffer-profile-rx-dec5-decimation", "[4 1 5]", "RX DEC5 DECIMATION");
	if(dec5Dec) {
		if(m_widgetGroup)
			m_widgetGroup->add(dec5Dec);
		layout->addWidget(dec5Dec);
		connect(this, &ObsSettingsWidget::readRequested, dec5Dec, &IIOWidget::readAsync);
	}

	// #21: adi,sniffer-profile-en-high-rej-dec5 - Checkbox
	IIOWidget *highRejDec5 = Ad9371WidgetFactory::createDebugCheckboxWidget(
		m_device, "adi,sniffer-profile-en-high-rej-dec5", "EN HIGH REJ DEC5");
	if(highRejDec5) {
		if(m_widgetGroup)
			m_widgetGroup->add(highRejDec5);
		layout->addWidget(highRejDec5);
		connect(this, &ObsSettingsWidget::readRequested, highRejDec5, &IIOWidget::readAsync);
	}

	// #22: adi,sniffer-profile-rhb1-decimation - Range [1 1 2]
	IIOWidget *rhb1Dec = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,sniffer-profile-rhb1-decimation", "[1 1 2]", "RHB1 DECIMATION");
	if(rhb1Dec) {
		if(m_widgetGroup)
			m_widgetGroup->add(rhb1Dec);
		layout->addWidget(rhb1Dec);
		connect(this, &ObsSettingsWidget::readRequested, rhb1Dec, &IIOWidget::readAsync);
	}

	// #23: adi,sniffer-profile-iq-rate_khz - Range [0 1 61440]
	IIOWidget *iqRate = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,sniffer-profile-iq-rate_khz",
									"[0 1 61440]", "IQ RATE KHZ");
	if(iqRate) {
		if(m_widgetGroup)
			m_widgetGroup->add(iqRate);
		layout->addWidget(iqRate);
		connect(this, &ObsSettingsWidget::readRequested, iqRate, &IIOWidget::readAsync);
	}

	// #24: adi,sniffer-profile-rf-bandwidth_hz - Range [0 1 20000000]
	IIOWidget *rfBw = Ad9371WidgetFactory::createDebugRangeWidget(m_device, "adi,sniffer-profile-rf-bandwidth_hz",
								      "[0 1 20000000]", "RF BANDWIDTH HZ");
	if(rfBw) {
		if(m_widgetGroup)
			m_widgetGroup->add(rfBw);
		layout->addWidget(rfBw);
		connect(this, &ObsSettingsWidget::readRequested, rfBw, &IIOWidget::readAsync);
	}

	// #25: adi,sniffer-profile-rx-bbf-3db-corner_khz - Range [0 1 20000]
	IIOWidget *bbfCorner = Ad9371WidgetFactory::createDebugRangeWidget(
		m_device, "adi,sniffer-profile-rx-bbf-3db-corner_khz", "[0 1 20000]", "RX BBF 3DB CORNER KHZ");
	if(bbfCorner) {
		if(m_widgetGroup)
			m_widgetGroup->add(bbfCorner);
		layout->addWidget(bbfCorner);
		connect(this, &ObsSettingsWidget::readRequested, bbfCorner, &IIOWidget::readAsync);
	}

	return section;
}

void ObsSettingsWidget::readChannelEnableFromDevice()
{
	if(!m_device || !m_chEnable0)
		return;

	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,obs-settings-obs-rx-channels-enable", &mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_OBS_SETTINGS) << "Failed to read OBS RX channels enable mask, error:" << ret;
		return;
	}

	// Block signals to prevent triggering writes during read
	m_chEnable0->blockSignals(true);
	m_chEnable1->blockSignals(true);
	m_chEnable2->blockSignals(true);
	m_chEnable3->blockSignals(true);
	m_chEnable4->blockSignals(true);

	m_chEnable0->onOffswitch()->setChecked((mask & (1LL << 0)) != 0);
	m_chEnable1->onOffswitch()->setChecked((mask & (1LL << 1)) != 0);
	m_chEnable2->onOffswitch()->setChecked((mask & (1LL << 2)) != 0);
	m_chEnable3->onOffswitch()->setChecked((mask & (1LL << 3)) != 0);
	m_chEnable4->onOffswitch()->setChecked((mask & (1LL << 4)) != 0);

	m_chEnable0->blockSignals(false);
	m_chEnable1->blockSignals(false);
	m_chEnable2->blockSignals(false);
	m_chEnable3->blockSignals(false);
	m_chEnable4->blockSignals(false);

	qDebug(CAT_AD9371_OBS_SETTINGS) << "Read OBS RX channels enable mask:" << QString("0x%1").arg(mask, 0, 16);
}

void ObsSettingsWidget::writeChannelEnableToDevice()
{
	if(!m_device || !m_chEnable0)
		return;

	// Read-modify-write: preserve bits not managed by this UI
	long long mask = 0;
	int ret = iio_device_debug_attr_read_longlong(m_device, "adi,obs-settings-obs-rx-channels-enable", &mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_OBS_SETTINGS) << "Failed to read mask before write, error:" << ret;
		return;
	}

	// Clear managed bits 0-4
	mask &= ~0x1FLL;

	if(m_chEnable0->onOffswitch()->isChecked())
		mask |= (1LL << 0);
	if(m_chEnable1->onOffswitch()->isChecked())
		mask |= (1LL << 1);
	if(m_chEnable2->onOffswitch()->isChecked())
		mask |= (1LL << 2);
	if(m_chEnable3->onOffswitch()->isChecked())
		mask |= (1LL << 3);
	if(m_chEnable4->onOffswitch()->isChecked())
		mask |= (1LL << 4);

	ret = iio_device_debug_attr_write_longlong(m_device, "adi,obs-settings-obs-rx-channels-enable", mask);
	if(ret < 0) {
		qWarning(CAT_AD9371_OBS_SETTINGS) << "Failed to write OBS RX channels enable mask, error:" << ret;
	}

	qDebug(CAT_AD9371_OBS_SETTINGS) << "Wrote OBS RX channels enable mask:" << QString("0x%1").arg(mask, 0, 16);
}
