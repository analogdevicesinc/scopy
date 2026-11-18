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

#pragma once

#include <core/acq_engine/sourceblock.h>

#include <QByteArray>
#include <QList>
#include <QMutex>
#include <QString>

#include <atomic>

namespace scopy::component {
class Channel;
class Device;
} // namespace scopy::component

namespace scopy::adalm2000 {
class M2kContext;
class M2kDigitalInputStream;
class M2kInputStream;

// The M2K inputs: analog "voltage0".."voltage1" plus "time", digital "dio0".."dio15". An input
// streams only when one of its channels is enabled at start. The tool sets the hardware up with
// set() on device() and channel(): every value is written on start, and one changed while running
// is written between buffers.
class M2kSource : public scopy::acq::SourceBlock
{
	Q_OBJECT
public:
	enum Input
	{
		Analog,
		Digital,
		INPUT_COUNT
	};

	enum Apply
	{
		Write,
		// Write, then drop the queued buffers and rearm the trigger.
		Restart,
		// Close the stream and open it again with every value written.
		Reopen,
	};

	enum Unit
	{
		Volts,
		// Counts over spec::FULL_SCALE_COUNTS, independent of the range.
		FullScale,
	};

	struct Setting
	{
		QObject *node;
		QByteArray attr;
		QString value;
	};

	static constexpr int ANALOG_CHANNELS = 2;
	static constexpr int DIGITAL_CHANNELS = 16;

	explicit M2kSource(M2kContext *ctx, const QString &id = QStringLiteral("m2k"), QObject *parent = nullptr);

	static QString channelId(Input in, int i);
	static QString timeChannelId() { return QStringLiteral("time"); }
	scopy::acq::DataKey rawKey(Input in, int i) const;
	// Seconds from the trigger, per sample of the latest analog buffer.
	scopy::acq::DataKey timeKey() const { return scopy::acq::DataKey::raw(id(), timeChannelId()); }

	scopy::component::Device *device(Input in) const { return m_dev[in]; }
	scopy::component::Channel *channel(Input in, int i) const;
	M2kInputStream *analogStream() const { return m_ain; }
	M2kDigitalInputStream *digitalStream() const { return m_din; }

	void set(QObject *node, const char *attr, const QString &value, Apply apply = Write);
	QString value(QObject *node, const char *attr) const;
	void setKernelBuffers(Input in, unsigned n);
	void setAnalogUnit(Unit unit) { m_unit = unit; }
	// No trigger within timeoutMs past the buffer's duration: one buffer taken with `untriggered`,
	// then the trigger rearmed. Zero turns it off.
	void setAutoTrigger(Input in, int timeoutMs, const QList<Setting> &untriggered);
	// Read after every open and every write while running, and reported by attributeRead().
	void watch(QObject *node, const char *attr);
	// Read before the first write on start and written back on stop, for state other tools share.
	void restoreOnStop(QObject *node, const char *attr);

	// Of the latest buffer; zero until a start reads the rate back.
	double sampleRate(Input in) const { return m_rate[in].load(); }
	int bufferDelay(Input in) const { return m_bufDelay[in].load(); }
	bool lastTimedOut(Input in) const { return m_timedOut[in].load(); }

	void onStart() override;
	void acquire(scopy::acq::DataStore *store) override;
	void onStop() override;
	void requestStop() override;
	std::optional<scopy::acq::StreamInfo> streamInfo(const scopy::acq::DataKey &key) const override;

Q_SIGNALS:
	void sampleRateChanged(int input, double hz);
	// On the worker thread, before the buffer it applies to is published.
	void attributeRead(QObject *node, const QByteArray &attr, const QString &value);

private:
	struct Entry
	{
		Setting setting;
		Apply apply;
		bool dirty;
	};

	int indexOf(QObject *node, const char *attr) const;
	Input inputOf(QObject *node) const;
	bool hasStream(Input in) const;
	void cancel(Input in);
	void invalidate(Input in);
	bool wanted(Input in) const;
	void written(Input in, const Setting &s);
	void open(Input in);
	void close(Input in);
	void stopAll();
	void saveRestored();
	void restore();
	void applyChanges();
	void readWatched();
	bool refill(Input in);
	void publish(Input in, scopy::acq::DataStore *store);

	scopy::component::Device *m_dev[INPUT_COUNT]{};
	M2kInputStream *m_ain{nullptr};
	M2kDigitalInputStream *m_din{nullptr};

	mutable QMutex m_mutex;
	QList<Entry> m_entries;
	QList<Setting> m_watched;
	QList<Setting> m_restored;
	QList<Setting> m_saved;
	unsigned m_kernelBuffers[INPUT_COUNT]{1, 1};
	bool m_reopen[INPUT_COUNT]{};
	int m_autoMs[INPUT_COUNT]{};
	QList<Setting> m_untriggered[INPUT_COUNT];

	std::atomic<Unit> m_unit{Volts};
	bool m_open[INPUT_COUNT]{};
	qint64 m_streamed[INPUT_COUNT]{};
	int m_armedDelay[INPUT_COUNT]{};
	bool m_armedStreaming[INPUT_COUNT]{};
	std::atomic<double> m_rate[INPUT_COUNT]{};
	std::atomic<int> m_bufDelay[INPUT_COUNT]{};
	std::atomic<bool> m_timedOut[INPUT_COUNT]{};
	std::atomic<bool> m_started{false};
	std::atomic<bool> m_interrupted{false};
};
} // namespace scopy::adalm2000
