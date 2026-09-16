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

class QComboBox;

namespace scopy {
namespace adc {

class ADCPlugin;
class ADCInstrumentController;
class ADCTimeInstrumentController;
class ADCFFTInstrumentController;
class GRTimeChannelComponent;
class GRFFTChannelComponent;
class TimePlotComponentSettings;

/**
 * Scripting/MCP facade over the ADC plugin.
 *
 * Error reporting contract:
 *  - Every setter returns bool: true means the value was applied AND read back as
 *    requested. false means nothing was applied, or the target rejected/clamped the
 *    value. Never returns true on a silent no-op.
 *  - On failure, lastError() carries the reason. It is cleared at the start of every
 *    call, so it always describes the most recent one.
 *  - Getters keep their natural return type but yield a sentinel when the lookup
 *    fails: -1 for int, NaN for double, empty list for QStringList. Sentinels are
 *    ambiguous by nature (a real value may collide), so check lastError() -- or
 *    equivalently ok() -- rather than comparing against the sentinel.
 *
 * Writes are applied by driving the same GUI widgets a user would, so the menus,
 * dock wrappers, signal paths and plots all stay in sync.
 */
class SCOPY_ADC_EXPORT ADC_API : public ApiObject
{
	Q_OBJECT
public:
	explicit ADC_API(ADCPlugin *adcPlugin);
	~ADC_API();

	// ==================== ERROR REPORTING ====================

	/// Reason the most recent call failed; empty when it succeeded.
	Q_INVOKABLE QString lastError() const;
	/// True when the most recent call succeeded.
	Q_INVOKABLE bool ok() const;

	// Tool listing
	Q_INVOKABLE QStringList getTools();

	// ==================== TIME DOMAIN ====================

	// Run control
	Q_INVOKABLE bool isTimeRunning();
	Q_INVOKABLE bool setTimeRunning(bool running);
	Q_INVOKABLE bool timeSingleShot();

	// Buffer/Plot settings
	Q_INVOKABLE int getTimeBufferSize();
	Q_INVOKABLE bool setTimeBufferSize(int size);
	Q_INVOKABLE int getTimePlotSize();
	Q_INVOKABLE bool setTimePlotSize(int size);
	Q_INVOKABLE bool isTimeSyncBufferPlot();
	Q_INVOKABLE bool setTimeSyncBufferPlot(bool sync);
	Q_INVOKABLE bool isTimeRollingMode();
	Q_INVOKABLE bool setTimeRollingMode(bool rolling);
	Q_INVOKABLE double getTimeSampleRate();
	Q_INVOKABLE bool setTimeSampleRate(double rate);

	// Single Y mode
	Q_INVOKABLE bool isTimeSingleYMode();
	Q_INVOKABLE bool setTimeSingleYMode(bool single);

	// Channels
	Q_INVOKABLE QStringList getTimeChannels();
	Q_INVOKABLE bool isTimeChannelEnabled(const QString &channel);
	Q_INVOKABLE bool setTimeChannelEnabled(const QString &channel, bool enabled);

	// Per-channel Y-mode (0=COUNT, 1=FS, 2=SCALE, 3=SCALE_OVERRIDE).
	// This is the YMODE combo in an individual channel's own menu, NOT the one in the
	// instrument's general settings -- for that use get/setTimeYMode below.
	Q_INVOKABLE int getTimeChannelYMode(const QString &channel);
	Q_INVOKABLE bool setTimeChannelYMode(const QString &channel, int mode);

	// Channel Y-axis
	Q_INVOKABLE double getTimeChannelYMin(const QString &channel);
	Q_INVOKABLE double getTimeChannelYMax(const QString &channel);
	Q_INVOKABLE bool setTimeChannelYMin(const QString &channel, double min);
	Q_INVOKABLE bool setTimeChannelYMax(const QString &channel, double max);

	// Cursors
	Q_INVOKABLE bool isTimeCursorsEnabled();
	Q_INVOKABLE bool setTimeCursorsEnabled(bool enabled);

	// Measurements
	Q_INVOKABLE QStringList getTimeMeasurements(const QString &channel);
	Q_INVOKABLE bool enableTimeMeasurement(const QString &channel, const QString &measurement);
	Q_INVOKABLE bool disableTimeMeasurement(const QString &channel, const QString &measurement);
	Q_INVOKABLE double getTimeMeasurementValue(const QString &channel, const QString &measurement);

	// ==================== FREQUENCY DOMAIN ====================

	// Run control
	Q_INVOKABLE bool isFreqRunning();
	Q_INVOKABLE bool setFreqRunning(bool running);
	Q_INVOKABLE bool freqSingleShot();

	// Settings
	Q_INVOKABLE int getFreqBufferSize();
	Q_INVOKABLE bool setFreqBufferSize(int size);
	Q_INVOKABLE double getFreqSampleRate();
	Q_INVOKABLE bool setFreqSampleRate(double rate);
	Q_INVOKABLE double getFreqOffset();
	Q_INVOKABLE bool setFreqOffset(double offset);

	// Complex mode
	Q_INVOKABLE bool isFreqComplexMode();
	Q_INVOKABLE bool setFreqComplexMode(bool complex);

	// Channels
	Q_INVOKABLE QStringList getFreqChannels();
	Q_INVOKABLE QStringList getComplexChannels();
	Q_INVOKABLE bool isFreqChannelEnabled(const QString &channel);
	Q_INVOKABLE bool setFreqChannelEnabled(const QString &channel, bool enabled);

	// FFT settings (applied to all channels)
	Q_INVOKABLE int getFreqWindow();
	Q_INVOKABLE bool setFreqWindow(int window);
	Q_INVOKABLE double getFreqPowerOffset();
	Q_INVOKABLE bool setFreqPowerOffset(double offset);

	// Channel Y-axis
	Q_INVOKABLE double getFreqChannelYMin(const QString &channel);
	Q_INVOKABLE double getFreqChannelYMax(const QString &channel);
	Q_INVOKABLE bool setFreqChannelYMin(const QString &channel, double min);
	Q_INVOKABLE bool setFreqChannelYMax(const QString &channel, double max);

	// Markers
	Q_INVOKABLE bool isFreqChannelMarkerEnabled(const QString &channel);
	Q_INVOKABLE bool setFreqChannelMarkerEnabled(const QString &channel, bool enabled);
	Q_INVOKABLE int getFreqChannelMarkerCount(const QString &channel);
	Q_INVOKABLE bool setFreqChannelMarkerCount(const QString &channel, int count);
	Q_INVOKABLE int getFreqChannelMarkerType(const QString &channel);
	Q_INVOKABLE bool setFreqChannelMarkerType(const QString &channel, int type);

	// Cursors
	Q_INVOKABLE bool isFreqCursorsEnabled();
	Q_INVOKABLE bool setFreqCursorsEnabled(bool enabled);

	// ==================== AUTOSCALE ====================

	Q_INVOKABLE bool isTimeAutoscaleEnabled();
	Q_INVOKABLE bool setTimeAutoscaleEnabled(bool enabled);
	Q_INVOKABLE bool isFreqAutoscaleEnabled();
	Q_INVOKABLE bool setFreqAutoscaleEnabled(bool enabled);

	// ==================== X-MODE ====================

	// XMode: 0=Samples, 1=Time/Frequency, 2=Override
	Q_INVOKABLE int getTimeXMode();
	Q_INVOKABLE bool setTimeXMode(int mode);
	Q_INVOKABLE int getFreqXMode();
	Q_INVOKABLE bool setFreqXMode(int mode);

	// ==================== Y-MODE ====================

	// The YMODE combo in the time instrument's general settings, alongside XMODE.
	// Setting it cascades to every channel and rescales the plot's Y axis, which is
	// what setTimeChannelYMode (a single channel's own menu) does not do.
	// YMode: 0=COUNT, 1=FS, 2=SCALE. SCALE is only offered when every channel
	// exposes a scale attribute.
	Q_INVOKABLE int getTimeYMode();
	Q_INVOKABLE bool setTimeYMode(int mode);

	// ==================== CHANNEL AVERAGING (FFT) ====================

	Q_INVOKABLE bool isFreqChannelAveragingEnabled(const QString &channel);
	Q_INVOKABLE bool setFreqChannelAveragingEnabled(const QString &channel, bool enabled);
	Q_INVOKABLE int getFreqChannelAveragingSize(const QString &channel);
	Q_INVOKABLE bool setFreqChannelAveragingSize(const QString &channel, int size);

	// ==================== WATERFALL ====================

	Q_INVOKABLE bool isWaterfallEnabled();
	Q_INVOKABLE bool setWaterfallEnabled(bool enabled);
	Q_INVOKABLE int getWaterfallHistory();
	Q_INVOKABLE bool setWaterfallHistory(int rows);

	// ==================== GENALYZER ====================

	Q_INVOKABLE bool isGenalyzerEnabled();
	Q_INVOKABLE bool setGenalyzerEnabled(bool enabled);
	Q_INVOKABLE int getGenalyzerMode();
	Q_INVOKABLE bool setGenalyzerMode(int mode);
	Q_INVOKABLE int getGenalyzerSSBWidth();
	Q_INVOKABLE bool setGenalyzerSSBWidth(int width);
	Q_INVOKABLE double getGenalyzerExpectedFreq();
	Q_INVOKABLE bool setGenalyzerExpectedFreq(double freq);
	Q_INVOKABLE int getGenalyzerHarmonicOrder();
	Q_INVOKABLE bool setGenalyzerHarmonicOrder(int order);
	Q_INVOKABLE int getGenalyzerSSBFundamental();
	Q_INVOKABLE bool setGenalyzerSSBFundamental(int ssb);
	Q_INVOKABLE int getGenalyzerSSBDefault();
	Q_INVOKABLE bool setGenalyzerSSBDefault(int ssb);
	Q_INVOKABLE QStringList getGenalyzerMetrics(const QString &channel);
	Q_INVOKABLE double getGenalyzerMetric(const QString &channel, const QString &metric);
	Q_INVOKABLE bool triggerGenalyzerAnalysis(const QString &channel);

private:
	// --- error plumbing ---
	/// Clears lastError. Called at the top of every public method.
	void clearError();
	/// Records a failure reason (also logged) and returns false so callers can `return fail(...)`.
	bool fail(const QString &reason) const;
	/// Records a failure reason and returns the int sentinel (-1).
	int failInt(const QString &reason) const;
	/// Records a failure reason and returns the double sentinel (NaN).
	double failDouble(const QString &reason) const;

	// --- readback verification ---
	/// Verifies an applied bool, failing with a descriptive message when it did not stick.
	bool verify(const QString &what, bool actual, bool expected) const;
	/// Verifies an applied int.
	bool verify(const QString &what, int actual, int expected) const;
	/// Verifies an applied double, tolerating the rounding a spinbox may introduce.
	bool verify(const QString &what, double actual, double expected) const;

	// --- lookups; all report via fail() when they return nullptr ---
	ADCTimeInstrumentController *getTimeController();
	ADCFFTInstrumentController *getFreqController();
	/// The time instrument's plot settings widget, which hosts the general-settings
	/// Y-AXIS controls (autoscale, Y range, YMODE).
	TimePlotComponentSettings *getTimePlotSettings();
	GRTimeChannelComponent *findTimeChannel(const QString &name);
	/// Finds any FFT channel -- real or complex -- so every name returned by
	/// getFreqChannels()/getComplexChannels() is accepted by every setter.
	GRFFTChannelComponent *findFreqChannel(const QString &name);
	/// Finds an FFT channel and requires it to be complex (genalyzer-only operations).
	GRFFTChannelComponent *findComplexChannel(const QString &name);

	/// Lists channel names of the requested kind, for "no such channel" messages.
	QStringList freqChannelNames();

	/// Renders a combo's entries as "value=label, ..." for "invalid mode" messages. The
	/// valid set is runtime dependent for both XMODE and YMODE, so it has to be read off
	/// the widget rather than assumed from the enum range.
	static QString comboValues(const QComboBox *cb);

	ADCPlugin *m_adcPlugin;
	mutable QString m_lastError;
};

} // namespace adc
} // namespace scopy

#endif // ADC_API_H
