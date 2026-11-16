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
 *
 */

#ifndef MULTIACQ_API_H
#define MULTIACQ_API_H

#include <QPointer>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <pluginbase/apiobject.h>

namespace scopy::multiacq {

class IioSource;
class MultiAcqTool;
class TimeCurve;
class TimePlot;

/// MultiDevice Acq: channels of several devices on time plots sharing one wall-clock axis.
/// Devices are added by URI and addressed by it; their channels by id "<iio device>:<channel>"
/// (e.g. "adxl345:accel_x"), as getChannels() lists them; plots by uuid, as getPlots() lists them.
/// Channel and device settings take effect on the next run; a running tool restarts by itself.
/// Whole setups are saved and loaded with scopy.saveSetup()/scopy.loadSetup(). A load connects the
/// saved devices that are not added yet, where the same board still answers at the saved URI;
/// getLoadReport() tells what the last load skipped.
class MultiAcq_API : public ApiObject
{
	Q_OBJECT
public:
	explicit MultiAcq_API(MultiAcqTool *tool, QObject *parent = nullptr);

	// --- run control
	/// Start acquiring from every enabled device; runs until stop().
	Q_INVOKABLE void run();
	/// Stop acquiring.
	Q_INVOKABLE void stop();
	/// One acquisition cycle.
	Q_INVOKABLE void single();
	/// True while acquiring.
	Q_INVOKABLE bool isRunning();
	/// Clear every plot and restart the relative time.
	Q_INVOKABLE void clear();

	// --- devices
	/// Added devices: [{uri, name, model, enabled, online, readIntervalMs, iioDevices[]}].
	Q_INVOKABLE QVariantList getDevices();
	/// Connect a device by URI (a bare address is taken as "ip:"). "" on success, else the reason.
	Q_INVOKABLE QString addDevice(const QString &uri);
	/// Remove a device and its curves.
	Q_INVOKABLE bool removeDevice(const QString &uri);
	/// Enable/disable a device; a disabled one is not read.
	Q_INVOKABLE bool setDeviceEnabled(const QString &uri, bool en);
	/// How often attribute-read ("polled") channels are read, in ms (>= 1).
	Q_INVOKABLE bool setReadInterval(const QString &uri, int ms);
	/// A device's IIO devices: [{name, bufferable, hasRate, rateOverride}].
	Q_INVOKABLE QVariantList getIioDevices(const QString &uri);
	/// For a bufferable IIO device that publishes no sampling_frequency: the rate its buffer
	/// samples are spaced at, Hz. 0 spreads them over the time between refills.
	Q_INVOKABLE bool setRateOverride(const QString &uri, const QString &iioDevice, double hz);

	// --- device channels
	/// A device's channels: [{id, enabled, mode, attribute, scaled, attributes[], bufferable,
	/// scalable, description}]. mode is "polled" (one attribute read per interval) or "buffered".
	Q_INVOKABLE QVariantList getChannels(const QString &uri);
	/// Enable/disable reading a channel. All start disabled.
	Q_INVOKABLE bool setChannelEnabled(const QString &uri, const QString &channel, bool en);
	/// Read a channel "polled" (from an attribute) or "buffered" (from the device buffer).
	Q_INVOKABLE bool setChannelMode(const QString &uri, const QString &channel, const QString &mode);
	/// The attribute a polled channel reads, one of its attributes[].
	Q_INVOKABLE bool setChannelAttribute(const QString &uri, const QString &channel, const QString &attribute);
	/// Plot (raw + offset) * scale instead of the plain reading, where the channel is scalable.
	Q_INVOKABLE bool setChannelScaled(const QString &uri, const QString &channel, bool scaled);

	// --- plots
	/// All plots: [{uuid, name, legend, timeLabels ("relative"|"timeofday"), timeFormat, span,
	/// history, live, autoscale, yMin, yMax, curves[{uri, channel, name, color, visible}]}].
	/// span and history are in seconds.
	Q_INVOKABLE QVariantList getPlots();
	/// Add a plot; empty name = "Plot N". Returns its uuid.
	Q_INVOKABLE int addPlot(const QString &name);
	/// Remove a plot and its curves.
	Q_INVOKABLE bool removePlot(int uuid);
	/// Rename a plot.
	Q_INVOKABLE bool setPlotName(int uuid, const QString &name);
	/// Show/hide a plot's legend.
	Q_INVOKABLE bool setPlotLegend(int uuid, bool on);
	/// X labels: "relative" (time since start) or "timeofday".
	Q_INVOKABLE bool setTimeLabels(int uuid, const QString &labels);
	/// X label format, e.g. "hh:mm:ss", "mm:ss.zzz", "dd.MM hh:mm".
	Q_INVOKABLE bool setTimeFormat(int uuid, const QString &format);
	/// Seconds shown across the plot; history grows to at least this.
	Q_INVOKABLE bool setSpan(int uuid, double seconds);
	/// Seconds of data kept for scrolling back, at least the span.
	Q_INVOKABLE bool setHistory(int uuid, double seconds);
	/// Follow the current time; off, the X axis stays put.
	Q_INVOKABLE bool setLive(int uuid, bool on);
	/// Turn Y autoscale on/off.
	Q_INVOKABLE bool setAutoscale(int uuid, bool on);
	/// Fix the Y axis to [min, max]; turns autoscale off.
	Q_INVOKABLE bool setYRange(int uuid, double min, double max);

	// --- curves: a device channel on a plot
	/// Put a channel on a plot, enabling the channel on its device if it is not.
	Q_INVOKABLE bool addCurve(int uuid, const QString &uri, const QString &channel);
	/// Take a channel off a plot.
	Q_INVOKABLE bool removeCurve(int uuid, const QString &uri, const QString &channel);
	/// Show/hide a curve.
	Q_INVOKABLE bool setCurveVisible(int uuid, const QString &uri, const QString &channel, bool visible);

	// --- setups
	/// What the last scopy.loadSetup() / File > Load could not restore; empty when nothing was skipped.
	Q_INVOKABLE QStringList getLoadReport();

private:
	IioSource *source(const QString &uri) const;
	TimePlot *plot(int uuid) const;
	TimeCurve *curve(int uuid, const QString &uri, const QString &channel) const;

	QPointer<MultiAcqTool> m_tool;
};

} // namespace scopy::multiacq

#endif // MULTIACQ_API_H
