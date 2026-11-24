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

#include "streamgeneration.h"

#include <QLoggingCategory>
#include <QTimer>

#include <component/outputstream.h>
#include <component/channel.h>
#include <component/streamwriter.h>
#include <qcoro/qcorotask.h>

Q_LOGGING_CATEGORY(CAT_DCDEBUGGER_GEN, "DCDebuggerGeneration")

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

StreamGeneration::StreamGeneration(OutputStream *stream, QObject *parent)
	: QObject(parent)
	, m_stream(stream)
{
	m_cyclic = supportsCyclic();
	if(m_stream && m_stream->samplesCount() > 0) {
		m_samples = m_stream->samplesCount();
	}
	// Default: every member channel, 0 / 90 degree sines (an I/Q pair on quadrature DACs).
	int i = 0;
	for(Channel *el : channels()) {
		ToneSpec t;
		t.phase = (i++ % 2) ? 90 : 0;
		m_tones.insert(el->id(), t);
	}
}

StreamGeneration::~StreamGeneration() = default;

OutputStream *StreamGeneration::stream() const { return m_stream; }

QList<Channel *> StreamGeneration::channels() const { return m_stream ? m_stream->channels() : QList<Channel *>{}; }

bool StreamGeneration::supportsCyclic() const { return m_stream && m_stream->supportsCyclic(); }

void StreamGeneration::setTones(const QHash<QString, ToneSpec> &tones)
{
	const bool channelsChanged = [&]() {
		for(Channel *el : channels()) {
			if(m_tones.value(el->id()).enabled != tones.value(el->id()).enabled) {
				return true;
			}
		}
		return false;
	}();
	m_tones = tones;
	// A cyclic buffer replays what was pushed: new values need a new buffer.
	reconfigure(channelsChanged || m_cyclic);
}

void StreamGeneration::setSamples(size_t samples)
{
	if(samples == m_samples) {
		return;
	}
	m_samples = samples;
	reconfigure(true);
}

void StreamGeneration::setSampleRate(double fs)
{
	if(fs == m_fs) {
		return;
	}
	m_fs = fs;
	reconfigure(m_cyclic);
}

void StreamGeneration::setCyclic(bool cyclic)
{
	cyclic = cyclic && supportsCyclic();
	if(cyclic == m_cyclic) {
		return;
	}
	m_cyclic = cyclic;
	reconfigure(true);
}

QHash<QString, ToneSpec> StreamGeneration::snappedTones() const
{
	QHash<QString, ToneSpec> out = m_tones;
	for(auto it = out.begin(); it != out.end(); ++it) {
		it->frequency = snapFrequency(it->frequency, m_fs, int(m_samples));
	}
	return out;
}

QList<int> StreamGeneration::enabledIndices() const
{
	QList<int> out;
	for(Channel *el : channels()) {
		auto it = m_tones.constFind(el->id());
		if(it != m_tones.cend() && it->enabled) {
			out.append(int(el->streamIndex()));
		}
	}
	return out;
}

void StreamGeneration::reconfigure(bool reopen)
{
	if(!m_running) {
		return;
	}
	if(reopen) {
		m_reopen = true;
	}
	// Cyclic streams sit idle after their push: apply now. Looping streams pick it up at the
	// next push.
	if(m_cyclic && !m_busy) {
		QTimer::singleShot(0, this, &StreamGeneration::next);
	}
}

void StreamGeneration::start()
{
	if(!m_stream || m_running) {
		return;
	}
	m_stopRequested = false;
	m_reopen = false;
	setRunning(true);
	open();
}

void StreamGeneration::stop()
{
	if(!m_running) {
		return;
	}
	m_stopRequested = true;
	if(!m_busy) {
		close();
	}
}

void StreamGeneration::shutdown()
{
	m_stopRequested = true;
	if(m_stream && m_stream->isOpen()) {
		QCoro::waitFor(m_stream->closeAsync());
	}
	m_busy = false;
	setRunning(false);
}

void StreamGeneration::open()
{
	const QList<int> enabled = enabledIndices();
	if(enabled.isEmpty()) {
		Q_EMIT errorOccurred("No channel enabled");
		close();
		return;
	}
	m_busy = true;
	if(m_stream->isOpen()) {
		QCoro::connect(m_stream->closeAsync(), this, [this](const CommandResponse<void> &r) {
			m_busy = false;
			if(!r) {
				fail("close", r.error());
				return;
			}
			open();
		});
		return;
	}
	if(Result<void> r = m_stream->setCyclic(m_cyclic); !r) {
		m_busy = false;
		fail("cyclic", r.error());
		return;
	}
	m_config.enabledChannels = enabled;
	m_config.samplesCount = m_samples;
	QCoro::connect(m_stream->openAsync(m_config), this, [this](const CommandResponse<void> &r) {
		m_busy = false;
		if(!r) {
			fail("open", r.error());
			return;
		}
		if(m_stopRequested) {
			close();
			return;
		}
		fillAndPush();
	});
}

void StreamGeneration::fillAndPush()
{
	StreamFormat &fmt = m_stream->writeFormat();
	StreamWriter writer(fmt);
	const int samples = int(writer.sampleCount());
	// Each writeFormat() entry names the member channel it describes by streamIndex. The
	// backend may enable more channels than requested: those without an enabled tone get 0.
	int ch = 0;
	for(; ch < writer.channelCount(); ++ch) {
		const Channel *member = m_stream->channelAt(fmt.channels.at(ch).streamIndex);
		ToneSpec tone = member ? m_tones.value(member->id()) : ToneSpec{};
		QVector<double> v(samples, 0.0);
		if(member && tone.enabled) {
			tone.frequency = snapFrequency(tone.frequency, m_fs, samples);
			v = generateTone(tone, samples, m_fs);
		}
		for(int s = 0; s < v.size(); ++s) {
			writer.setNormalized(ch, size_t(s), v[s]);
		}
	}

	m_busy = true;
	QCoro::connect(m_stream->pushAsync(), this, [this, ch](const CommandResponse<void> &r) {
		m_busy = false;
		if(!m_stream || !m_stream->isOpen()) {
			setRunning(false);
			return;
		}
		if(!r) {
			fail("push", r.error());
			return;
		}
		m_pushes++;
		Q_EMIT pushed(ch, m_stream->samplesCount(), m_cyclic);
		if(m_stopRequested) {
			close();
		} else if(!m_cyclic || m_reopen) {
			// Looping streams: next buffer from the event loop (synchronous backends must not
			// starve the GUI). Cyclic ones only come back here to apply a change.
			QTimer::singleShot(0, this, &StreamGeneration::next);
		}
	});
}

void StreamGeneration::next()
{
	if(m_busy) {
		return;
	}
	if(!m_stream || !m_stream->isOpen()) {
		setRunning(false);
		return;
	}
	if(m_stopRequested) {
		close();
	} else if(m_reopen) {
		m_reopen = false;
		open(); // closes, then reopens with the new configuration
	} else if(!m_cyclic) {
		fillAndPush();
	}
}

void StreamGeneration::close()
{
	if(!m_stream || !m_stream->isOpen()) {
		setRunning(false);
		return;
	}
	m_busy = true;
	QCoro::connect(m_stream->closeAsync(), this, [this](const CommandResponse<void> &r) {
		m_busy = false;
		if(!r) {
			Q_EMIT errorOccurred("close failed: " + r.error().errorString());
		}
		setRunning(false);
	});
}

void StreamGeneration::fail(const QString &what, const scopy::Error &error)
{
	const QString msg = what + " failed: " + error.errorString();
	qWarning(CAT_DCDEBUGGER_GEN) << msg;
	Q_EMIT errorOccurred(msg);
	m_stopRequested = true;
	close();
}

void StreamGeneration::setRunning(bool running)
{
	if(m_running == running) {
		return;
	}
	m_running = running;
	Q_EMIT runningChanged(running);
}

#include "moc_streamgeneration.cpp"
