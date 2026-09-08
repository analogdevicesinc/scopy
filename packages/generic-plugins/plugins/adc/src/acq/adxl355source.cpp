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

#include "adxl355source.h"

#include <core/acq_engine/DataKey.h>
#include <core/acq_engine/DataStore.h>

#include <gui/style.h>
#include <gui/widgets/menusectionwidget.h>

#include <iio.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMutexLocker>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QSpacerItem>
#include <QStringList>
#include <QVBoxLayout>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <stdexcept>

using namespace scopy;
using namespace scopy::adc;

const QStringList Adxl355Source::kAccelChannels = {QStringLiteral("accel_x"), QStringLiteral("accel_y"),
						  QStringLiteral("accel_z")};

namespace {

// Longest attribute value we read back. The available lists are the big ones: 11
// rates printed at six decimals is under 150 bytes, so this has plenty of margin
// and a truncated read is reported rather than silently parsed short.
constexpr int kAttrBufLen = 512;

// The device reports temperature scale in m°C per LSB (-110.497238), so the
// triplet converts as (raw + offset) * scale / 1000.
constexpr double kMilliDegPerDeg = 1000.0;

// A labelled row, matching GenalyzerTransformSettings' and SnapshotSourceWidget's
// layout so every block panel in the rail lines up.
QWidget *labelledRow(const QString &text, QWidget *field, QWidget *parent)
{
	auto *w = new QWidget(parent);
	auto *lay = new QHBoxLayout(w);
	lay->setContentsMargins(0, 0, 0, 0);

	auto *label = new QLabel(text, w);
	Style::setStyle(label, style::properties::label::subtle);

	lay->addWidget(label);
	lay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	lay->addWidget(field);
	return w;
}

} // namespace

Adxl355Source::Adxl355Source(iio_context *ctx, const QString &id, const QString &devName, QObject *parent)
	: SourceBlock(id, parent)
	, m_ctx(ctx)
	, m_devName(devName)
{
	// The channel set is fixed by the part, so it is registered here rather than
	// discovered in onStart(): the rail shows four rows before the first run, and
	// a reader can pick which ones to acquire before starting.
	//
	// All three accel channels on, temp off. The accel triple is what the part is
	// for; temp costs a separate attribute round trip per cycle on a 115200 link,
	// so it is opt-in.
	for(const QString &chId : kAccelChannels) {
		enableChannel(chId, true);
	}
	enableChannel(QString::fromLatin1(kTempChannel), false);

	// Found up front, not in onStart(): every read below needs it, and a device
	// that is absent at construction time is worth knowing about before a run is
	// attempted. onStart() looks it up again and throws there — this is a cache,
	// not the check.
	if(m_ctx) {
		m_dev = iio_context_find_device(m_ctx, m_devName.toLocal8Bit().constData());
	}

	// The ODR list is a property of the firmware rather than of the current
	// configuration, so it is read once here; re-reading it per run would spend
	// serial bandwidth to get the same answer. The high-pass list is not like
	// that — refreshDeviceInfo() re-reads it, because it moves with the ODR.
	m_odrOptions = readAvailable(kAccelChannels.first(), "sampling_frequency_available");

	refreshDeviceInfo();
}

Adxl355Source::~Adxl355Source() { onStop(); }

// ---------------------------------------------------------------------------
// Device attribute access
// ---------------------------------------------------------------------------

iio_channel *Adxl355Source::findChannel(const QString &channelId) const
{
	if(!m_dev) {
		return nullptr;
	}
	// output=false: every channel on this part is an input.
	return iio_device_find_channel(m_dev, channelId.toLocal8Bit().constData(), false);
}

bool Adxl355Source::readChannelDouble(const QString &channelId, const char *attr, double &out) const
{
	iio_channel *ch = findChannel(channelId);
	if(!ch) {
		return false;
	}
	// attr_read_double rather than a string parse: it goes through the same
	// locale-independent conversion the rest of libiio uses, and the firmware
	// prints these with six decimals.
	return iio_channel_attr_read_double(ch, attr, &out) == 0;
}

bool Adxl355Source::writeAccelDouble(const char *attr, double value)
{
	bool anyOk = false;
	// All three, even though the part has one shared ODR and one shared filter:
	// the ABI exposes them per channel, and writing only accel_x would leave the
	// other two reporting a rate they are not running at — which streamInfo()
	// would then publish as three different timelines.
	for(const QString &chId : kAccelChannels) {
		iio_channel *ch = findChannel(chId);
		if(!ch) {
			continue;
		}
		if(iio_channel_attr_write_double(ch, attr, value) == 0) {
			anyOk = true;
		}
	}
	return anyOk;
}

QList<double> Adxl355Source::readAvailable(const QString &channelId, const char *attr) const
{
	QList<double> values;
	iio_channel  *ch = findChannel(channelId);
	if(!ch) {
		return values;
	}

	char          buf[kAttrBufLen] = {0};
	const ssize_t ret = iio_channel_attr_read(ch, attr, buf, sizeof(buf));
	if(ret < 0) {
		// Absent attribute, which is a legitimate firmware variation. The
		// caller offers no combo rather than inventing a list.
		return values;
	}

	// Space-separated, in whatever order the firmware prints — the ADXL355 lists
	// rates descending and filter corners with 0 first. Sorted descending here so
	// a widget can present them without knowing that.
	const QStringList parts = QString::fromLatin1(buf, static_cast<int>(ret))
					  .trimmed()
					  .split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
	for(const QString &p : parts) {
		bool         ok = false;
		const double v = p.toDouble(&ok);
		if(ok) {
			values.append(v);
		}
	}
	std::sort(values.begin(), values.end(), std::greater<double>());
	return values;
}

void Adxl355Source::refreshDeviceInfo()
{
	double v = 0.0;

	// One scale for the triple: the part has a single full-scale range, and the
	// three channels report the same 0.000038245. accel_x stands for all three.
	if(readChannelDouble(kAccelChannels.first(), "scale", v)) {
		m_accelScale.store(v, std::memory_order_relaxed);
	}
	if(readChannelDouble(kAccelChannels.first(), "sampling_frequency", v)) {
		m_sampleRate.store(v, std::memory_order_relaxed);
	}
	if(readChannelDouble(kAccelChannels.first(), "filter_high_pass_3db_frequency", v)) {
		m_hpfFrequency.store(v, std::memory_order_relaxed);
	}

	// Temperature's own scale and offset, which are unrelated to the accel scale
	// and have a sign of their own (scale is negative on this part).
	const QString tempId = QString::fromLatin1(kTempChannel);
	if(readChannelDouble(tempId, "scale", v)) {
		m_tempScale.store(v, std::memory_order_relaxed);
	}
	if(readChannelDouble(tempId, "offset", v)) {
		m_tempOffset.store(v, std::memory_order_relaxed);
	}

	// After the ODR read above, and not cached from construction: the corners the
	// device accepts are a ratio of the current rate, so this list is only valid
	// for the rate just read back. Read with the lock released — it is a serial
	// round trip, and a widget asking for the list meanwhile should wait on the
	// swap rather than on the transport.
	QList<double> hpf = readAvailable(kAccelChannels.first(), "filter_high_pass_3db_frequency_available");
	{
		QMutexLocker lk(&m_hpfOptionsMutex);
		m_hpfOptions = std::move(hpf);
	}
}

QList<double> Adxl355Source::highPassOptions() const
{
	QMutexLocker lk(&m_hpfOptionsMutex);
	return m_hpfOptions;
}

// ---------------------------------------------------------------------------
// Configuration, applied between cycles
// ---------------------------------------------------------------------------

double Adxl355Source::sampleRate() const { return m_sampleRate.load(std::memory_order_relaxed); }

double Adxl355Source::highPassFrequency() const { return m_hpfFrequency.load(std::memory_order_relaxed); }

void Adxl355Source::setSampleRate(double sr)
{
	if(sr <= 0.0) {
		return;
	}

	// Running: the worker owns the transport, so this is queued for the cycle
	// boundary. Stopped: no worker exists, so writing now gives the widget
	// immediate feedback instead of a value that only lands on the next run.
	if(m_buf.load(std::memory_order_relaxed)) {
		QMutexLocker lk(&m_pendingMutex);
		m_pendingSampleRate = sr;
		return;
	}

	if(writeAccelDouble("sampling_frequency", sr)) {
		// Re-reads the rate the device actually took rather than storing `sr` —
		// it may clamp to its own list, and publishing the request as the
		// timeline would scale every downstream measurement wrongly. Also
		// re-reads the high-pass list, which moves with the rate, and the filter
		// corner, which the device re-derives (or drops to 0) when the rate
		// changes under it.
		refreshDeviceInfo();
		Q_EMIT deviceConfigChanged();
	}
}

void Adxl355Source::setHighPassFrequency(double hz)
{
	if(hz < 0.0) {
		return;
	}

	if(m_buf.load(std::memory_order_relaxed)) {
		QMutexLocker lk(&m_pendingMutex);
		m_pendingHpf = hz;
		return;
	}

	if(writeAccelDouble("filter_high_pass_3db_frequency", hz)) {
		// Read back, and it matters more here than for the rate: a corner from
		// the wrong ODR's list is not refused — the write returns success and the
		// device reads back 0, so the filter is off while the caller believes it
		// asked for something. Reporting what it took is the only way that shows.
		double actual = 0.0;
		if(readChannelDouble(kAccelChannels.first(), "filter_high_pass_3db_frequency", actual)) {
			m_hpfFrequency.store(actual, std::memory_order_relaxed);
			if(hz != 0.0 && actual == 0.0) {
				report(scopy::acq::AcquisitionError::Severity::Warning,
				       QString("High-pass corner %1 Hz was rejected at %2 Hz ODR "
					       "(filter now disabled)")
					       .arg(hz)
					       .arg(m_sampleRate.load(std::memory_order_relaxed)));
			}
		}
		Q_EMIT deviceConfigChanged();
	}
}

void Adxl355Source::applyPendingConfig()
{
	double wantedSr = qQNaN();
	double wantedHpf = qQNaN();
	{
		QMutexLocker lk(&m_pendingMutex);
		wantedSr = m_pendingSampleRate;
		wantedHpf = m_pendingHpf;
		m_pendingSampleRate = qQNaN();
		m_pendingHpf = qQNaN();
	}

	bool changed = false;

	// The rate first, when both are pending. The corners are a ratio of the rate,
	// so applying the filter against the old rate and then moving the rate would
	// leave the device deriving a different corner than the one asked for.
	//
	// qIsNaN rather than a sentinel value: 0 is a legal filter corner ("disabled"
	// in the device's own list), so it cannot double as "nothing requested".
	if(!qIsNaN(wantedSr) && writeAccelDouble("sampling_frequency", wantedSr)) {
		// Re-reads rate, filter corner and the corner list, all of which the rate
		// change moves.
		refreshDeviceInfo();
		changed = true;
	}
	if(!qIsNaN(wantedHpf) && writeAccelDouble("filter_high_pass_3db_frequency", wantedHpf)) {
		double actual = 0.0;
		if(readChannelDouble(kAccelChannels.first(), "filter_high_pass_3db_frequency", actual)) {
			m_hpfFrequency.store(actual, std::memory_order_relaxed);
			// A corner from another rate's list is accepted and then ignored —
			// see setHighPassFrequency(). Worth a warning rather than a silently
			// unfiltered trace.
			if(wantedHpf != 0.0 && actual == 0.0) {
				report(scopy::acq::AcquisitionError::Severity::Warning,
				       QString("High-pass corner %1 Hz was rejected at %2 Hz ODR "
					       "(filter now disabled)")
					       .arg(wantedHpf)
					       .arg(m_sampleRate.load(std::memory_order_relaxed)));
			}
		}
		changed = true;
	}

	if(changed) {
		Q_EMIT deviceConfigChanged();
	}
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void Adxl355Source::onStart()
{
	SourceBlock::onStart();

	m_dev = iio_context_find_device(m_ctx, m_devName.toLocal8Bit().constData());
	if(!m_dev) {
		throw std::runtime_error(QString("Device '%1' not found").arg(m_devName).toStdString());
	}

	// Another tool may have moved the ODR since construction, and the timeline
	// every channel is published on comes from it.
	refreshDeviceInfo();

	// Only the scan elements. temp has no buffer index, so enabling it on the
	// device would either be rejected or silently add nothing to the stride.
	int scanElements = 0;
	for(const QString &chId : kAccelChannels) {
		if(!isChannelEnabled(chId)) {
			continue;
		}
		if(iio_channel *ch = findChannel(chId)) {
			iio_channel_enable(ch);
			++scanElements;
		}
	}

	if(scanElements == 0) {
		// A run with only temp enabled, or with nothing enabled at all. No
		// buffer is created — iio_device_create_buffer with no enabled scan
		// element fails — and acquire() falls through to the attribute path.
		// Not an error: temp-only is a legitimate way to run this device.
		return;
	}

	iio_buffer *buf = iio_device_create_buffer(m_dev, m_bufferSize, false);
	m_buf.store(buf);
	if(!buf) {
		throw std::runtime_error(
			QString("Failed to create IIO buffer (size %1)").arg(m_bufferSize).toStdString());
	}
}

void Adxl355Source::onStop()
{
	m_stopRequested = true;

	// Cancel before destroy, so a worker blocked in refill returns instead of
	// having the buffer freed underneath it.
	iio_buffer *buf = m_buf.exchange(nullptr);
	if(buf) {
		iio_buffer_cancel(buf);
		iio_buffer_destroy(buf);
	}

	if(m_dev) {
		// Every accel channel, not just the enabled ones: the GUI may have
		// flipped one off after onStart() enabled it on the device.
		for(const QString &chId : kAccelChannels) {
			if(iio_channel *ch = findChannel(chId)) {
				iio_channel_disable(ch);
			}
		}
		// m_dev is deliberately kept: the settings widget reads and writes
		// attributes while stopped, which is the whole point of the immediate
		// path in setSampleRate(). PlutoIIOSource clears it here because it has
		// no such path.
	}
}

// ---------------------------------------------------------------------------
// Acquisition
// ---------------------------------------------------------------------------

void Adxl355Source::writeAccelChannel(scopy::acq::DataStore *store, const QString &channelId, iio_channel *ch,
				      ptrdiff_t step)
{
	iio_buffer *buf = m_buf.load();
	if(!buf) {
		return;
	}

	const double scale = m_accelScale.load(std::memory_order_relaxed);
	// A missing scale attribute would silently zero the trace, so raw LSBs are
	// published instead — a plot in the wrong unit is diagnosable, an empty one
	// is not. streamInfo() reports the unit the same way.
	const double factor = (scale != 0.0) ? scale : 1.0;

	const char *p = static_cast<const char *>(iio_buffer_first(buf, ch));
	const char *end = static_cast<const char *>(iio_buffer_end(buf));

	QVector<float> samples;
	samples.reserve(static_cast<int>(m_bufferSize));

	for(; p < end; p += step) {
		// s20/32>>0 in a 32-bit container: the 20 significant bits are not
		// sign-extended to 32 by the transport, so a plain int32_t read gives a
		// positive number for every negative sample. iio_channel_convert() does
		// the byte order, the shift and the sign extension from the channel's own
		// declared format — hand-rolling it here would hardcode this firmware's
		// particular format string.
		int32_t raw = 0;
		iio_channel_convert(ch, &raw, p);
		samples.append(static_cast<float>(raw * factor));
	}

	store->write(scopy::acq::DataKey::raw(id(), channelId), std::move(samples));
}

void Adxl355Source::writeTempChannel(scopy::acq::DataStore *store, int count)
{
	double raw = 0.0;
	const QString tempId = QString::fromLatin1(kTempChannel);
	if(!readChannelDouble(tempId, "raw", raw)) {
		return;
	}

	const double scale = m_tempScale.load(std::memory_order_relaxed);
	const double offset = m_tempOffset.load(std::memory_order_relaxed);
	// (raw + offset) * scale, with scale in m°C/LSB — the standard IIO
	// conversion, and the reason offset is added before scaling rather than
	// after.
	const float degC = static_cast<float>((raw + offset) * scale / kMilliDegPerDeg);

	// Flat across the chunk. The device gives one value per read and nothing
	// pretends otherwise — this puts it on the same sample-index X axis as the
	// accel traces so the two can share a plot, at the cost of a step-shaped
	// trace between cycles. A single-sample chunk would instead read as a
	// stream at 1/bufferSize the accel rate and misalign every window.
	QVector<float> samples(count, degC);
	store->write(scopy::acq::DataKey::raw(id(), tempId), std::move(samples));
}

void Adxl355Source::acquire(scopy::acq::DataStore *store)
{
	if(m_stopRequested) {
		return;
	}

	// Before the refill, while the transport is idle. An attribute write racing a
	// refill corrupts the serial framing.
	applyPendingConfig();

	iio_buffer *buf = m_buf.load();

	// The accel path, when there is a buffer. Without one — temp-only run — this
	// whole block is skipped and only the attribute read below happens.
	if(buf) {
		const ssize_t ret = iio_buffer_refill(buf);
		if(ret < 0) {
			// A cancelled buffer surfaces here as an error; a requested stop
			// is not a fault.
			if(m_stopRequested) {
				return;
			}
			throw std::runtime_error(QString("iio_buffer_refill error %1").arg(ret).toStdString());
		}

		const ptrdiff_t step = iio_buffer_step(buf);

		for(const QString &chId : kAccelChannels) {
			if(!isChannelEnabled(chId)) {
				continue;
			}
			if(iio_channel *ch = findChannel(chId)) {
				writeAccelChannel(store, chId, ch, step);
			}
		}
	}

	if(m_stopRequested) {
		return;
	}

	if(isChannelEnabled(QString::fromLatin1(kTempChannel))) {
		writeTempChannel(store, static_cast<int>(m_bufferSize));
	}
}

// ---------------------------------------------------------------------------
// Settings
// ---------------------------------------------------------------------------

QWidget *Adxl355Source::createSettingsWidget(QWidget *parent)
{
	auto *container = new QWidget;
	auto *layout = new QVBoxLayout(container);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(6);

	auto *section = new MenuSectionWidget(container);
	Style::setStyle(section, style::properties::widget::border);
	section->contentLayout()->setSpacing(10);
	layout->addWidget(section);

	// Combos rather than spinboxes: both attributes accept only the values in the
	// device's own *_available list, and a free-entry box would let the reader ask
	// for a rate the part silently clamps.
	QComboBox *odrCombo = nullptr;
	QComboBox *hpfCombo = nullptr;

	if(!m_odrOptions.isEmpty()) {
		odrCombo = new QComboBox(section);
		for(double v : std::as_const(m_odrOptions)) {
			odrCombo->addItem(QString::number(v, 'g', 6), v);
		}
		section->contentLayout()->addWidget(
			labelledRow(QStringLiteral("Sampling frequency (Hz):"), odrCombo, section));
	} else {
		// No list means the attribute is absent on this firmware. Saying so beats
		// offering a combo whose every write fails.
		section->contentLayout()->addWidget(
			new QLabel(QStringLiteral("Sampling frequency: not reported by device"), section));
	}

	// Built empty and filled by the sync below, unlike the ODR combo: the corners
	// depend on the current rate, so this box's contents change under the reader
	// every time the rate does.
	if(!highPassOptions().isEmpty()) {
		hpfCombo = new QComboBox(section);
		section->contentLayout()->addWidget(
			labelledRow(QStringLiteral("High-pass corner (Hz):"), hpfCombo, section));
	}

	// What the device is actually running at, which is not always what a combo
	// asked for. Its own row so a clamped write is visible rather than inferred
	// from a combo that snapped back.
	auto *readback = new QLabel(section);
	Style::setStyle(readback, style::properties::label::subtle);
	section->contentLayout()->addWidget(readback);

	// One sync path, driven both at build time and by deviceConfigChanged, so the
	// widget shows the device's state rather than the last thing typed into it.
	auto sync = [this, odrCombo, hpfCombo, readback]() {
		const double sr = m_sampleRate.load(std::memory_order_relaxed);
		const double hpf = m_hpfFrequency.load(std::memory_order_relaxed);
		const QList<double> hpfOpts = highPassOptions();

		// Blocked: this reflects what the device already reported, so echoing it
		// back through a write would be a serial round trip for nothing.
		if(odrCombo) {
			QSignalBlocker b(odrCombo);
			const int idx = odrCombo->findData(sr);
			if(idx >= 0) {
				odrCombo->setCurrentIndex(idx);
			}
		}
		if(hpfCombo) {
			// Refilled, not just reselected: a rate change replaces the whole set
			// of corners the device accepts, and leaving the old numbers on screen
			// would let the reader pick one that reads back as 0.
			QSignalBlocker b(hpfCombo);
			hpfCombo->clear();
			for(double v : std::as_const(hpfOpts)) {
				// 0 is the device's own "no filtering" entry, printed as such
				// rather than as a 0 Hz corner.
				hpfCombo->addItem(v == 0.0 ? QStringLiteral("Disabled")
							   : QString::number(v, 'g', 6),
						  v);
			}
			const int idx = hpfCombo->findData(hpf);
			if(idx >= 0) {
				hpfCombo->setCurrentIndex(idx);
			}
		}

		readback->setText(QStringLiteral("Device: %1 Hz ODR, %2")
					  .arg(sr, 0, 'g', 6)
					  .arg(hpf == 0.0 ? QStringLiteral("no high-pass")
							  : QStringLiteral("%1 Hz high-pass").arg(hpf, 0, 'g', 4)));
	};
	sync();

	if(odrCombo) {
		connect(odrCombo, &QComboBox::currentIndexChanged, this, [this, odrCombo](int idx) {
			if(idx >= 0) {
				setSampleRate(odrCombo->itemData(idx).toDouble());
			}
		});
	}
	if(hpfCombo) {
		connect(hpfCombo, &QComboBox::currentIndexChanged, this, [this, hpfCombo](int idx) {
			if(idx >= 0) {
				setHighPassFrequency(hpfCombo->itemData(idx).toDouble());
			}
		});
	}

	// Queued: a pending write is applied by the worker between refills, so this
	// signal can arrive off the acquisition thread. `section` as context rather
	// than `this` — the block outlives the widget, and a sync into a deleted
	// combo would be a dangling read.
	connect(this, &Adxl355Source::deviceConfigChanged, section, sync, Qt::QueuedConnection);

	return withBaseSettings(container, parent);
}

// ---------------------------------------------------------------------------
// Stream descriptors
// ---------------------------------------------------------------------------

std::optional<scopy::acq::StreamInfo> Adxl355Source::streamInfo(const scopy::acq::DataKey &key) const
{
	const int accelIndex = kAccelChannels.indexOf(key.channelId());
	const bool isTemp = (key.channelId() == QString::fromLatin1(kTempChannel));

	// Anything not one of our four channels, or shaped as something other than a
	// raw key of ours, belongs to another block.
	if(key.sourceId() != id() || (accelIndex < 0 && !isTemp) || !key.isRaw()) {
		return std::nullopt;
	}

	scopy::acq::StreamInfo info;
	info.kind = scopy::acq::ReprKind::Curve;
	// The device's own ODR, so a view measures in seconds rather than samples
	// without the instrument having to guess it. Read back from the device, so a
	// clamped write is reflected here.
	info.sampleRate = m_sampleRate.load(std::memory_order_relaxed);

	if(isTemp) {
		info.label = QStringLiteral("Temperature");
		info.unit = QStringLiteral("°C");
		// Slot 3, after the accel triple, so temp is not drawn in accel_x's
		// colour when both are on one plot.
		info.colorIndex = kAccelChannels.size();
		return info;
	}

	// "accel_x" -> "Accel X". The key keeps the device's own channel name; the
	// label is what a plot shows.
	info.label = QStringLiteral("Accel %1").arg(kAccelChannels.at(accelIndex).mid(6).toUpper());
	// Only claim m/s² when the scale that produces it was actually read — see
	// writeAccelChannel(), which publishes raw LSBs when it was not.
	info.unit = (m_accelScale.load(std::memory_order_relaxed) != 0.0) ? QStringLiteral("m/s²")
									 : QStringLiteral("LSB");
	info.colorIndex = accelIndex;
	return info;
}

#include "moc_adxl355source.cpp"
