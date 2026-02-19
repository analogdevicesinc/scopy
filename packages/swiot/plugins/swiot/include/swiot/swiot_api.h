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

#ifndef SWIOT_API_H
#define SWIOT_API_H

#include "scopy-swiot_export.h"
#include <pluginbase/apiobject.h>
#include <QString>
#include <QStringList>

namespace scopy::swiot {

class SWIOTPlugin;
class SwiotConfig;
class Ad74413r;
class Max14906;
class Faults;

class SCOPY_SWIOT_EXPORT SWIOT_API : public ApiObject
{
	Q_OBJECT
public:
	explicit SWIOT_API(SWIOTPlugin *swiotPlugin);
	~SWIOT_API();

	// Tool selection
	Q_INVOKABLE QStringList getTools();
	Q_INVOKABLE bool isRuntimeMode();
	Q_INVOKABLE void switchToConfigMode();

	// Config instrument
	Q_INVOKABLE int getConfigChannelCount();
	Q_INVOKABLE void applyConfig();
	Q_INVOKABLE void setChannelEnabled(int channelId, bool enabled);
	Q_INVOKABLE bool isChannelEnabled(int channelId);
	Q_INVOKABLE void setChannelDevice(int channelId, const QString &device);
	Q_INVOKABLE QString getChannelDevice(int channelId);
	Q_INVOKABLE QStringList getAvailableDevices(int channelId);
	Q_INVOKABLE void setChannelFunction(int channelId, const QString &function);
	Q_INVOKABLE QString getChannelFunction(int channelId);
	Q_INVOKABLE QStringList getAvailableFunctions(int channelId);

	// AD74413R instrument
	Q_INVOKABLE bool isAdRunning();
	Q_INVOKABLE void setAdRunning(bool running);
	Q_INVOKABLE void adSingleShot();
	Q_INVOKABLE double getAdTimespan();
	Q_INVOKABLE void setAdTimespan(double timespan);
	Q_INVOKABLE void setAdPlotLabelsEnabled(bool enabled);
	Q_INVOKABLE bool isAdPlotLabelsEnabled();
	Q_INVOKABLE void setAdMeasurementsEnabled(bool enabled);
	Q_INVOKABLE bool isAdMeasurementsEnabled();
	Q_INVOKABLE void setAdPlotChannelEnabled(int idx, bool enabled);
	Q_INVOKABLE bool isAdChannelEnabled(int channelId);
	Q_INVOKABLE void setAdChannelSamplingFrequency(int channelId, int frequency);
	Q_INVOKABLE int getAdChannelSamplingFrequency(int channelId);
	Q_INVOKABLE void setAdChannelYMin(int channelId, double value);
	Q_INVOKABLE void setAdChannelYMax(int channelId, double value);
	Q_INVOKABLE double getAdChannelYMin(int channelId);
	Q_INVOKABLE double getAdChannelYMax(int channelId);
	Q_INVOKABLE int getAdChannelCount();
	Q_INVOKABLE QStringList getAdChannelNames();
	Q_INVOKABLE double getAdSampleRate();

	// AD74413R widget group (generic attribute access)
	Q_INVOKABLE QStringList getAdWidgetKeys();
	Q_INVOKABLE QString getAdWidgetValue(const QString &key);
	Q_INVOKABLE void setAdWidgetValue(const QString &key, const QString &value);

	// MAX14906 instrument
	Q_INVOKABLE bool isMaxRunning();
	Q_INVOKABLE void setMaxRunning(bool running);
	Q_INVOKABLE double getMaxTimespan();
	Q_INVOKABLE void setMaxTimespan(double timespan);
	Q_INVOKABLE void setMaxChannelOutput(int channelId, bool value);
	Q_INVOKABLE bool getMaxChannelOutput(int channelId);
	Q_INVOKABLE int getMaxChannelCount();
	Q_INVOKABLE QStringList getMaxChannelNames();
	Q_INVOKABLE double getMaxXAxisMin();
	Q_INVOKABLE double getMaxXAxisMax();

	// Faults instrument
	Q_INVOKABLE bool isFaultsRunning();
	Q_INVOKABLE void setFaultsRunning(bool running);
	Q_INVOKABLE void faultsSingleShot();
	Q_INVOKABLE void resetAdFaultsStored();
	Q_INVOKABLE void resetMaxFaultsStored();
	Q_INVOKABLE void clearAdFaultsSelection();
	Q_INVOKABLE void clearMaxFaultsSelection();
	Q_INVOKABLE QList<int> getAdActiveFaultBits();
	Q_INVOKABLE QList<int> getMaxActiveFaultBits();
	Q_INVOKABLE QList<int> getAdSelectedFaultBits();
	Q_INVOKABLE QList<int> getMaxSelectedFaultBits();
	Q_INVOKABLE QList<int> getAdStoredFaultBits();
	Q_INVOKABLE QList<int> getMaxStoredFaultBits();
	Q_INVOKABLE void selectAdFaultBit(int bitIndex);
	Q_INVOKABLE void selectMaxFaultBit(int bitIndex);
	Q_INVOKABLE bool isAdFaultsExplanationEnabled();
	Q_INVOKABLE void setAdFaultsExplanationEnabled(bool enabled);
	Q_INVOKABLE bool isMaxFaultsExplanationEnabled();
	Q_INVOKABLE void setMaxFaultsExplanationEnabled(bool enabled);

private:
	SwiotConfig *getConfigInstrument();
	Ad74413r *getAd74413rInstrument();
	Max14906 *getMax14906Instrument();
	Faults *getFaultsInstrument();

	SWIOTPlugin *m_swiotPlugin;
};
} // namespace scopy::swiot
#endif // SWIOT_API_H
