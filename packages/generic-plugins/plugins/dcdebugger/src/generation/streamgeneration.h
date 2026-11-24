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

#ifndef STREAMGENERATION_H
#define STREAMGENERATION_H

#include "scopy-dcdebugger_export.h"
#include "waveform.h"

#include <QHash>
#include <QObject>
#include <QPointer>

#include <component/stream.h>
#include <core/result.h>

namespace scopy::component {
class OutputStream;
class Channel;
} // namespace scopy::component

namespace scopy::dcdebugger {

// Drives one OutputStream through the component interface: open with the enabled tones' member
// channels, encode every tone into the write buffer, push. Cyclic streams are pushed once and
// replay on the device; others are refilled and pushed in a loop. Changes while running are
// applied by reopening (cyclic, or new channels / samples) or at the next push (tone values).
class SCOPY_DCDEBUGGER_EXPORT StreamGeneration : public QObject
{
	Q_OBJECT
public:
	explicit StreamGeneration(component::OutputStream *stream, QObject *parent = nullptr);
	~StreamGeneration() override;

	component::OutputStream *stream() const;
	QList<component::Channel *> channels() const;

	// Tones by member channel id; channels without a tone, or with enabled = false, stay off.
	QHash<QString, ToneSpec> tones() const { return m_tones; }
	void setTones(const QHash<QString, ToneSpec> &tones);
	size_t samples() const { return m_samples; }
	void setSamples(size_t samples);
	double sampleRate() const { return m_fs; }
	void setSampleRate(double fs);
	bool supportsCyclic() const;
	bool cyclic() const { return m_cyclic; }
	void setCyclic(bool cyclic);

	bool isRunning() const { return m_running; }
	quint64 pushes() const { return m_pushes; }

	// The tones as they are encoded: frequencies snapped to the buffer.
	QHash<QString, ToneSpec> snappedTones() const;

public Q_SLOTS:
	void start();
	void stop();
	// Teardown: waits for the stream to close (nested event loop). Only for disconnect.
	void shutdown();

Q_SIGNALS:
	void runningChanged(bool running);
	void pushed(int channels, quint64 samples, bool cyclic);
	void errorOccurred(const QString &message);

private:
	QList<int> enabledIndices() const;
	void open();
	void fillAndPush();
	void next();
	void close();
	void fail(const QString &what, const scopy::Error &error);
	void setRunning(bool running);
	void reconfigure(bool reopen);

	QPointer<component::OutputStream> m_stream;
	component::StreamConfig m_config; // outlives the open in flight
	QHash<QString, ToneSpec> m_tones;
	size_t m_samples = 8192;
	double m_fs = 0;
	bool m_cyclic = true;

	bool m_running = false;
	bool m_stopRequested = false;
	bool m_reopen = false; // apply by reopening at the next opportunity
	bool m_busy = false;
	quint64 m_pushes = 0;
};

} // namespace scopy::dcdebugger

#endif // STREAMGENERATION_H
