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

#ifndef ADXL355SOURCE_H
#define ADXL355SOURCE_H

#include <core/acq_engine/SourceBlock.h>

#include <atomic>
#include <QMutex>
#include <QStringList>

struct iio_context;
struct iio_device;
struct iio_buffer;
struct iio_channel;

namespace scopy {
namespace adc {

// SourceBlock over an ADXL355 accelerometer, as exposed by the no-OS/tinyiiod
// firmware behind a serial IIO context:
//
//   iio:device0: adxl355 (buffer capable)
//     accel_x  (input, index 0, le:s20/32>>0)   7 attrs
//     accel_y  (input, index 1, le:s20/32>>0)   7 attrs
//     accel_z  (input, index 2, le:s20/32>>0)   7 attrs
//     temp     (input, no index)                offset / raw / scale
//
// Two different acquisition paths, because the device has two different kinds of
// channel and pretending otherwise would misreport one of them:
//
//   accel_x/y/z are scan elements — they have a buffer index and a format, so
//   they are read through an iio_buffer like any other buffered channel. Raw
//   samples are 20-bit signed in a 32-bit container; iio_channel_convert() does
//   the demux/shift/sign-extend rather than hand-rolled bit twiddling, then the
//   channel's own `scale` attribute converts to m/s².
//
//   temp has no index, so it is *not* in the buffer at all: it is an attribute
//   triplet (raw, offset, scale). Enabling it makes acquire() read `raw` once
//   per cycle and publish (raw + offset) * scale / 1000 as °C, held flat across
//   the chunk so it lands on the same sample-index X axis as the accel traces.
//   One value per cycle is genuinely all the device offers — the flat chunk is a
//   presentation choice, not resampling.
//
// Keys: DataKey::raw(id(), "accel_x") … "accel_z", DataKey::raw(id(), "temp").
//
// Rate and filter corner come from the device's own *_available lists rather
// than a hardcoded table, so a firmware offering a different set stays correct.
// The two lists do not behave the same way, though: the rate list is fixed,
// while the corner list is re-derived by the device from the current rate — see
// highPassOptions(). Both attributes are written to all three accel channels:
// they are per-channel in the ABI but one shared ODR/HPF in the part, and
// leaving them to disagree would put channels on timelines that only look
// independent.
//
// Threading: the settings widget writes wanted values from the GUI thread while
// the worker may be blocked in a refill. An attribute write racing a refill on
// the serial backend corrupts the transport, so the widget only records the
// request under m_pendingMutex and acquire() applies it between refills. When
// the engine is stopped there is no worker and the write happens immediately.
//
// SERIAL BANDWIDTH. At 115200 8N1 the link carries ~11 kB/s, and one cycle of
// three enabled channels is bufferSize * 3 * 4 bytes — a 1024-sample buffer is
// ~12 kB, so better than a second per cycle regardless of the configured ODR.
// The instrument's buffer size is the knob that matters here, not the FPS cap.
class Adxl355Source : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	// The three buffered channels, in scan-element order.
	static const QStringList kAccelChannels;
	// The attribute-read channel. Not a scan element — see the class comment.
	static constexpr const char *kTempChannel = "temp";

	// Reads scale, ODR and the available lists off the device, so streamInfo()
	// carries a real sample rate before the first run — a view registers its
	// depth claim before cycle one, and a source that only learned its rate in
	// onStart() would hand out an unlabelled timeline until then. `ctx` must
	// outlive this block.
	explicit Adxl355Source(iio_context *ctx, const QString &id = "adxl355",
			       const QString &devName = "adxl355", QObject *parent = nullptr);
	~Adxl355Source() override;

	// Finds the device, enables the buffered channels, creates the buffer. temp
	// being enabled does not create one: it is not a scan element, so a run with
	// only temp enabled reads attributes and never touches a buffer.
	void onStart() override;

	// Cancels and destroys the buffer, which unblocks a blocked refill. Reached
	// from the destructor too, so it stays idempotent.
	void onStop() override;

	void acquire(scopy::acq::DataStore *store) override;

	// Per-channel labels and units — m/s² for the accel triple, °C for temp —
	// plus the device's ODR as the timeline. The base class would answer
	// unitless curves on an unknown timeline for all four.
	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;

	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

	// The device's current ODR in Hz, as last read back. 0 when unknown.
	double sampleRate() const;

	// Requests an ODR, which must be one of samplingFrequencyOptions(). Applied
	// on the spot when stopped, at the next cycle boundary when running.
	void setSampleRate(double sr);

	// Requested high-pass corner in Hz; 0 disables the filter.
	double highPassFrequency() const;
	void   setHighPassFrequency(double hz);

	// The ODR list the device accepts, descending. Fixed for the lifetime of the
	// firmware, so it is read once. Empty when the attribute is absent — the
	// widget then offers no combo rather than a made-up list.
	QList<double> samplingFrequencyOptions() const { return m_odrOptions; }

	// The high-pass corners the device accepts *at the current ODR*, descending.
	//
	// THIS LIST MOVES WITH THE ODR. On the ADXL355 the corner is a fixed ratio of
	// the output rate, so the device re-derives the list on every rate change:
	// 9.88 Hz at 4000 Hz ODR becomes 1.235 Hz at 500. Writing a corner from the
	// wrong list is not refused — the attribute write returns 0 and the device
	// reads back 0.000000, i.e. the filter silently ends up disabled. So this is
	// re-read after every ODR change rather than cached from construction, and a
	// widget rebuilds its choices from deviceConfigChanged().
	QList<double> highPassOptions() const;

Q_SIGNALS:
	// An ODR or filter write landed on the device, so a settings widget can
	// show what the part actually took rather than what was asked for. Emitted
	// from whichever thread applied it — connect queued.
	void deviceConfigChanged();

private:
	iio_channel *findChannel(const QString &channelId) const;

	// Attribute reads/writes, all no-ops without a device. Kept private: a
	// caller outside would be writing the device behind the pending-write
	// mechanism that exists to keep those off the worker's transport.
	bool   readChannelDouble(const QString &channelId, const char *attr, double &out) const;
	bool   writeAccelDouble(const char *attr, double value);
	QList<double> readAvailable(const QString &channelId, const char *attr) const;

	// Re-reads scale and ODR into the cached values. Called from the
	// constructor and from onStart(), so a device reconfigured by another tool
	// between runs is picked up.
	void refreshDeviceInfo();

	// Applies whatever the widget asked for since the last cycle. Worker
	// thread, between refills.
	void applyPendingConfig();

	// Publishes one accel channel from the current buffer contents.
	void writeAccelChannel(scopy::acq::DataStore *store, const QString &channelId, iio_channel *ch,
			       ptrdiff_t step);

	// Reads the temperature attribute triplet and publishes a flat chunk.
	void writeTempChannel(scopy::acq::DataStore *store, int count);

	iio_context *m_ctx{nullptr};
	QString      m_devName;
	iio_device  *m_dev{nullptr};

	// Read by acquire() and cleared by onStop() from the GUI thread while the
	// worker may be inside a refill — the same reason PlutoIIOSource holds its
	// buffer in an atomic.
	std::atomic<iio_buffer *> m_buf{nullptr};

	// m/s² per LSB, and °C conversion terms. Written by refreshDeviceInfo() on
	// the GUI thread, read per cycle by the worker.
	std::atomic<double> m_accelScale{0.0};
	std::atomic<double> m_tempScale{0.0};
	std::atomic<double> m_tempOffset{0.0};

	// The ODR the device last reported, which is what streamInfo() publishes as
	// the timeline. Not the requested value: a device that clamped the request
	// would otherwise have every downstream measurement scaled wrongly.
	std::atomic<double> m_sampleRate{0.0};
	std::atomic<double> m_hpfFrequency{0.0};

	// Fixed for the firmware, so read once and never guarded.
	QList<double> m_odrOptions;

	// Re-read whenever the ODR moves — see highPassOptions(). Written by
	// refreshDeviceInfo() from whichever thread applied the rate change and read
	// by the widget on the GUI thread, so unlike m_odrOptions it needs the lock.
	mutable QMutex m_hpfOptionsMutex;
	QList<double>  m_hpfOptions;

	// Requests from the GUI thread, drained by applyPendingConfig(). NaN means
	// "nothing requested" — 0 is a legal value for the filter corner.
	mutable QMutex m_pendingMutex;
	double         m_pendingSampleRate{qQNaN()};
	double         m_pendingHpf{qQNaN()};
};

} // namespace adc
} // namespace scopy

#endif // ADXL355SOURCE_H
