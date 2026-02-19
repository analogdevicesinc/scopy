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
 *
 */

#include "ad936x_advanced_api.h"
#include "ad936xplugin.h"
#include "ad936x/ad963xadvanced.h"
#include <pluginbase/toolmenuentry.h>
#include <iio-widgets/iiowidgetgroup.h>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_AD936X_ADVANCED_API, "AD936X_ADVANCED_API")

using namespace scopy::ad936x;

AD936X_ADVANCED_API::AD936X_ADVANCED_API(Ad936xPlugin *plugin)
	: ApiObject()
	, m_plugin(plugin)
{}

AD936X_ADVANCED_API::~AD936X_ADVANCED_API() {}

// --- Private helpers ---

QString AD936X_ADVANCED_API::readFromWidget(const QString &key)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD936X_ADVANCED_API) << "Widget manager not available";
		return QString();
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD936X_ADVANCED_API) << "Widget not found for key:" << key;
		return QString();
	}

	QPair<QString, QString> result = widget->read();
	return result.first;
}

void AD936X_ADVANCED_API::writeToWidget(const QString &key, const QString &value)
{
	if(!m_plugin->m_widgetGroup) {
		qWarning(CAT_AD936X_ADVANCED_API) << "Widget manager not available";
		return;
	}

	IIOWidget *widget = m_plugin->m_widgetGroup->get(key);
	if(!widget) {
		qWarning(CAT_AD936X_ADVANCED_API) << "Widget not found for key:" << key;
		return;
	}

	widget->writeAsync(value);
}

// --- Tool management ---

QStringList AD936X_ADVANCED_API::getTools()
{
	QStringList tools;
	for(ToolMenuEntry *tool : m_plugin->m_toolList) {
		tools.append(tool->name());
	}
	return tools;
}

// --- ENSM Mode ---

QString AD936X_ADVANCED_API::isFddModeEnabled()
{
	return readFromWidget("ad9361-phy/adi,frequency-division-duplex-mode-enable");
}

void AD936X_ADVANCED_API::setFddModeEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,frequency-division-duplex-mode-enable", value);
}

QString AD936X_ADVANCED_API::isEnsmPinPulseModeEnabled()
{
	return readFromWidget("ad9361-phy/adi,ensm-enable-pin-pulse-mode-enable");
}

void AD936X_ADVANCED_API::setEnsmPinPulseModeEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,ensm-enable-pin-pulse-mode-enable", value);
}

QString AD936X_ADVANCED_API::isEnsmTxnrxControlEnabled()
{
	return readFromWidget("ad9361-phy/adi,ensm-enable-txnrx-control-enable");
}

void AD936X_ADVANCED_API::setEnsmTxnrxControlEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,ensm-enable-txnrx-control-enable", value);
}

QString AD936X_ADVANCED_API::isTddDualSynthModeEnabled()
{
	return readFromWidget("ad9361-phy/adi,tdd-use-dual-synth-mode-enable");
}

void AD936X_ADVANCED_API::setTddDualSynthModeEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,tdd-use-dual-synth-mode-enable", value);
}

QString AD936X_ADVANCED_API::isTddFddVcoTablesEnabled()
{
	return readFromWidget("ad9361-phy/adi,tdd-use-fdd-vco-tables-enable");
}

void AD936X_ADVANCED_API::setTddFddVcoTablesEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,tdd-use-fdd-vco-tables-enable", value);
}

QString AD936X_ADVANCED_API::isTddSkipVcoCalEnabled()
{
	return readFromWidget("ad9361-phy/adi,tdd-skip-vco-cal-enable");
}

void AD936X_ADVANCED_API::setTddSkipVcoCalEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,tdd-skip-vco-cal-enable", value);
}

QString AD936X_ADVANCED_API::isUpdateTxGainInAlertEnabled()
{
	return readFromWidget("ad9361-phy/adi,update-tx-gain-in-alert-enable");
}

void AD936X_ADVANCED_API::setUpdateTxGainInAlertEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,update-tx-gain-in-alert-enable", value);
}

// --- Mode ---

QString AD936X_ADVANCED_API::getRxRfPortInputSelect()
{
	return readFromWidget("ad9361-phy/adi,rx-rf-port-input-select");
}

void AD936X_ADVANCED_API::setRxRfPortInputSelect(const QString &value)
{
	writeToWidget("ad9361-phy/adi,rx-rf-port-input-select", value);
}

QString AD936X_ADVANCED_API::getTxRfPortInputSelect()
{
	return readFromWidget("ad9361-phy/adi,tx-rf-port-input-select");
}

void AD936X_ADVANCED_API::setTxRfPortInputSelect(const QString &value)
{
	writeToWidget("ad9361-phy/adi,tx-rf-port-input-select", value);
}

QString AD936X_ADVANCED_API::isRx1Rx2PhaseInversionEnabled()
{
	return readFromWidget("ad9361-phy/adi,rx1-rx2-phase-inversion-enable");
}

void AD936X_ADVANCED_API::setRx1Rx2PhaseInversionEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,rx1-rx2-phase-inversion-enable", value);
}

// --- Clocks ---

QString AD936X_ADVANCED_API::isXoDisableUseExtRefclkEnabled()
{
	return readFromWidget("ad9361-phy/adi,xo-disable-use-ext-refclk-enable");
}

void AD936X_ADVANCED_API::setXoDisableUseExtRefclkEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,xo-disable-use-ext-refclk-enable", value);
}

QString AD936X_ADVANCED_API::isExternalRxLoEnabled() { return readFromWidget("ad9361-phy/altvoltage0_out/external"); }

void AD936X_ADVANCED_API::setExternalRxLoEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/altvoltage0_out/external", value);
}

QString AD936X_ADVANCED_API::isExternalTxLoEnabled() { return readFromWidget("ad9361-phy/altvoltage1_out/external"); }

void AD936X_ADVANCED_API::setExternalTxLoEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/altvoltage1_out/external", value);
}

QString AD936X_ADVANCED_API::getClkOutputModeSelect()
{
	return readFromWidget("ad9361-phy/adi,clk-output-mode-select");
}

void AD936X_ADVANCED_API::setClkOutputModeSelect(const QString &value)
{
	writeToWidget("ad9361-phy/adi,clk-output-mode-select", value);
}

QString AD936X_ADVANCED_API::isRxFastlockPincontrolEnabled()
{
	return readFromWidget("ad9361-phy/adi,rx-fastlock-pincontrol-enable");
}

void AD936X_ADVANCED_API::setRxFastlockPincontrolEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,rx-fastlock-pincontrol-enable", value);
}

QString AD936X_ADVANCED_API::getRxFastlockDelay() { return readFromWidget("ad9361-phy/adi,rx-fastlock-delay-ns"); }

void AD936X_ADVANCED_API::setRxFastlockDelay(const QString &value)
{
	writeToWidget("ad9361-phy/adi,rx-fastlock-delay-ns", value);
}

QString AD936X_ADVANCED_API::isTxFastlockPincontrolEnabled()
{
	return readFromWidget("ad9361-phy/adi,tx-fastlock-pincontrol-enable");
}

void AD936X_ADVANCED_API::setTxFastlockPincontrolEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,tx-fastlock-pincontrol-enable", value);
}

QString AD936X_ADVANCED_API::getTxFastlockDelay() { return readFromWidget("ad9361-phy/adi,tx-fastlock-delay-ns"); }

void AD936X_ADVANCED_API::setTxFastlockDelay(const QString &value)
{
	writeToWidget("ad9361-phy/adi,tx-fastlock-delay-ns", value);
}

// --- eLNA ---

QString AD936X_ADVANCED_API::getElnaGain() { return readFromWidget("ad9361-phy/adi,elna-gain-mdB"); }

void AD936X_ADVANCED_API::setElnaGain(const QString &value) { writeToWidget("ad9361-phy/adi,elna-gain-mdB", value); }

QString AD936X_ADVANCED_API::getElnaBypassLoss() { return readFromWidget("ad9361-phy/adi,elna-bypass-loss-mdB"); }

void AD936X_ADVANCED_API::setElnaBypassLoss(const QString &value)
{
	writeToWidget("ad9361-phy/adi,elna-bypass-loss-mdB", value);
}

QString AD936X_ADVANCED_API::getElnaSettlingDelay() { return readFromWidget("ad9361-phy/adi,elna-settling-delay-ns"); }

void AD936X_ADVANCED_API::setElnaSettlingDelay(const QString &value)
{
	writeToWidget("ad9361-phy/adi,elna-settling-delay-ns", value);
}

QString AD936X_ADVANCED_API::isElnaRx1Gpo0ControlEnabled()
{
	return readFromWidget("ad9361-phy/adi,elna-rx1-gpo0-control-enable");
}

void AD936X_ADVANCED_API::setElnaRx1Gpo0ControlEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,elna-rx1-gpo0-control-enable", value);
}

QString AD936X_ADVANCED_API::isElnaRx2Gpo1ControlEnabled()
{
	return readFromWidget("ad9361-phy/adi,elna-rx2-gpo1-control-enable");
}

void AD936X_ADVANCED_API::setElnaRx2Gpo1ControlEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,elna-rx2-gpo1-control-enable", value);
}

QString AD936X_ADVANCED_API::isElnaGaintableAllIndexEnabled()
{
	return readFromWidget("ad9361-phy/adi,elna-gaintable-all-index-enable");
}

void AD936X_ADVANCED_API::setElnaGaintableAllIndexEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,elna-gaintable-all-index-enable", value);
}

// --- RSSI ---

QString AD936X_ADVANCED_API::getRssiDuration() { return readFromWidget("ad9361-phy/adi,rssi-duration"); }

void AD936X_ADVANCED_API::setRssiDuration(const QString &value)
{
	writeToWidget("ad9361-phy/adi,rssi-duration", value);
}

QString AD936X_ADVANCED_API::getRssiDelay() { return readFromWidget("ad9361-phy/adi,rssi-delay"); }

void AD936X_ADVANCED_API::setRssiDelay(const QString &value) { writeToWidget("ad9361-phy/adi,rssi-delay", value); }

QString AD936X_ADVANCED_API::getRssiWait() { return readFromWidget("ad9361-phy/adi,rssi-wait"); }

void AD936X_ADVANCED_API::setRssiWait(const QString &value) { writeToWidget("ad9361-phy/adi,rssi-wait", value); }

QString AD936X_ADVANCED_API::getRssiRestartMode() { return readFromWidget("ad9361-phy/adi,rssi-restart-mode"); }

void AD936X_ADVANCED_API::setRssiRestartMode(const QString &value)
{
	writeToWidget("ad9361-phy/adi,rssi-restart-mode", value);
}

// --- Gain Mode ---

QString AD936X_ADVANCED_API::getGcRx1Mode() { return readFromWidget("ad9361-phy/adi,gc-rx1-mode"); }

void AD936X_ADVANCED_API::setGcRx1Mode(const QString &value) { writeToWidget("ad9361-phy/adi,gc-rx1-mode", value); }

QString AD936X_ADVANCED_API::getGcRx2Mode() { return readFromWidget("ad9361-phy/adi,gc-rx2-mode"); }

void AD936X_ADVANCED_API::setGcRx2Mode(const QString &value) { writeToWidget("ad9361-phy/adi,gc-rx2-mode", value); }

QString AD936X_ADVANCED_API::isSplitGainTableModeEnabled()
{
	return readFromWidget("ad9361-phy/adi,split-gain-table-mode-enable");
}

void AD936X_ADVANCED_API::setSplitGainTableModeEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,split-gain-table-mode-enable", value);
}

QString AD936X_ADVANCED_API::getGcDecPowMeasurementDuration()
{
	return readFromWidget("ad9361-phy/adi,gc-dec-pow-measurement-duration");
}

void AD936X_ADVANCED_API::setGcDecPowMeasurementDuration(const QString &value)
{
	writeToWidget("ad9361-phy/adi,gc-dec-pow-measurement-duration", value);
}

QString AD936X_ADVANCED_API::getAgcAttackDelayExtraMargin()
{
	return readFromWidget("ad9361-phy/adi,agc-attack-delay-extra-margin-us");
}

void AD936X_ADVANCED_API::setAgcAttackDelayExtraMargin(const QString &value)
{
	writeToWidget("ad9361-phy/adi,agc-attack-delay-extra-margin-us", value);
}

// --- Gain AGC Thresholds ---

QString AD936X_ADVANCED_API::getAgcOuterThreshHigh() { return readFromWidget("ad9361-phy/adi,agc-outer-thresh-high"); }

void AD936X_ADVANCED_API::setAgcOuterThreshHigh(const QString &value)
{
	writeToWidget("ad9361-phy/adi,agc-outer-thresh-high", value);
}

QString AD936X_ADVANCED_API::getAgcInnerThreshHigh() { return readFromWidget("ad9361-phy/adi,agc-inner-thresh-high"); }

void AD936X_ADVANCED_API::setAgcInnerThreshHigh(const QString &value)
{
	writeToWidget("ad9361-phy/adi,agc-inner-thresh-high", value);
}

QString AD936X_ADVANCED_API::getAgcInnerThreshLow() { return readFromWidget("ad9361-phy/adi,agc-inner-thresh-low"); }

void AD936X_ADVANCED_API::setAgcInnerThreshLow(const QString &value)
{
	writeToWidget("ad9361-phy/adi,agc-inner-thresh-low", value);
}

QString AD936X_ADVANCED_API::getAgcOuterThreshLow() { return readFromWidget("ad9361-phy/adi,agc-outer-thresh-low"); }

void AD936X_ADVANCED_API::setAgcOuterThreshLow(const QString &value)
{
	writeToWidget("ad9361-phy/adi,agc-outer-thresh-low", value);
}

QString AD936X_ADVANCED_API::getAgcGainUpdateInterval()
{
	return readFromWidget("ad9361-phy/adi,agc-gain-update-interval-us");
}

void AD936X_ADVANCED_API::setAgcGainUpdateInterval(const QString &value)
{
	writeToWidget("ad9361-phy/adi,agc-gain-update-interval-us", value);
}

// --- TX Monitor ---

QString AD936X_ADVANCED_API::getTxmon1FrontEndGain() { return readFromWidget("ad9361-phy/adi,txmon-1-front-end-gain"); }

void AD936X_ADVANCED_API::setTxmon1FrontEndGain(const QString &value)
{
	writeToWidget("ad9361-phy/adi,txmon-1-front-end-gain", value);
}

QString AD936X_ADVANCED_API::getTxmon2FrontEndGain() { return readFromWidget("ad9361-phy/adi,txmon-2-front-end-gain"); }

void AD936X_ADVANCED_API::setTxmon2FrontEndGain(const QString &value)
{
	writeToWidget("ad9361-phy/adi,txmon-2-front-end-gain", value);
}

QString AD936X_ADVANCED_API::getTxmonLowHighThresh() { return readFromWidget("ad9361-phy/adi,txmon-low-high-thresh"); }

void AD936X_ADVANCED_API::setTxmonLowHighThresh(const QString &value)
{
	writeToWidget("ad9361-phy/adi,txmon-low-high-thresh", value);
}

QString AD936X_ADVANCED_API::getTxmonLowGain() { return readFromWidget("ad9361-phy/adi,txmon-low-gain"); }

void AD936X_ADVANCED_API::setTxmonLowGain(const QString &value)
{
	writeToWidget("ad9361-phy/adi,txmon-low-gain", value);
}

QString AD936X_ADVANCED_API::getTxmonHighGain() { return readFromWidget("ad9361-phy/adi,txmon-high-gain"); }

void AD936X_ADVANCED_API::setTxmonHighGain(const QString &value)
{
	writeToWidget("ad9361-phy/adi,txmon-high-gain", value);
}

QString AD936X_ADVANCED_API::isTxmonDcTrackingEnabled()
{
	return readFromWidget("ad9361-phy/adi,txmon-dc-tracking-enable");
}

void AD936X_ADVANCED_API::setTxmonDcTrackingEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,txmon-dc-tracking-enable", value);
}

QString AD936X_ADVANCED_API::isTxmonOneShotModeEnabled()
{
	return readFromWidget("ad9361-phy/adi,txmon-one-shot-mode-enable");
}

void AD936X_ADVANCED_API::setTxmonOneShotModeEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,txmon-one-shot-mode-enable", value);
}

// --- Temp Sensor ---

QString AD936X_ADVANCED_API::getTempSenseMeasurementInterval()
{
	return readFromWidget("ad9361-phy/adi,temp-sense-measurement-interval-ms");
}

void AD936X_ADVANCED_API::setTempSenseMeasurementInterval(const QString &value)
{
	writeToWidget("ad9361-phy/adi,temp-sense-measurement-interval-ms", value);
}

QString AD936X_ADVANCED_API::isTempSensePeriodicMeasurementEnabled()
{
	return readFromWidget("ad9361-phy/adi,temp-sense-periodic-measurement-enable");
}

void AD936X_ADVANCED_API::setTempSensePeriodicMeasurementEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,temp-sense-periodic-measurement-enable", value);
}

// --- MISC ---

QString AD936X_ADVANCED_API::getDcOffsetAttenuationHighRange()
{
	return readFromWidget("ad9361-phy/adi,dc-offset-attenuation-high-range");
}

void AD936X_ADVANCED_API::setDcOffsetAttenuationHighRange(const QString &value)
{
	writeToWidget("ad9361-phy/adi,dc-offset-attenuation-high-range", value);
}

QString AD936X_ADVANCED_API::getDcOffsetAttenuationLowRange()
{
	return readFromWidget("ad9361-phy/adi,dc-offset-attenuation-low-range");
}

void AD936X_ADVANCED_API::setDcOffsetAttenuationLowRange(const QString &value)
{
	writeToWidget("ad9361-phy/adi,dc-offset-attenuation-low-range", value);
}

QString AD936X_ADVANCED_API::isQecTrackingSlowModeEnabled()
{
	return readFromWidget("ad9361-phy/adi,qec-tracking-slow-mode-enable");
}

void AD936X_ADVANCED_API::setQecTrackingSlowModeEnabled(const QString &value)
{
	writeToWidget("ad9361-phy/adi,qec-tracking-slow-mode-enable", value);
}

// --- BIST ---

QString AD936X_ADVANCED_API::getBistPrbs() { return readFromWidget("ad9361-phy/bist_prbs"); }

void AD936X_ADVANCED_API::setBistPrbs(const QString &value) { writeToWidget("ad9361-phy/bist_prbs", value); }

QString AD936X_ADVANCED_API::getLoopback() { return readFromWidget("ad9361-phy/loopback"); }

void AD936X_ADVANCED_API::setLoopback(const QString &value) { writeToWidget("ad9361-phy/loopback", value); }

// --- Generic widget access ---

QStringList AD936X_ADVANCED_API::getWidgetKeys()
{
	if(!m_plugin->m_widgetGroup) {
		return QStringList();
	}
	return m_plugin->m_widgetGroup->keys();
}

QString AD936X_ADVANCED_API::readWidget(const QString &key) { return readFromWidget(key); }

void AD936X_ADVANCED_API::writeWidget(const QString &key, const QString &value) { writeToWidget(key, value); }

// --- Navigation ---

void AD936X_ADVANCED_API::switchSubtab(const QString &name)
{
	QWidget *tool = m_plugin->m_toolList[1]->tool();
	AD936XAdvanced *adv = qobject_cast<AD936XAdvanced *>(tool);
	if(adv) {
		adv->switchSubtab(name);
	}
}

// --- Utility ---

void AD936X_ADVANCED_API::refresh()
{
	if(!m_plugin->m_widgetGroup) {
		return;
	}

	auto widgets = m_plugin->m_widgetGroup->getAll();
	for(auto *widget : widgets) {
		widget->readAsync();
	}
}

#include "moc_ad936x_advanced_api.cpp"
