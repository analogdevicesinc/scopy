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

#ifndef ADALM2000OSC_API_H
#define ADALM2000OSC_API_H

#include "scopy-adalm2000_export.h"

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <pluginbase/apiobject.h>

namespace scopy::adalm2000 {
class M2kOscController;

// Traces are 0 (CH1), 1 (CH2), then math channels and references in the order added; the
// per-channel lists cover CH1 and CH2. Math formulas use X1 (CH1), X2 (CH2) and S (sample index).
// Trigger channel: 0 CH1, 1 CH2. Digital trigger: 0 none, 1 trigger in, 2 logic analyzer.
// Logic, with both on: 0 OR, 1 AND, 2 XOR, 3 NOR, 4 NAND, 5 XNOR. Condition: 0 rising,
// 1 falling, 2 low, 3 high; the external one adds 4 any edge. Trigger out: 0 none, 2 trigger in,
// 3 oscilloscope, 4 logic analyzer. Measurements are named by id ("period", "peak_peak", ...);
// `measurements` lists the shown ones as "trace:id".
class SCOPY_ADALM2000_EXPORT Adalm2000Osc_API : public ApiObject
{
	Q_OBJECT
	Q_PROPERTY(bool running READ running WRITE setRunning STORED false)
	Q_PROPERTY(int time_base_samples READ timeBaseSamples WRITE setTimeBaseSamples)
	Q_PROPERTY(double sample_rate READ sampleRate WRITE setSampleRate)
	Q_PROPERTY(int buffer_size READ bufferSize WRITE setBufferSize)
	Q_PROPERTY(int delay READ delay WRITE setDelay)
	Q_PROPERTY(double time_base READ timeBase WRITE setTimeBase STORED false)
	Q_PROPERTY(double time_position READ timePosition WRITE setTimePosition STORED false)
	Q_PROPERTY(QList<bool> enabled READ enabled WRITE setEnabled)
	Q_PROPERTY(QList<double> volts_per_div READ voltsPerDiv WRITE setVoltsPerDiv)
	Q_PROPERTY(QList<double> vertical_position READ verticalPosition WRITE setVerticalPosition)
	Q_PROPERTY(QList<double> probe READ probe WRITE setProbe)
	Q_PROPERTY(QList<bool> ac_coupled READ acCoupled WRITE setAcCoupled)
	Q_PROPERTY(bool cursors_x READ cursorsX WRITE setCursorsX)
	Q_PROPERTY(bool cursors_y READ cursorsY WRITE setCursorsY)
	Q_PROPERTY(QList<double> cursor_x READ cursorX WRITE setCursorX)
	Q_PROPERTY(QList<double> cursor_y READ cursorY WRITE setCursorY)
	Q_PROPERTY(bool trigger_enabled READ triggerEnabled WRITE setTriggerEnabled)
	Q_PROPERTY(bool trigger_auto READ triggerAuto WRITE setTriggerAuto)
	Q_PROPERTY(bool trigger_analog READ triggerAnalog WRITE setTriggerAnalog)
	Q_PROPERTY(int trigger_channel READ triggerChannel WRITE setTriggerChannel)
	Q_PROPERTY(int trigger_digital READ triggerDigital WRITE setTriggerDigital)
	Q_PROPERTY(int trigger_logic READ triggerLogic WRITE setTriggerLogic)
	Q_PROPERTY(int trigger_condition READ triggerCondition WRITE setTriggerCondition)
	Q_PROPERTY(int trigger_external_condition READ triggerExternalCondition WRITE setTriggerExternalCondition)
	Q_PROPERTY(double trigger_level READ triggerLevel WRITE setTriggerLevel)
	Q_PROPERTY(double trigger_hysteresis READ triggerHysteresis WRITE setTriggerHysteresis)
	Q_PROPERTY(int trigger_out READ triggerOut WRITE setTriggerOut)
	Q_PROPERTY(QStringList math READ math WRITE setMath)
	Q_PROPERTY(QStringList measurements READ measurements WRITE setMeasurements)
	Q_PROPERTY(QStringList measurement_stats READ measurementStats WRITE setMeasurementStats)
	Q_PROPERTY(bool gated READ gated WRITE setGated)
	Q_PROPERTY(bool xy_enabled READ xyEnabled WRITE setXYEnabled)
	Q_PROPERTY(QList<int> xy_sources READ xySources WRITE setXYSources)
	Q_PROPERTY(bool histogram_enabled READ histogramEnabled WRITE setHistogramEnabled)
	Q_PROPERTY(bool fft_enabled READ fftEnabled WRITE setFftEnabled)

public:
	explicit Adalm2000Osc_API(M2kOscController *tool);

	bool running() const;
	void setRunning(bool on);
	double sampleRate() const;
	void setSampleRate(double hz);
	int bufferSize() const;
	void setBufferSize(int n);
	int delay() const;
	void setDelay(int samples);
	double timeBase() const;
	void setTimeBase(double secPerDiv);
	double timePosition() const;
	void setTimePosition(double s);
	int timeBaseSamples() const;
	void setTimeBaseSamples(int n);
	QList<bool> enabled() const;
	void setEnabled(const QList<bool> &on);
	QList<double> voltsPerDiv() const;
	void setVoltsPerDiv(const QList<double> &v);
	QList<double> verticalPosition() const;
	void setVerticalPosition(const QList<double> &v);
	QList<double> probe() const;
	void setProbe(const QList<double> &v);
	QList<bool> acCoupled() const;
	void setAcCoupled(const QList<bool> &on);
	bool cursorsX() const;
	void setCursorsX(bool on);
	bool cursorsY() const;
	void setCursorsY(bool on);
	// The two cursors' positions: seconds for X, volts on the selected channel for Y.
	QList<double> cursorX() const;
	void setCursorX(const QList<double> &pos);
	QList<double> cursorY() const;
	void setCursorY(const QList<double> &pos);
	bool triggerEnabled() const;
	void setTriggerEnabled(bool on);
	bool triggerAuto() const;
	void setTriggerAuto(bool on);
	bool triggerAnalog() const;
	void setTriggerAnalog(bool on);
	int triggerChannel() const;
	void setTriggerChannel(int ch);
	int triggerDigital() const;
	void setTriggerDigital(int source);
	int triggerLogic() const;
	void setTriggerLogic(int logic);
	int triggerCondition() const;
	void setTriggerCondition(int c);
	int triggerExternalCondition() const;
	void setTriggerExternalCondition(int c);
	double triggerLevel() const;
	void setTriggerLevel(double v);
	double triggerHysteresis() const;
	void setTriggerHysteresis(double v);
	int triggerOut() const;
	void setTriggerOut(int select);
	QStringList math() const;
	void setMath(const QStringList &formulas);
	QStringList measurements() const;
	void setMeasurements(const QStringList &list);
	QStringList measurementStats() const;
	void setMeasurementStats(const QStringList &list);
	bool gated() const;
	void setGated(bool on);
	bool xyEnabled() const;
	void setXYEnabled(bool on);
	bool histogramEnabled() const;
	void setHistogramEnabled(bool on);
	QList<int> xySources() const;
	void setXYSources(const QList<int> &traces);
	bool fftEnabled() const;
	void setFftEnabled(bool on);

	Q_INVOKABLE void single();
	Q_INVOKABLE void autoset();
	Q_INVOKABLE bool isAutosetting() const;
	Q_INVOKABLE QList<double> samples(int ch) const;
	Q_INVOKABLE QList<double> time() const;
	// Counts per level over the range shown for the channel, with the histogram on.
	Q_INVOKABLE QList<double> histogram(int ch) const;
	// dBV per bin, and the bins' frequencies.
	Q_INVOKABLE QList<double> fft(int ch) const;
	Q_INVOKABLE QList<double> fftFrequencies(int ch) const;
	Q_INVOKABLE bool exportData(const QString &path) const;
	Q_INVOKABLE double measurement(int trace, const QString &id) const;
	Q_INVOKABLE QVariantMap statistics(int trace, const QString &id) const;
	Q_INVOKABLE int traceCount() const;
	Q_INVOKABLE QString traceName(int trace) const;
	Q_INVOKABLE int addMath(const QString &formula);
	Q_INVOKABLE int snapshot(int trace);
	// CSV, or tab-delimited for .txt. Returns the references added.
	Q_INVOKABLE int importReferences(const QString &path);
	Q_INVOKABLE void removeTrace(int trace);

private:
	M2kOscController *m_tool;
};
} // namespace scopy::adalm2000
#endif // ADALM2000OSC_API_H
