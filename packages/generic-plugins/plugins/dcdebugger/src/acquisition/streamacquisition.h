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

#ifndef STREAMACQUISITION_H
#define STREAMACQUISITION_H

#include "scopy-dcdebugger_export.h"

#include <QList>
#include <QObject>
#include <QPointer>
#include <QStringList>
#include <QVector>

#include <component/stream.h>
#include <core/result.h>

namespace scopy::component {
class InputStream;
class ScanElement;
} // namespace scopy::component

namespace scopy::dcdebugger {

// One refilled buffer, de-interleaved and scaled: data[i] belongs to the scan element ids[i].
struct AcquiredFrame
{
	QStringList ids;
	QVector<QVector<double>> data;
	quint64 index = 0;
};

// Drives one InputStream through the component interface: open with the selected scan
// elements, refill in a loop (or once), decode every buffer, close when stopped. Config
// changes while running are applied by reopening the stream between two refills.
class SCOPY_DCDEBUGGER_EXPORT StreamAcquisition : public QObject
{
	Q_OBJECT
public:
	explicit StreamAcquisition(component::InputStream *stream, QObject *parent = nullptr);
	~StreamAcquisition() override;

	component::InputStream *stream() const;
	QList<component::ScanElement *> scanElements() const;

	QList<int> enabledChannels() const { return m_enabled; }
	void setEnabledChannels(const QList<int> &indices);
	size_t samples() const { return m_samples; }
	void setSamples(size_t samples);

	bool isRunning() const { return m_running; }

	// Decodes the stream's current buffer (valid after a refill).
	static AcquiredFrame decode(component::InputStream *stream);

public Q_SLOTS:
	// Continuous acquisition, or one buffer when single is set.
	void start(bool single = false);
	// Finishes the refill in flight, then closes the stream.
	void stop();
	// Teardown: waits for the stream to close (nested event loop). Only for disconnect.
	void shutdown();

Q_SIGNALS:
	void runningChanged(bool running);
	void frameReady(const scopy::dcdebugger::AcquiredFrame &frame);
	void errorOccurred(const QString &message);

private:
	void open();
	void refill();
	void next();
	void close();
	void fail(const QString &what, const scopy::Error &error);
	void setRunning(bool running);

	QPointer<component::InputStream> m_stream;
	component::StreamConfig m_config; // outlives the open in flight
	QList<int> m_enabled;
	size_t m_samples = 4096;
	bool m_running = false;
	bool m_continuous = true;
	bool m_stopRequested = false;
	bool m_reconfigure = false;
	bool m_busy = false; // an open / refill / close is in flight
	quint64 m_frames = 0;
};

// First sample index in [from, to) where `data` crosses `level` with the given edge, or -1. The
// trigger arms only once the signal is more than `hysteresis` on the other side of the level,
// so noise around the level does not fire it.
SCOPY_DCDEBUGGER_EXPORT int findEdge(const QVector<double> &data, double level, bool rising, int from = 0, int to = -1,
				     double hysteresis = 0);

} // namespace scopy::dcdebugger

Q_DECLARE_METATYPE(scopy::dcdebugger::AcquiredFrame)

#endif // STREAMACQUISITION_H
