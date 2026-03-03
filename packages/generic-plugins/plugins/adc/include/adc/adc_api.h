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

#ifndef ADC_API_H
#define ADC_API_H

#include "scopy-adc_export.h"
#include <pluginbase/apiobject.h>
#include <QStringList>

namespace scopy {
namespace adc {

class ADCPlugin;
class ADCTimeInstrumentController;
class ADCFFTInstrumentController;
class GRTimeChannelComponent;
class GRFFTChannelComponent;

class SCOPY_ADC_EXPORT ADC_API : public ApiObject
{
	Q_OBJECT
public:
	explicit ADC_API(ADCPlugin *adcPlugin);
	~ADC_API();

	// Tool listing
	Q_INVOKABLE QStringList getTools();

	// ==================== TIME DOMAIN ====================

	// Run control
	Q_INVOKABLE bool isTimeRunning();
	Q_INVOKABLE void setTimeRunning(bool running);
	Q_INVOKABLE void timeSingleShot();

	// Buffer/Plot settings
	Q_INVOKABLE int getTimeBufferSize();
	Q_INVOKABLE void setTimeBufferSize(int size);
	Q_INVOKABLE int getTimePlotSize();
	Q_INVOKABLE void setTimePlotSize(int size);
	Q_INVOKABLE bool isTimeSyncBufferPlot();
	Q_INVOKABLE void setTimeSyncBufferPlot(bool sync);
	Q_INVOKABLE bool isTimeRollingMode();
	Q_INVOKABLE void setTimeRollingMode(bool rolling);
	Q_INVOKABLE double getTimeSampleRate();
	Q_INVOKABLE void setTimeSampleRate(double rate);

	// Single Y mode
	Q_INVOKABLE bool isTimeSingleYMode();
	Q_INVOKABLE void setTimeSingleYMode(bool single);

	// Channels
	Q_INVOKABLE QStringList getTimeChannels();
	Q_INVOKABLE bool isTimeChannelEnabled(const QString &channel);
	Q_INVOKABLE void setTimeChannelEnabled(const QString &channel, bool enabled);

	// Channel Y-mode (0=COUNT, 1=FS, 2=SCALE, 3=SCALE_OVERRIDE)
	Q_INVOKABLE int getTimeChannelYMode(const QString &channel);
	Q_INVOKABLE void setTimeChannelYMode(const QString &channel, int mode);

	// Channel Y-axis
	Q_INVOKABLE double getTimeChannelYMin(const QString &channel);
	Q_INVOKABLE double getTimeChannelYMax(const QString &channel);
	Q_INVOKABLE void setTimeChannelYMin(const QString &channel, double min);
	Q_INVOKABLE void setTimeChannelYMax(const QString &channel, double max);

	// Cursors
	Q_INVOKABLE bool isTimeCursorsEnabled();
	Q_INVOKABLE void setTimeCursorsEnabled(bool enabled);

	// Measurements
	Q_INVOKABLE QStringList getTimeMeasurements(const QString &channel);
	Q_INVOKABLE void enableTimeMeasurement(const QString &channel, const QString &measurement);
	Q_INVOKABLE void disableTimeMeasurement(const QString &channel, const QString &measurement);
	Q_INVOKABLE double getTimeMeasurementValue(const QString &channel, const QString &measurement);

	// ==================== FREQUENCY DOMAIN ====================

	// Run control
	Q_INVOKABLE bool isFreqRunning();
	Q_INVOKABLE void setFreqRunning(bool running);
	Q_INVOKABLE void freqSingleShot();

	// Settings
	Q_INVOKABLE int getFreqBufferSize();
	Q_INVOKABLE void setFreqBufferSize(int size);
	Q_INVOKABLE double getFreqSampleRate();
	Q_INVOKABLE void setFreqSampleRate(double rate);
	Q_INVOKABLE double getFreqOffset();
	Q_INVOKABLE void setFreqOffset(double offset);

	// Complex mode
	Q_INVOKABLE bool isFreqComplexMode();
	Q_INVOKABLE void setFreqComplexMode(bool complex);

	// Channels
	Q_INVOKABLE QStringList getFreqChannels();
	Q_INVOKABLE QStringList getComplexChannels();
	Q_INVOKABLE bool isFreqChannelEnabled(const QString &channel);
	Q_INVOKABLE void setFreqChannelEnabled(const QString &channel, bool enabled);

	// FFT settings (applied to all channels)
	Q_INVOKABLE int getFreqWindow();
	Q_INVOKABLE void setFreqWindow(int window);
	Q_INVOKABLE double getFreqPowerOffset();
	Q_INVOKABLE void setFreqPowerOffset(double offset);

	// Channel Y-axis
	Q_INVOKABLE double getFreqChannelYMin(const QString &channel);
	Q_INVOKABLE double getFreqChannelYMax(const QString &channel);
	Q_INVOKABLE void setFreqChannelYMin(const QString &channel, double min);
	Q_INVOKABLE void setFreqChannelYMax(const QString &channel, double max);

	// Markers
	Q_INVOKABLE bool isFreqChannelMarkerEnabled(const QString &channel);
	Q_INVOKABLE void setFreqChannelMarkerEnabled(const QString &channel, bool enabled);
	Q_INVOKABLE void setFreqChannelMarkerCount(const QString &channel, int count);
	Q_INVOKABLE void setFreqChannelMarkerType(const QString &channel, int type);

	// Cursors
	Q_INVOKABLE bool isFreqCursorsEnabled();
	Q_INVOKABLE void setFreqCursorsEnabled(bool enabled);

	// ==================== XY PLOT ====================

	Q_INVOKABLE bool isTimeXYPlotEnabled();
	Q_INVOKABLE void setTimeXYPlotEnabled(bool enabled);
	Q_INVOKABLE QString getTimeXYSource();
	Q_INVOKABLE void setTimeXYSource(const QString &channel);

	// ==================== AUTOSCALE ====================

	Q_INVOKABLE bool isTimeAutoscaleEnabled();
	Q_INVOKABLE void setTimeAutoscaleEnabled(bool enabled);
	Q_INVOKABLE bool isFreqAutoscaleEnabled();
	Q_INVOKABLE void setFreqAutoscaleEnabled(bool enabled);

	// ==================== X-MODE ====================

	// XMode: 0=Samples, 1=Time/Frequency, 2=Override
	Q_INVOKABLE int getTimeXMode();
	Q_INVOKABLE void setTimeXMode(int mode);
	Q_INVOKABLE int getFreqXMode();
	Q_INVOKABLE void setFreqXMode(int mode);

	// ==================== CHANNEL AVERAGING (FFT) ====================

	Q_INVOKABLE bool isFreqChannelAveragingEnabled(const QString &channel);
	Q_INVOKABLE void setFreqChannelAveragingEnabled(const QString &channel, bool enabled);
	Q_INVOKABLE int getFreqChannelAveragingSize(const QString &channel);
	Q_INVOKABLE void setFreqChannelAveragingSize(const QString &channel, int size);

	// ==================== GENALYZER ====================

	Q_INVOKABLE bool isGenalyzerEnabled();
	Q_INVOKABLE void setGenalyzerEnabled(bool enabled);
	Q_INVOKABLE int getGenalyzerMode();
	Q_INVOKABLE void setGenalyzerMode(int mode);
	Q_INVOKABLE int getGenalyzerSSBWidth();
	Q_INVOKABLE void setGenalyzerSSBWidth(int width);
	Q_INVOKABLE double getGenalyzerExpectedFreq();
	Q_INVOKABLE void setGenalyzerExpectedFreq(double freq);
	Q_INVOKABLE int getGenalyzerHarmonicOrder();
	Q_INVOKABLE void setGenalyzerHarmonicOrder(int order);
	Q_INVOKABLE void setGenalyzerSSBFundamental(int ssb);
	Q_INVOKABLE void setGenalyzerSSBDefault(int ssb);
	Q_INVOKABLE double getGenalyzerMetric(const QString &channel, const QString &metric);
	Q_INVOKABLE void triggerGenalyzerAnalysis(const QString &channel);

private:
	ADCTimeInstrumentController *getTimeController();
	ADCFFTInstrumentController *getFreqController();
	GRTimeChannelComponent *findTimeChannel(const QString &name);
	GRFFTChannelComponent *findFreqChannel(const QString &name);
	GRFFTChannelComponent *findComplexChannel(const QString &name);

	ADCPlugin *m_adcPlugin;
};

} // namespace adc
} // namespace scopy

#endif // ADC_API_H
