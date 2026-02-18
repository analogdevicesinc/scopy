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

#ifndef AD936X_ADVANCED_API_H
#define AD936X_ADVANCED_API_H

#include "scopy-ad936x_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::ad936x {

class Ad936xPlugin;

class SCOPY_AD936X_EXPORT AD936X_ADVANCED_API : public ApiObject
{
	Q_OBJECT
public:
	explicit AD936X_ADVANCED_API(Ad936xPlugin *plugin);
	~AD936X_ADVANCED_API();

	// Tool management
	Q_INVOKABLE QStringList getTools();

	// --- ENSM Mode ---
	Q_INVOKABLE QString isFddModeEnabled();
	Q_INVOKABLE void setFddModeEnabled(const QString &value);
	Q_INVOKABLE QString isEnsmPinPulseModeEnabled();
	Q_INVOKABLE void setEnsmPinPulseModeEnabled(const QString &value);
	Q_INVOKABLE QString isEnsmTxnrxControlEnabled();
	Q_INVOKABLE void setEnsmTxnrxControlEnabled(const QString &value);
	Q_INVOKABLE QString isTddDualSynthModeEnabled();
	Q_INVOKABLE void setTddDualSynthModeEnabled(const QString &value);
	Q_INVOKABLE QString isTddFddVcoTablesEnabled();
	Q_INVOKABLE void setTddFddVcoTablesEnabled(const QString &value);
	Q_INVOKABLE QString isTddSkipVcoCalEnabled();
	Q_INVOKABLE void setTddSkipVcoCalEnabled(const QString &value);
	Q_INVOKABLE QString isUpdateTxGainInAlertEnabled();
	Q_INVOKABLE void setUpdateTxGainInAlertEnabled(const QString &value);

	// --- Mode ---
	Q_INVOKABLE QString getRxRfPortInputSelect();
	Q_INVOKABLE void setRxRfPortInputSelect(const QString &value);
	Q_INVOKABLE QString getTxRfPortInputSelect();
	Q_INVOKABLE void setTxRfPortInputSelect(const QString &value);
	Q_INVOKABLE QString isRx1Rx2PhaseInversionEnabled();
	Q_INVOKABLE void setRx1Rx2PhaseInversionEnabled(const QString &value);

	// --- Clocks ---
	Q_INVOKABLE QString isXoDisableUseExtRefclkEnabled();
	Q_INVOKABLE void setXoDisableUseExtRefclkEnabled(const QString &value);
	Q_INVOKABLE QString isExternalRxLoEnabled();
	Q_INVOKABLE void setExternalRxLoEnabled(const QString &value);
	Q_INVOKABLE QString isExternalTxLoEnabled();
	Q_INVOKABLE void setExternalTxLoEnabled(const QString &value);
	Q_INVOKABLE QString getClkOutputModeSelect();
	Q_INVOKABLE void setClkOutputModeSelect(const QString &value);
	Q_INVOKABLE QString isRxFastlockPincontrolEnabled();
	Q_INVOKABLE void setRxFastlockPincontrolEnabled(const QString &value);
	Q_INVOKABLE QString getRxFastlockDelay();
	Q_INVOKABLE void setRxFastlockDelay(const QString &value);
	Q_INVOKABLE QString isTxFastlockPincontrolEnabled();
	Q_INVOKABLE void setTxFastlockPincontrolEnabled(const QString &value);
	Q_INVOKABLE QString getTxFastlockDelay();
	Q_INVOKABLE void setTxFastlockDelay(const QString &value);

	// --- eLNA ---
	Q_INVOKABLE QString getElnaGain();
	Q_INVOKABLE void setElnaGain(const QString &value);
	Q_INVOKABLE QString getElnaBypassLoss();
	Q_INVOKABLE void setElnaBypassLoss(const QString &value);
	Q_INVOKABLE QString getElnaSettlingDelay();
	Q_INVOKABLE void setElnaSettlingDelay(const QString &value);
	Q_INVOKABLE QString isElnaRx1Gpo0ControlEnabled();
	Q_INVOKABLE void setElnaRx1Gpo0ControlEnabled(const QString &value);
	Q_INVOKABLE QString isElnaRx2Gpo1ControlEnabled();
	Q_INVOKABLE void setElnaRx2Gpo1ControlEnabled(const QString &value);
	Q_INVOKABLE QString isElnaGaintableAllIndexEnabled();
	Q_INVOKABLE void setElnaGaintableAllIndexEnabled(const QString &value);

	// --- RSSI ---
	Q_INVOKABLE QString getRssiDuration();
	Q_INVOKABLE void setRssiDuration(const QString &value);
	Q_INVOKABLE QString getRssiDelay();
	Q_INVOKABLE void setRssiDelay(const QString &value);
	Q_INVOKABLE QString getRssiWait();
	Q_INVOKABLE void setRssiWait(const QString &value);
	Q_INVOKABLE QString getRssiRestartMode();
	Q_INVOKABLE void setRssiRestartMode(const QString &value);

	// --- Gain Mode ---
	Q_INVOKABLE QString getGcRx1Mode();
	Q_INVOKABLE void setGcRx1Mode(const QString &value);
	Q_INVOKABLE QString getGcRx2Mode();
	Q_INVOKABLE void setGcRx2Mode(const QString &value);
	Q_INVOKABLE QString isSplitGainTableModeEnabled();
	Q_INVOKABLE void setSplitGainTableModeEnabled(const QString &value);
	Q_INVOKABLE QString getGcDecPowMeasurementDuration();
	Q_INVOKABLE void setGcDecPowMeasurementDuration(const QString &value);
	Q_INVOKABLE QString getAgcAttackDelayExtraMargin();
	Q_INVOKABLE void setAgcAttackDelayExtraMargin(const QString &value);

	// --- Gain AGC Thresholds ---
	Q_INVOKABLE QString getAgcOuterThreshHigh();
	Q_INVOKABLE void setAgcOuterThreshHigh(const QString &value);
	Q_INVOKABLE QString getAgcInnerThreshHigh();
	Q_INVOKABLE void setAgcInnerThreshHigh(const QString &value);
	Q_INVOKABLE QString getAgcInnerThreshLow();
	Q_INVOKABLE void setAgcInnerThreshLow(const QString &value);
	Q_INVOKABLE QString getAgcOuterThreshLow();
	Q_INVOKABLE void setAgcOuterThreshLow(const QString &value);
	Q_INVOKABLE QString getAgcGainUpdateInterval();
	Q_INVOKABLE void setAgcGainUpdateInterval(const QString &value);

	// --- TX Monitor ---
	Q_INVOKABLE QString getTxmon1FrontEndGain();
	Q_INVOKABLE void setTxmon1FrontEndGain(const QString &value);
	Q_INVOKABLE QString getTxmon2FrontEndGain();
	Q_INVOKABLE void setTxmon2FrontEndGain(const QString &value);
	Q_INVOKABLE QString getTxmonLowHighThresh();
	Q_INVOKABLE void setTxmonLowHighThresh(const QString &value);
	Q_INVOKABLE QString getTxmonLowGain();
	Q_INVOKABLE void setTxmonLowGain(const QString &value);
	Q_INVOKABLE QString getTxmonHighGain();
	Q_INVOKABLE void setTxmonHighGain(const QString &value);
	Q_INVOKABLE QString isTxmonDcTrackingEnabled();
	Q_INVOKABLE void setTxmonDcTrackingEnabled(const QString &value);
	Q_INVOKABLE QString isTxmonOneShotModeEnabled();
	Q_INVOKABLE void setTxmonOneShotModeEnabled(const QString &value);

	// --- Temp Sensor ---
	Q_INVOKABLE QString getTempSenseMeasurementInterval();
	Q_INVOKABLE void setTempSenseMeasurementInterval(const QString &value);
	Q_INVOKABLE QString isTempSensePeriodicMeasurementEnabled();
	Q_INVOKABLE void setTempSensePeriodicMeasurementEnabled(const QString &value);

	// --- MISC ---
	Q_INVOKABLE QString getDcOffsetAttenuationHighRange();
	Q_INVOKABLE void setDcOffsetAttenuationHighRange(const QString &value);
	Q_INVOKABLE QString getDcOffsetAttenuationLowRange();
	Q_INVOKABLE void setDcOffsetAttenuationLowRange(const QString &value);
	Q_INVOKABLE QString isQecTrackingSlowModeEnabled();
	Q_INVOKABLE void setQecTrackingSlowModeEnabled(const QString &value);

	// --- BIST ---
	Q_INVOKABLE QString getBistPrbs();
	Q_INVOKABLE void setBistPrbs(const QString &value);
	Q_INVOKABLE QString getLoopback();
	Q_INVOKABLE void setLoopback(const QString &value);

	// --- Generic widget access ---
	Q_INVOKABLE QStringList getWidgetKeys();
	Q_INVOKABLE QString readWidget(const QString &key);
	Q_INVOKABLE void writeWidget(const QString &key, const QString &value);

	// Navigation
	Q_INVOKABLE void switchSubtab(const QString &name);

	// Utility
	Q_INVOKABLE void refresh();

private:
	QString readFromWidget(const QString &key);
	void writeToWidget(const QString &key, const QString &value);

	Ad936xPlugin *m_plugin;
};

} // namespace scopy::ad936x

#endif // AD936X_ADVANCED_API_H
