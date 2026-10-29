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

#ifndef ACQUISITION_API_H
#define ACQUISITION_API_H

#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <pluginbase/apiobject.h>

namespace scopy::adc {
class AcqChannel;
class AcqInstrumentController;
class AcqPlot;
} // namespace scopy::adc

namespace scopy::acq {
class Block;
} // namespace scopy::acq

namespace scopy::decoder {
class DecoderEditor;
} // namespace scopy::decoder

namespace scopy::acquisition {

class AcquisitionPlugin;

/// Generic IIO acquisition instrument: sources stream device channels into a DataStore,
/// processors (FFT, trigger, decoders) derive new streams from it, and plots draw them.
/// Streams are named by key strings "source_channel_stage" (e.g. "dc_cf-ad9361-lpc:voltage0_raw");
/// list them with getKeys(). Plots are addressed by uuid, plot channels by uid, as
/// getPlots()/getPlotChannels() list them. Several connected devices share this one object:
/// setDevice() picks which one the calls act on (default: the last connected).
class Acquisition_API : public ApiObject
{
	Q_OBJECT
public:
	// Called by each plugin on connect/disconnect. The object is registered with the
	// first device and unregistered with the last.
	static void attach(AcquisitionPlugin *plugin);
	static void detach(AcquisitionPlugin *plugin);

	// --- devices
	/// URIs of all connected acquisition devices.
	Q_INVOKABLE QStringList getDevices();
	/// URI of the device the other calls act on.
	Q_INVOKABLE QString getDevice();
	/// Select the device (by URI from getDevices()) the other calls act on. False if unknown.
	Q_INVOKABLE bool setDevice(const QString &uri);

	// --- run control
	/// Start continuous acquisition; runs until stop().
	Q_INVOKABLE void run();
	/// Stop acquisition.
	Q_INVOKABLE void stop();
	/// One acquisition cycle (or, in triggered mode, until one trigger fire); returns immediately.
	Q_INVOKABLE void single();
	/// single(), then block until it ends. False on timeout (the run is then stopped) or when a
	/// block failed and the engine stopped itself; true only for a clean capture.
	Q_INVOKABLE bool singleAndWait(int timeoutMs);
	/// True while acquiring.
	Q_INVOKABLE bool isRunning();
	/// Engine mode: "continuous" or "triggered". Triggered waits for a fire only with the trigger
	/// block enabled; without it, single() is one plain cycle. Refused while running.
	Q_INVOKABLE bool setMode(const QString &mode);
	/// Current engine mode: "continuous" or "triggered".
	Q_INVOKABLE QString getMode();
	/// Samples read per channel per cycle (power of two, 16..1048576); also the FFT size. False if
	/// out of range or while running.
	Q_INVOKABLE bool setBufferSize(int n);
	/// Samples read per channel per cycle.
	Q_INVOKABLE int getBufferSize();

	// --- pipeline
	/// Names of the source blocks (one per IIO device), usable as `block`/`source` args.
	Q_INVOKABLE QStringList getSources();
	/// Names of the processor blocks (e.g. "genalyzer-fft", "trigger"), usable as `block` args.
	Q_INVOKABLE QStringList getProcessors();
	/// A block's state: {enabled, source(bool), in[] (watched input keys), out[] (keys it writes)};
	/// sources also list channels[] (the enabled channel ids).
	Q_INVOKABLE QVariantMap getBlockInfo(const QString &block);
	/// Enable/disable a source or processor block. A disabled block produces nothing.
	Q_INVOKABLE bool setBlockEnabled(const QString &block, bool en);
	/// Set a processor's input keys (comma-separated). The FFT ("genalyzer-fft", output
	/// "genalyzer_spectrum_fft") produces nothing until set: 1 key = real FFT, 2 keys = complex
	/// I/Q FFT, empty = none. Refused while the engine runs with the block enabled.
	Q_INVOKABLE bool setBlockInputs(const QString &block, const QString &keys);
	/// Channel ids of a source block (e.g. "voltage0").
	Q_INVOKABLE QStringList getSourceChannels(const QString &source);
	/// Enable/disable one channel of a source block. All start disabled; only enabled ones stream.
	Q_INVOKABLE bool setSourceChannelEnabled(const QString &source, const QString &channel, bool en);

	// --- decoders (sigrok). Each runs as a processor block named by its uid, so getBlockInfo()
	// and setBlockEnabled() apply; it writes annotations to "decoder_<uid>/<stage>_annotations",
	// read with getAnnotations(). Inputs must be bit streams (UInt8 keys).
	/// Ids of the protocol decoders available (e.g. "uart", "i2c", "spi").
	Q_INVOKABLE QStringList getDecoderIds();
	/// A decoder's description: {id, name, description, inputs[], outputs[],
	/// channels[{id, name, required}], options[{id, name, default, choices[]}]}.
	Q_INVOKABLE QVariantMap getDecoderInfo(const QString &decoderId);
	/// Add a decoder, as the Decoders panel's "+ Add" does. Returns its uid, "" on failure.
	Q_INVOKABLE QString addDecoder(const QString &decoderId);
	/// Remove a decoder and its annotation channels.
	Q_INVOKABLE void removeDecoder(const QString &uid);
	/// Active decoders: [{uid, stages[] (decoder ids), inputs[] (bound keys, bit order), outputs[]}].
	Q_INVOKABLE QVariantList getDecoders();
	/// Bind a decoder channel role (an id from getDecoderInfo(), e.g. "rx") to a raw key; empty
	/// unassigns. The key must already be in the store. Live: a running engine picks it up next cycle.
	Q_INVOKABLE bool setDecoderChannel(const QString &uid, const QString &role, const QString &key);
	/// Set a decoder option (an id from getDecoderInfo(), e.g. "baudrate"). Live, like setDecoderChannel.
	Q_INVOKABLE bool setDecoderOption(const QString &uid, const QString &option, const QString &value);
	/// The sample rate the decoder assumes for its input, in Hz. Live, like setDecoderChannel.
	Q_INVOKABLE bool setDecoderSampleRate(const QString &uid, double hz);

	// --- datastore: discovery
	/// Keys written at least once since connect; they stay listed after clearStore(), empty.
	Q_INVOKABLE QStringList getKeys();
	/// Keys every block declares, enabled or not, whether or not they hold data yet.
	Q_INVOKABLE QStringList getDeclaredKeys();
	/// True if the key has been written (see getKeys()); getLatest() tells whether it holds data.
	Q_INVOKABLE bool hasKey(const QString &key);
	/// Key metadata: {key, source, channel, stage, present, type, depth, capacity, chunkSize, label, unit,
	/// sampleRate, xKey, kind}.
	Q_INVOKABLE QVariantMap getKeyInfo(const QString &key);
	/// Total store writes so far; grows each cycle, so a change means new data arrived.
	Q_INVOKABLE double getWriteCount();

	// --- datastore: reading. Annotation keys read through getAnnotations() only.
	/// Newest chunk (one cycle, bufferSize samples) of a numeric key as a number array.
	Q_INVOKABLE QVariantList getLatest(const QString &key);
	/// Newest n samples of a numeric key, spanning chunks if retained (see claimDepth).
	Q_INVOKABLE QVariantList getWindow(const QString &key, int n);
	/// One retained chunk of a key; index 0 = newest, 1 = the one before, ...
	Q_INVOKABLE QVariantList getChunk(const QString &key, int index);
	/// Statistics of the newest n samples: {count, min, max, mean, rms, pp}.
	Q_INVOKABLE QVariantMap getStats(const QString &key, int n);
	/// Newest decoder annotations of a key, at most max (<=0: all):
	/// [{start, end, decoder, class, text, severity, value?}].
	Q_INVOKABLE QVariantList getAnnotations(const QString &key, int max);
	/// Write the newest n samples of each key (comma-separated) to a CSV file, one column per key.
	Q_INVOKABLE bool saveCsv(const QString &keys, int n, const QString &path);

	// --- datastore: history and lifecycle
	/// Keep at least `chunks` past chunks of a key, for getChunk()/getWindow() across cycles.
	/// History builds within one run(); every run, single() included, starts empty.
	Q_INVOKABLE void claimDepth(const QString &key, int chunks);
	/// Drop the history kept by claimDepth().
	Q_INVOKABLE void releaseDepth(const QString &key);
	/// Discard all stored samples; keys and depth claims stay.
	Q_INVOKABLE void clearStore();

	// --- plots
	/// All plots: [{uuid, name, kind ("basic"|"waterfall"), channels[] (plot channel uids)}].
	Q_INVOKABLE QVariantList getPlots();
	/// Add a plot, kind "basic" (curves) or "waterfall". Returns its uuid, -1 on failure.
	Q_INVOKABLE int addPlot(const QString &name, const QString &kind);
	/// Remove a plot and its channels.
	Q_INVOKABLE void removePlot(int uuid);
	/// Rename a plot.
	Q_INVOKABLE void setPlotName(int uuid, const QString &name);
	/// Show/hide a plot's legend.
	Q_INVOKABLE void setPlotLegend(int uuid, bool on);
	/// Show/hide a plot's axis labels.
	Q_INVOKABLE void setPlotLabels(int uuid, bool on);
	/// Show/hide a plot's cursors.
	Q_INVOKABLE void setPlotCursors(int uuid, bool on);
	/// Show/hide a strip of labelled blocks along the bottom of a plot, one per acquired buffer,
	/// and a dashed vertical line at each boundary between buffers.
	Q_INVOKABLE void setPlotBufferStarts(int uuid, bool on);
	/// Samples shown across time plots (the visible window, may exceed bufferSize).
	Q_INVOKABLE void setPlotSize(int n);
	/// Samples shown across time plots.
	Q_INVOKABLE int getPlotSize();

	// --- plot channels
	/// All plot channels: [{uid, plot, kind, yKey, xKey, name, color, enabled, keyPresent, sampleRate}].
	Q_INVOKABLE QVariantList getPlotChannels();
	/// Draw yKey on a plot. kind: "curve" | "digital" | "annotations" | "waterfall". Empty xKey = the
	/// key's default abscissa (sample index, or frequency for the FFT output "genalyzer_spectrum_fft"). Returns
	/// uid, -1 on failure.
	Q_INVOKABLE int addPlotChannel(int plotUuid, const QString &kind, const QString &yKey, const QString &xKey);
	/// Remove a plot channel.
	Q_INVOKABLE void removePlotChannel(int uid);
	/// Show/hide a plot channel.
	Q_INVOKABLE void setPlotChannelEnabled(int uid, bool en);
	/// Rename a plot channel.
	Q_INVOKABLE void setPlotChannelName(int uid, const QString &name);
	/// Set a plot channel's color: "#rrggbb" or an SVG color name.
	Q_INVOKABLE void setPlotChannelColor(int uid, const QString &color);

	// --- plot channel axes, axis = "x" | "y"
	/// Fix a plot channel's axis to [min, max]; turns autoscale off.
	Q_INVOKABLE void setAxisRange(int uid, const QString &axis, double min, double max);
	/// Turn autoscale of a plot channel's axis on/off.
	Q_INVOKABLE void setAxisAutoscale(int uid, const QString &axis, bool on);
	/// A plot channel's axis: {min, max, autoscale, unit, source}.
	Q_INVOKABLE QVariantMap getAxis(int uid, const QString &axis);

private:
	Acquisition_API() = default;

	adc::AcqInstrumentController *ctrl() const;
	scopy::acq::Block *block(const QString &name) const;
	adc::AcqPlot *plot(int uuid) const;
	adc::AcqChannel *channel(int uid) const;
	decoder::DecoderEditor *decoderEditor(const QString &uid) const;

	QList<AcquisitionPlugin *> m_plugins;
	AcquisitionPlugin *m_current{nullptr};
};

} // namespace scopy::acquisition

#endif // ACQUISITION_API_H
