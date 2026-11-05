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

#ifndef MULTIACQ_IIOSOURCE_H
#define MULTIACQ_IIOSOURCE_H

#include <core/acq_engine/sourceblock.h>

#include <component/controller.h>

#include <QHash>
#include <QList>
#include <QMutex>
#include <QSet>
#include <QStringList>
#include <QVector>
#include <QWaitCondition>

#include <atomic>

class QThread;

namespace scopy {
namespace component {
class Attribute;
class Channel;
class Device;
class InputStream;
namespace iio {
class IIOSampleCodec;
class IIOScanElement;
} // namespace iio
} // namespace component

namespace multiacq {

// A source over one device-controller context, i.e. one URI. It owns the context handle, so
// the board stays connected for as long as the block lives.
//
// Each channel is read one of two ways, chosen per channel:
//   Attribute - one attribute read at the read interval; repeated reads make a "fictional
//               buffer".
//   Buffer    - samples streamed from the device's buffer, for channels that can stream.
// and its value is either the plain reading or, where the channel has the attributes,
// (raw + offset) * scale.
//
// Every IIO device gets a lane (a thread) per read mode in use. Lanes stamp every sample
// with wall-clock time and queue it; acquire() only drains the queue. So a slow or
// unreachable board stalls nothing but its own lanes, and channels of different boards line
// up by their timestamps.
//
// Each channel writes two keys: <id>_<dev:chan>_raw (float values) and
// <id>_<dev:chan>_time (double, ms since epoch). The raw key's StreamInfo names the time
// key as its X.
//
// Channel, mode and attribute choices are latched when the engine starts; configChanged()
// tells the owner a change is waiting for the next run.
class IioSource : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	enum class Mode
	{
		Polled,
		Buffered
	};

	struct ChannelDesc
	{
		QString channelId; // "<dev>:<iioId>"
		QString iioId;
		QStringList attributes; // readable attributes, for Polled
		bool bufferable{false};
		bool hasScale{false};
		bool hasOffset{false};
	};

	struct DeviceDesc
	{
		QString name;
		QList<ChannelDesc> channels;
		bool bufferable{false};
		// Publishes sampling_frequency, on the device or a channel.
		bool hasRate{false};
	};

	struct ChannelConfig
	{
		Mode mode{Mode::Polled};
		QString attribute; // for Polled
		// (raw + offset) * scale, where the channel can be scaled at all.
		bool scaled{true};
	};

	// Whether `cfg` reads a raw value that the channel's scale/offset can convert: a buffer
	// sample always is one, an attribute only when it is "raw".
	static bool canScale(const ChannelDesc &ch, const ChannelConfig &cfg);
	// The conversion `cfg` applies, naming the attributes it uses; empty for none.
	static QString formula(const ChannelDesc &ch, const ChannelConfig &cfg);
	// One line for the settings page: the value plotted and the attributes it comes from.
	static QString describe(const ChannelDesc &ch, const ChannelConfig &cfg);

	// `id` becomes the DataKey source id, so it must not contain '_'.
	IioSource(const QString &id, const QString &uri, scopy::component::ContextHandle ctx,
		  QObject *parent = nullptr);
	~IioSource() override;

	QString uri() const { return m_uri; }
	// What the reader sees; set once, before the engine runs.
	QString displayName() const { return m_displayName.isEmpty() ? id() : m_displayName; }
	void setDisplayName(const QString &name) { m_displayName = name; }
	QList<DeviceDesc> devices() const { return m_descs; }

	static scopy::acq::DataKey timeKey(const QString &sourceId, const QString &channelId);

	// GUI thread. All latched at the next start.
	ChannelConfig config(const QString &channelId) const;
	void setConfig(const QString &channelId, const ChannelConfig &cfg);
	// For a device that publishes no sampling_frequency: the rate its buffer samples are
	// spaced at, Hz. 0 spreads them over the time between refills.
	double rateOverride(const QString &device) const;
	void setRateOverride(const QString &device, double hz);
	int readIntervalMs() const { return m_readIntervalMs.load(); }
	void setReadIntervalMs(int ms);

	// False while any lane fails to read its device.
	bool isOnline() const { return m_online.load(); }

	bool isAvailable() const override;
	void onStart() override;
	void acquire(scopy::acq::DataStore *store) override;
	void onStop() override;
	void requestStop() override;
	QList<scopy::acq::DataKey> outputKeys() const override;
	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;
	QWidget *createSettingsWidget(QWidget *parent = nullptr) override;

Q_SIGNALS:
	// A setting changed that only takes effect on the next start.
	void configChanged();
	// From a lane thread; connect queued.
	void onlineChanged(bool online);
	// From a lane thread; connect queued. The rate `device`'s buffer samples are spaced at, and
	// where it came from.
	void bufferRateChanged(const QString &device, double hz, const QString &from);

private:
	struct Chan
	{
		ChannelDesc desc;
		scopy::component::Channel *channel{nullptr};
		scopy::component::iio::IIOScanElement *element{nullptr};
		scopy::component::iio::IIOSampleCodec *codec{nullptr};
	};

	struct Dev
	{
		scopy::component::Device *device{nullptr};
		scopy::component::InputStream *stream{nullptr};
		QString name;
		QList<Chan> channels;
	};

	// Samples a lane produced since the last acquire().
	struct Pending
	{
		QVector<double> t;
		QVector<float> v;
	};

	// How a lane turns a reading into a value.
	struct Conversion
	{
		double scale{1.0};
		double offset{0.0};
		float apply(double raw) const { return static_cast<float>((raw + offset) * scale); }
	};

	void discover();
	// One channel's rows on the settings page: enable, read mode, attribute, value.
	QWidget *createChannelSettings(const ChannelDesc &ch, QWidget *parent);
	// One lane's whole life, on its own thread.
	void runPolled(const Dev &dev, const QList<Chan> &chans, const QList<ChannelConfig> &cfgs);
	void runBuffered(const Dev &dev, const QList<Chan> &chans, const QList<ChannelConfig> &cfgs);
	// Reads the channel's scale and offset when `cfg` wants them. Lane thread.
	static Conversion conversion(const Chan &ch, const ChannelConfig &cfg);
	void push(const QString &channelId, const QVector<double> &t, const QVector<float> &v);
	// A lane's verdict on its device; `why` is reported when it starts failing.
	void setDeviceOk(const QString &device, bool ok, const QString &why = QString());
	void updateOnline();
	// Sleeps a lane for `ms`, waking early on stop. False once stopping.
	bool laneSleep(int ms);
	void stopLanes();

	const QString m_uri;
	QString m_displayName;
	scopy::component::ContextHandle m_ctx;
	QList<Dev> m_devices;	     // fixed by discover()
	QList<DeviceDesc> m_descs; // the same, for the UI

	mutable QMutex m_cfgMutex;
	QHash<QString, ChannelConfig> m_configs;
	QHash<QString, double> m_rateOverrides;
	std::atomic<int> m_readIntervalMs{100};

	QList<QThread *> m_lanes;
	std::atomic<bool> m_lanesStop{false};
	QMutex m_sleepMutex;
	QWaitCondition m_sleepCond;

	QMutex m_pendingMutex;
	QWaitCondition m_pendingCond;
	QHash<QString, Pending> m_pending;

	QMutex m_failingMutex;
	QSet<QString> m_failing; // devices whose lane cannot read them
	std::atomic<bool> m_online{true};
};

} // namespace multiacq
} // namespace scopy

#endif // MULTIACQ_IIOSOURCE_H
