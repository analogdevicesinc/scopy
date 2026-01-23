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

#include "advanced/agcsetupwidget.h"
#include "adrv9009widgetfactory.h"
#include <gui/widgets/menucollapsesection.h>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSpacerItem>
#include <QScrollArea>
#include <QLoggingCategory>
#include <QLabel>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_AGCSETUP, "AgcSetup")

using namespace scopy;
using namespace scopy::adrv9009;

AgcSetupWidget::AgcSetupWidget(iio_device *device, QWidget *parent)
	: QWidget(parent)
	, m_device(device)
{
	if(!m_device) {
		qWarning(CAT_AGCSETUP) << "No device provided to AGC Setup widget";
		return;
	}

	setupUi();
}

AgcSetupWidget::~AgcSetupWidget() {}

void AgcSetupWidget::setupUi()
{
	// Main layout for this widget
	QVBoxLayout *mainLayout = new QVBoxLayout(this);
	mainLayout->setContentsMargins(0, 0, 0, 0);
	mainLayout->setSpacing(0);

	// Create scroll area for all sections
	QScrollArea *scrollArea = new QScrollArea();
	scrollArea->setWidgetResizable(true);
	scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

	// Create content widget for scroll area
	QWidget *contentWidget = new QWidget();
	QVBoxLayout *contentLayout = new QVBoxLayout(contentWidget);
	contentLayout->setContentsMargins(10, 10, 10, 10);
	contentLayout->setSpacing(15);

	contentLayout->addWidget(createAgcConfigurationWidget(contentWidget));
	contentLayout->addWidget(createAnalogPeakDetector(contentWidget));
	contentLayout->addWidget(createPowerMeasurementDetector(contentWidget));

	// Add spacer to push sections to top
	contentLayout->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

	// Set up scroll area
	scrollArea->setWidget(contentWidget);
	mainLayout->addWidget(scrollArea);

	qDebug(CAT_AGCSETUP) << "AGC Setup widget created with 63 attributes in 3 collapsible sections";
}

QWidget *AgcSetupWidget::createAgcConfigurationWidget(QWidget *parent)
{
	// 1. AGC Config section (19 attributes)
	MenuSectionCollapseWidget *agcConfigSection = new MenuSectionCollapseWidget(
		"AGC Config", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	agcConfigSection->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// RX1/RX2 MAX GAIN INDEX - Two-column layout [0 1 255]
	auto maxGainIndexGroup = createAgcRxChannelGroup("adi,rxagc-agc-rx%1-max-gain-index", "MAX GAIN INDEX",
							 "[0 1 255]", agcConfigSection);
	layout->addWidget(maxGainIndexGroup);

	// RX1/RX2 MIN GAIN INDEX - Two-column layout [0 1 255]
	auto minGainIndexGroup = createAgcRxChannelGroup("adi,rxagc-agc-rx%1-min-gain-index", "MIN GAIN INDEX",
							 "[0 1 255]", agcConfigSection);
	layout->addWidget(minGainIndexGroup);

	// RX1/RX2 ATTACK DELAY - Two-column layout [0 1 63]
	auto attackDelayGroup = createAgcRxChannelGroup("adi,rxagc-agc-rx%1-attack-delay", "ATTACK DELAY", "[0 1 63]",
							agcConfigSection);
	layout->addWidget(attackDelayGroup);

	// Peak Wait Time - Range Widget [0 1 31]
	auto peakWaitTime = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-agc-peak-wait-time",
								     "[0 1 31]", "Peak Wait Time");
	if(peakWaitTime) {
		layout->addWidget(peakWaitTime);
		connect(this, &AgcSetupWidget::readRequested, peakWaitTime, &IIOWidget::readAsync);
	}

	// Gain Update Counter (us) - Range Widget [0 1 16000000]
	auto gainUpdateCounter = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-agc-gain-update-counter_us", "[0 1 16000000]", "Gain Update Counter (us)");
	if(gainUpdateCounter) {
		layout->addWidget(gainUpdateCounter);
		connect(this, &AgcSetupWidget::readRequested, gainUpdateCounter, &IIOWidget::readAsync);
	}

	// Slow Loop Settling Delay - Range Widget [0 1 127]
	auto slowLoopSettlingDelay = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-agc-slow-loop-settling-delay", "[0 1 127]", "Slow Loop Settling Delay");
	if(slowLoopSettlingDelay) {
		layout->addWidget(slowLoopSettlingDelay);
		connect(this, &AgcSetupWidget::readRequested, slowLoopSettlingDelay, &IIOWidget::readAsync);
	}

	// Low Thresh Prevent Gain - Checkbox
	auto lowThreshPreventGain = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-agc-low-thresh-prevent-gain", "AGC Low Thresh Prevent Gain");
	if(lowThreshPreventGain) {
		layout->addWidget(lowThreshPreventGain);
		lowThreshPreventGain->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, lowThreshPreventGain, &IIOWidget::readAsync);
	}

	// Change Gain If Thresh High - Checkbox
	auto changeGainIfThreshHigh = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-agc-change-gain-if-thresh-high", "AGC Change Gain If Thresh High");
	if(changeGainIfThreshHigh) {
		layout->addWidget(changeGainIfThreshHigh);
		changeGainIfThreshHigh->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, changeGainIfThreshHigh, &IIOWidget::readAsync);
	}

	// Peak Thresh Gain Control Mode - Checkbox
	auto peakThreshGainControlMode = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-agc-peak-thresh-gain-control-mode", "AGC Peak Thresh Gain Control Mode");
	if(peakThreshGainControlMode) {
		layout->addWidget(peakThreshGainControlMode);
		peakThreshGainControlMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, peakThreshGainControlMode, &IIOWidget::readAsync);
	}

	// Reset On RXON - Checkbox
	auto resetOnRxon = Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,rxagc-agc-reset-on-rxon",
								       "AGC Reset On RXON");
	if(resetOnRxon) {
		layout->addWidget(resetOnRxon);
		resetOnRxon->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, resetOnRxon, &IIOWidget::readAsync);
	}

	// Enable Sync Pulse For Gain Counter - Checkbox
	auto enableSyncPulseForGainCounter = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-agc-enable-sync-pulse-for-gain-counter", "AGC Enable Sync Pulse For Gain Counter");
	if(enableSyncPulseForGainCounter) {
		layout->addWidget(enableSyncPulseForGainCounter);
		enableSyncPulseForGainCounter->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, enableSyncPulseForGainCounter, &IIOWidget::readAsync);
	}

	// Enable IP3 Optimization Thresh - Checkbox
	auto enableIp3OptimizationThresh = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-agc-enable-ip3-optimization-thresh", "AGC Enable IP3 Optimization Thresh");
	if(enableIp3OptimizationThresh) {
		layout->addWidget(enableIp3OptimizationThresh);
		enableIp3OptimizationThresh->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, enableIp3OptimizationThresh, &IIOWidget::readAsync);
	}

	// IP3 Over Range Thresh - Range Widget [0 1 63]
	auto ip3OverRangeThresh = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-ip3-over-range-thresh",
									   "[0 1 63]", "IP3 Over Range Thresh");
	if(ip3OverRangeThresh) {
		layout->addWidget(ip3OverRangeThresh);
		connect(this, &AgcSetupWidget::readRequested, ip3OverRangeThresh, &IIOWidget::readAsync);
	}

	// IP3 Over Range Thresh Index - Range Widget [0 1 255]
	auto ip3OverRangeThreshIndex = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-ip3-over-range-thresh-index", "[0 1 255]", "IP3 Over Range Thresh Index");
	if(ip3OverRangeThreshIndex) {
		layout->addWidget(ip3OverRangeThreshIndex);
		connect(this, &AgcSetupWidget::readRequested, ip3OverRangeThreshIndex, &IIOWidget::readAsync);
	}

	// IP3 Peak Exceeded Count - Range Widget [0 1 255]
	auto ip3PeakExceededCnt = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-ip3-peak-exceeded-cnt",
									   "[0 1 255]", "IP3 Peak Exceeded Count");
	if(ip3PeakExceededCnt) {
		layout->addWidget(ip3PeakExceededCnt);
		connect(this, &AgcSetupWidget::readRequested, ip3PeakExceededCnt, &IIOWidget::readAsync);
	}

	// Enable Fast Recovery Loop - Checkbox
	auto enableFastRecoveryLoop = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-agc-enable-fast-recovery-loop", "AGC Enable Fast Recovery Loop");
	if(enableFastRecoveryLoop) {
		layout->addWidget(enableFastRecoveryLoop);
		connect(this, &AgcSetupWidget::readRequested, enableFastRecoveryLoop, &IIOWidget::readAsync);
	}

	return agcConfigSection;
}

QWidget *AgcSetupWidget::createAnalogPeakDetector(QWidget *parent)
{
	// 2. Analog Peak Detector section (29 attributes)
	MenuSectionCollapseWidget *apdSection = new MenuSectionCollapseWidget(
		"Analog Peak Detector", MenuCollapseSection::MHCW_ARROW, MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	apdSection->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// Under Range Low Interval (ns) - Range Widget [0 1 4294967295]
	auto underRangeLowInterval =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-peak-agc-under-range-low-interval_ns",
							 "[0 1 4294967295]", "Under Range Low Interval (ns)");
	if(underRangeLowInterval) {
		layout->addWidget(underRangeLowInterval);
	}

	// Under Range Mid Interval - Range Widget [0 1 63]
	auto underRangeMidInterval = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-agc-under-range-mid-interval", "[0 1 63]", "AGC Under Range Mid Interval");
	if(underRangeMidInterval) {
		layout->addWidget(underRangeMidInterval);
	}

	// Under Range High Interval - Range Widget [0 1 63]
	auto underRangeHighInterval = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-agc-under-range-high-interval", "[0 1 63]", "AGC Under Range High Interval");
	if(underRangeHighInterval) {
		layout->addWidget(underRangeHighInterval);
	}

	// APD High Thresh - Range Widget [7 1 49]
	auto apdHighThresh = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-peak-apd-high-thresh",
								      "[7 1 49]", "APD High Thresh");
	if(apdHighThresh) {
		layout->addWidget(apdHighThresh);
	}

	// APD Low Gain Mode High Thresh - Range Widget [7 1 49]
	auto apdLowGainModeHighThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-apd-low-gain-mode-high-thresh", "[7 1 49]", "APD Low Gain Mode High Thresh");
	if(apdLowGainModeHighThresh) {
		layout->addWidget(apdLowGainModeHighThresh);
	}

	// APD Low Thresh - Range Widget [7 1 49]
	auto apdLowThresh = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-peak-apd-low-thresh",
								     "[7 1 49]", "APD Low Thresh");
	if(apdLowThresh) {
		layout->addWidget(apdLowThresh);
	}

	// APD Low Gain Mode Low Thresh - Range Widget [7 1 49]
	auto apdLowGainModeLowThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-apd-low-gain-mode-low-thresh", "[7 1 49]", "APD Low Gain Mode Low Thresh");
	if(apdLowGainModeLowThresh) {
		layout->addWidget(apdLowGainModeLowThresh);
	}

	// APD Upper Thresh Peak Exceeded Count - Range Widget [0 1 255]
	auto apdUpperThreshPeakExceededCnt =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-peak-apd-upper-thresh-peak-exceeded-cnt",
							 "[0 1 255]", "APD Upper Thresh Peak Exceeded Count");
	if(apdUpperThreshPeakExceededCnt) {
		layout->addWidget(apdUpperThreshPeakExceededCnt);
	}

	// APD Lower Thresh Peak Exceeded Count - Range Widget [0 1 255]
	auto apdLowerThreshPeakExceededCnt =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-peak-apd-lower-thresh-peak-exceeded-cnt",
							 "[0 1 255]", "APD Lower Thresh Peak Exceeded Count");
	if(apdLowerThreshPeakExceededCnt) {
		layout->addWidget(apdLowerThreshPeakExceededCnt);
	}

	// APD Gain Step Attack - Range Widget [0 1 31]
	auto apdGainStepAttack = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-apd-gain-step-attack", "[0 1 31]", "APD Gain Step Attack");
	if(apdGainStepAttack) {
		layout->addWidget(apdGainStepAttack);
	}

	// APD Gain Step Recovery - Range Widget [0 1 31]
	auto apdGainStepRecovery = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-apd-gain-step-recovery", "[0 1 31]", "APD Gain Step Recovery");
	if(apdGainStepRecovery) {
		layout->addWidget(apdGainStepRecovery);
		connect(this, &AgcSetupWidget::readRequested, apdGainStepRecovery, &IIOWidget::readAsync);
	}

	// Enable HB2 Overload - Checkbox
	auto enableHb2Overload = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-peak-enable-hb2-overload", "Enable HB2 Overload");
	if(enableHb2Overload) {
		layout->addWidget(enableHb2Overload);
		enableHb2Overload->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, enableHb2Overload, &IIOWidget::readAsync);
	}

	// HB2 Overload Duration Count - Range Widget [0 1 6]
	auto hb2OverloadDurationCnt = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-hb2-overload-duration-cnt", "[0 1 6]", "HB2 Overload Duration Count");
	if(hb2OverloadDurationCnt) {
		layout->addWidget(hb2OverloadDurationCnt);
		connect(this, &AgcSetupWidget::readRequested, hb2OverloadDurationCnt, &IIOWidget::readAsync);
	}

	// HB2 Overload Thresh Count - Range Widget [1 1 15]
	auto hb2OverloadThreshCnt = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-hb2-overload-thresh-cnt", "[1 1 15]", "HB2 Overload Thresh Count");
	if(hb2OverloadThreshCnt) {
		layout->addWidget(hb2OverloadThreshCnt);
		connect(this, &AgcSetupWidget::readRequested, hb2OverloadThreshCnt, &IIOWidget::readAsync);
	}

	// HB2 High Thresh - Range Widget [0 1 255]
	auto hb2HighThresh = Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-peak-hb2-high-thresh",
								      "[0 1 255]", "HB2 High Thresh");
	if(hb2HighThresh) {
		layout->addWidget(hb2HighThresh);
		connect(this, &AgcSetupWidget::readRequested, hb2HighThresh, &IIOWidget::readAsync);
	}

	// HB2 Under Range Low Thresh - Range Widget [0 1 255]
	auto hb2UnderRangeLowThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-hb2-under-range-low-thresh", "[0 1 255]", "HB2 Under Range Low Thresh");
	if(hb2UnderRangeLowThresh) {
		layout->addWidget(hb2UnderRangeLowThresh);
		connect(this, &AgcSetupWidget::readRequested, hb2UnderRangeLowThresh, &IIOWidget::readAsync);
	}

	// HB2 Under Range Mid Thresh - Range Widget [0 1 255]
	auto hb2UnderRangeMidThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-hb2-under-range-mid-thresh", "[0 1 255]", "HB2 Under Range Mid Thresh");
	if(hb2UnderRangeMidThresh) {
		layout->addWidget(hb2UnderRangeMidThresh);
		connect(this, &AgcSetupWidget::readRequested, hb2UnderRangeMidThresh, &IIOWidget::readAsync);
	}

	// HB2 Under Range High Thresh - Range Widget [0 1 255]
	auto hb2UnderRangeHighThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-hb2-under-range-high-thresh", "[0 1 255]", "HB2 Under Range High Thresh");
	if(hb2UnderRangeHighThresh) {
		layout->addWidget(hb2UnderRangeHighThresh);
		connect(this, &AgcSetupWidget::readRequested, hb2UnderRangeHighThresh, &IIOWidget::readAsync);
	}

	// HB2 Upper Thresh Peak Exceeded Count - Range Widget [0 1 255]
	auto hb2UpperThreshPeakExceededCnt =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-peak-hb2-upper-thresh-peak-exceeded-cnt",
							 "[0 1 255]", "HB2 Upper Thresh Peak Exceeded Count");
	if(hb2UpperThreshPeakExceededCnt) {
		layout->addWidget(hb2UpperThreshPeakExceededCnt);
		connect(this, &AgcSetupWidget::readRequested, hb2UpperThreshPeakExceededCnt, &IIOWidget::readAsync);
	}

	// HB2 Lower Thresh Peak Exceeded Count - Range Widget [0 1 255]
	auto hb2LowerThreshPeakExceededCnt =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-peak-hb2-lower-thresh-peak-exceeded-cnt",
							 "[0 1 255]", "HB2 Lower Thresh Peak Exceeded Count");
	if(hb2LowerThreshPeakExceededCnt) {
		layout->addWidget(hb2LowerThreshPeakExceededCnt);
		connect(this, &AgcSetupWidget::readRequested, hb2LowerThreshPeakExceededCnt, &IIOWidget::readAsync);
	}

	// HB2 Gain Step High Recovery - Range Widget [0 1 31]
	auto hb2GainStepHighRecovery = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-hb2-gain-step-high-recovery", "[0 1 31]", "HB2 Gain Step High Recovery");
	if(hb2GainStepHighRecovery) {
		layout->addWidget(hb2GainStepHighRecovery);
		connect(this, &AgcSetupWidget::readRequested, hb2GainStepHighRecovery, &IIOWidget::readAsync);
	}

	// HB2 Gain Step Low Recovery - Range Widget [0 1 31]
	auto hb2GainStepLowRecovery = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-hb2-gain-step-low-recovery", "[0 1 31]", "HB2 Gain Step Low Recovery");
	if(hb2GainStepLowRecovery) {
		layout->addWidget(hb2GainStepLowRecovery);
		connect(this, &AgcSetupWidget::readRequested, hb2GainStepLowRecovery, &IIOWidget::readAsync);
	}

	// HB2 Gain Step Mid Recovery - Range Widget [0 1 31]
	auto hb2GainStepMidRecovery = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-hb2-gain-step-mid-recovery", "[0 1 31]", "HB2 Gain Step Mid Recovery");
	if(hb2GainStepMidRecovery) {
		layout->addWidget(hb2GainStepMidRecovery);
		connect(this, &AgcSetupWidget::readRequested, hb2GainStepMidRecovery, &IIOWidget::readAsync);
	}

	// HB2 Gain Step Attack - Range Widget [0 1 31]
	auto hb2GainStepAttack = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-peak-hb2-gain-step-attack", "[0 1 31]", "HB2 Gain Step Attack");
	if(hb2GainStepAttack) {
		layout->addWidget(hb2GainStepAttack);
		connect(this, &AgcSetupWidget::readRequested, hb2GainStepAttack, &IIOWidget::readAsync);
	}

	// HB2 Overload Power Mode - Checkbox
	auto hb2OverloadPowerMode = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-peak-hb2-overload-power-mode", "HB2 Overload Power Mode");
	if(hb2OverloadPowerMode) {
		layout->addWidget(hb2OverloadPowerMode);
		hb2OverloadPowerMode->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, hb2OverloadPowerMode, &IIOWidget::readAsync);
	}

	// HB2 OVRG Sel - Checkbox
	auto hb2OvrgSel =
		Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,rxagc-peak-hb2-ovrg-sel", "HB2 OVRG Sel");
	if(hb2OvrgSel) {
		layout->addWidget(hb2OvrgSel);
		hb2OvrgSel->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, hb2OvrgSel, &IIOWidget::readAsync);
	}

	// HB2 Thresh Config - Checkbox
	auto hb2ThreshConfig = Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,rxagc-peak-hb2-thresh-config",
									   "HB2 Thresh Config");
	if(hb2ThreshConfig) {
		layout->addWidget(hb2ThreshConfig);
		hb2ThreshConfig->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, hb2ThreshConfig, &IIOWidget::readAsync);
	}

	return apdSection;
}

QWidget *AgcSetupWidget::createPowerMeasurementDetector(QWidget *parent)
{
	// 3. Power Measurement Detector section (15 attributes)
	MenuSectionCollapseWidget *powerSection =
		new MenuSectionCollapseWidget("Power Measurement Detector", MenuCollapseSection::MHCW_ARROW,
					      MenuCollapseSection::MHW_BASEWIDGET, parent);

	QWidget *widget = new QWidget(parent);
	QVBoxLayout *layout = new QVBoxLayout(widget);
	layout->setContentsMargins(10, 10, 10, 10);
	layout->setSpacing(10);

	powerSection->contentLayout()->addWidget(widget);
	Style::setBackgroundColor(widget, json::theme::background_primary);
	Style::setStyle(widget, style::properties::widget::border_interactive);

	// Power Enable Measurement - Checkbox
	auto powerEnableMeasurement = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-power-power-enable-measurement", "Power Enable Measurement");
	if(powerEnableMeasurement) {
		layout->addWidget(powerEnableMeasurement);
		powerEnableMeasurement->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, powerEnableMeasurement, &IIOWidget::readAsync);
	}

	// Power Use RFIR Out - Checkbox
	auto powerUseRfirOut = Adrv9009WidgetFactory::createCheckboxWidget(
		m_device, "adi,rxagc-power-power-use-rfir-out", "Power Use RFIR Out");
	if(powerUseRfirOut) {
		layout->addWidget(powerUseRfirOut);
		powerUseRfirOut->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, powerUseRfirOut, &IIOWidget::readAsync);
	}

	// Power Use BBDC2 - Checkbox
	auto powerUseBbdc2 = Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,rxagc-power-power-use-bbdc2",
									 "Power Use BBDC2");
	if(powerUseBbdc2) {
		layout->addWidget(powerUseBbdc2);
		powerUseBbdc2->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, powerUseBbdc2, &IIOWidget::readAsync);
	}

	// Under Range High Power Thresh - Range Widget [0 1 127]
	auto underRangeHighPowerThresh =
		Adrv9009WidgetFactory::createRangeWidget(m_device, "adi,rxagc-power-under-range-high-power-thresh",
							 "[0 1 127]", "Under Range High Power Thresh");
	if(underRangeHighPowerThresh) {
		layout->addWidget(underRangeHighPowerThresh);
		connect(this, &AgcSetupWidget::readRequested, underRangeHighPowerThresh, &IIOWidget::readAsync);
	}

	// Under Range Low Power Thresh - Range Widget [0 1 31]
	auto underRangeLowPowerThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-power-under-range-low-power-thresh", "[0 1 31]", "Under Range Low Power Thresh");
	if(underRangeLowPowerThresh) {
		layout->addWidget(underRangeLowPowerThresh);
		connect(this, &AgcSetupWidget::readRequested, underRangeLowPowerThresh, &IIOWidget::readAsync);
	}

	// Under Range High Power Gain Step Recovery - Range Widget [0 1 31]
	auto underRangeHighPowerGainStepRecovery = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-power-under-range-high-power-gain-step-recovery", "[0 1 31]",
		"Under Range High Power Gain Step Recovery");
	if(underRangeHighPowerGainStepRecovery) {
		layout->addWidget(underRangeHighPowerGainStepRecovery);
		connect(this, &AgcSetupWidget::readRequested, underRangeHighPowerGainStepRecovery,
			&IIOWidget::readAsync);
	}

	// Under Range Low Power Gain Step Recovery - Range Widget [0 1 31]
	auto underRangeLowPowerGainStepRecovery = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-power-under-range-low-power-gain-step-recovery", "[0 1 31]",
		"Under Range Low Power Gain Step Recovery");
	if(underRangeLowPowerGainStepRecovery) {
		layout->addWidget(underRangeLowPowerGainStepRecovery);
		connect(this, &AgcSetupWidget::readRequested, underRangeLowPowerGainStepRecovery,
			&IIOWidget::readAsync);
	}

	// Power Measurement Duration - Range Widget [0 1 31]
	auto powerMeasurementDuration = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-power-power-measurement-duration", "[0 1 31]", "Power Measurement Duration");
	if(powerMeasurementDuration) {
		layout->addWidget(powerMeasurementDuration);
		connect(this, &AgcSetupWidget::readRequested, powerMeasurementDuration, &IIOWidget::readAsync);
	}

	// TDD Power Meas Duration - Range Widget [0 1 65535]
	auto durationWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-power-rx1-tdd-power-meas-duration", "[0 1 65535]", "RX1 TDD POWER MEAS DURATION");
	if(durationWidget) {
		layout->addWidget(durationWidget);
		connect(this, &AgcSetupWidget::readRequested, durationWidget, &IIOWidget::readAsync);
	}

	// TDD Power Meas Delay - Range Widget [0 1 65535]
	auto delayWidget = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-power-rx1-tdd-power-meas-delay", "[0 1 65535]", "RX1 TDD POWER MEAS DELAY");
	if(delayWidget) {
		layout->addWidget(delayWidget);
		connect(this, &AgcSetupWidget::readRequested, delayWidget, &IIOWidget::readAsync);
	}

	// Upper0 Power Thresh - Range Widget [0 1 127]
	auto upper0PowerThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-power-upper0-power-thresh", "[0 1 127]", "Upper0 Power Thresh");
	if(upper0PowerThresh) {
		layout->addWidget(upper0PowerThresh);
		connect(this, &AgcSetupWidget::readRequested, upper0PowerThresh, &IIOWidget::readAsync);
	}

	// Upper1 Power Thresh - Range Widget [0 1 15]
	auto upper1PowerThresh = Adrv9009WidgetFactory::createRangeWidget(
		m_device, "adi,rxagc-power-upper1-power-thresh", "[0 1 15]", "Upper1 Power Thresh");
	if(upper1PowerThresh) {
		layout->addWidget(upper1PowerThresh);
		connect(this, &AgcSetupWidget::readRequested, upper1PowerThresh, &IIOWidget::readAsync);
	}

	// Power Log Shift - Checkbox
	auto powerLogShift = Adrv9009WidgetFactory::createCheckboxWidget(m_device, "adi,rxagc-power-power-log-shift",
									 "Power Log Shift");
	if(powerLogShift) {
		layout->addWidget(powerLogShift);
		powerLogShift->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
		connect(this, &AgcSetupWidget::readRequested, powerLogShift, &IIOWidget::readAsync);
	}

	return powerSection;
}

QWidget *AgcSetupWidget::createAgcRxChannelGroup(const QString &baseAttr, const QString &displayName,
						 const QString &range, QWidget *parent)
{
	// Create container widget with proper Scopy styling
	QWidget *channelWidget = new QWidget(parent);
	Style::setStyle(channelWidget, style::properties::widget::border);

	// Create horizontal layout for RX1/RX2 columns
	QHBoxLayout *channelLayout = new QHBoxLayout(channelWidget);
	channelLayout->setContentsMargins(10, 10, 10, 10);
	channelLayout->setSpacing(20);

	// Add combined label with proper Scopy styling hierarchy
	QLabel *headerLabel = new QLabel(QString("RX1/RX2 %1").arg(displayName));
	Style::setStyle(headerLabel, style::properties::label::menuMedium);
	headerLabel->setAlignment(Qt::AlignCenter);
	channelLayout->addWidget(headerLabel);

	// Create RX1 widget
	QString rx1Attr = QString(baseAttr).arg(1);
	auto rx1Widget =
		Adrv9009WidgetFactory::createRangeWidget(m_device, rx1Attr, range, QString("RX1 %1").arg(displayName));
	if(rx1Widget) {
		channelLayout->addWidget(rx1Widget);
		connect(this, &AgcSetupWidget::readRequested, rx1Widget, &IIOWidget::readAsync);
	}

	// Create RX2 widget
	QString rx2Attr = QString(baseAttr).arg(2);
	auto rx2Widget =
		Adrv9009WidgetFactory::createRangeWidget(m_device, rx2Attr, range, QString("RX2 %1").arg(displayName));
	if(rx2Widget) {
		channelLayout->addWidget(rx2Widget);
		connect(this, &AgcSetupWidget::readRequested, rx2Widget, &IIOWidget::readAsync);
	}

	return channelWidget;
}
