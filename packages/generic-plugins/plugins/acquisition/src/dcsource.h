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

#ifndef ACQ_DCSOURCE_H
#define ACQ_DCSOURCE_H

#include <core/acq_engine/sourceblock.h>

#include <QHash>
#include <QList>
#include <QMutex>
#include <QString>

namespace scopy {
namespace component {
class Context;
class Device;
class InputStream;
namespace iio {
class IIOSampleCodec;
class IIOScanElement;
} // namespace iio
} // namespace component

namespace adc {

// The generic acquisition source over a device-controller context: it streams from any device
// that has a readable buffer, so a new part needs no block of its own.
//
// One block holds *every* streamable device in the context rather than one block per device.
// The rail therefore shows a single "DC" row, and channel ids carry the device —
// "<deviceName>:<iioChannelId>", e.g. "adxl355:accel_x" — so two devices that each have a
// "voltage0" stay distinct. This is also why DataKey's channel component must tolerate
// underscores.
//
// Threading: the engine calls onStart() on the GUI thread before the worker exists, then
// acquire() and onStop() on the worker, which has no running event loop. The component API is
// coroutine-based, so every call here goes through QCoro::waitFor(), whose nested QEventLoop
// pumps the calling thread's dispatcher while a pooled executor thread runs the command.
// Nothing in this block touches libiio.
class DcSource : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	explicit DcSource(scopy::component::Context *ctx, const QString &id = QStringLiteral("dc"),
			  QObject *parent = nullptr);
	~DcSource() override;

	bool isAvailable() const override;

	void onStart() override;
	void acquire(scopy::acq::DataStore *store) override;
	void onStop() override;

	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;

	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

private:
	// One streamable channel: the scan element gives the mask index, the codec decodes raw
	// bytes in the channel's own declared format, and channelId is what DataKeys carry.
	struct Chan
	{
		QString channelId; // "<dev>:<iioId>"
		QString iioId;	   // "accel_x"
		scopy::component::iio::IIOScanElement *element{nullptr};
		scopy::component::iio::IIOSampleCodec *codec{nullptr};
	};

	struct Dev
	{
		scopy::component::Device *device{nullptr};
		scopy::component::InputStream *stream{nullptr};
		QString name;
		// Every input scan element, ascending by index — the order buildStreamFormat()
		// appends enabled channels in, so a position here is a position there.
		QList<Chan> channels;
		// The subset the stream was actually opened with, same order. Empty means closed,
		// so it doubles as the open flag. Decoding reads this rather than the live enable
		// flags: a channel switched on mid-run is not in the device's mask yet.
		QList<Chan> open;
	};

	// Ctor-time tree walk: every direct-child Device with an input stream that has at least
	// one input scan element. No I/O, so it is safe to call before onStart().
	void discover();

	// Read "sampling_frequency" off each open device, for streamInfo(). Nothing prefetches
	// the attribute cache, so this is one round trip per device per run.
	void readSampleRates();

	// One refilled buffer -> one DataStore write per open channel.
	void decodeAndWrite(const Dev &dev, scopy::acq::DataStore *store);

	// Blocking close of every open stream. Idempotent.
	void closeAll();

	scopy::component::Context *m_ctx{nullptr};
	// Mutated only from onStart()/onStop(), which the engine serialises against acquire().
	QList<Dev> m_devices;

	// The one member two threads touch: written by onStart(), read by streamInfo() from the
	// GUI thread at any time.
	QHash<QString, double> m_sampleRates;
	mutable QMutex m_rateMutex;
};

} // namespace adc
} // namespace scopy

#endif // ACQ_DCSOURCE_H
