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

#include "streamacquisition.h"

#include <QLoggingCategory>
#include <QTimer>

#include <algorithm>

#include <component/inputstream.h>
#include <component/scanelement.h>
#include <component/streamview.h>
#include <qcoro/qcorotask.h>

Q_LOGGING_CATEGORY(CAT_DCDEBUGGER_ACQ, "DCDebuggerAcquisition")

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

StreamAcquisition::StreamAcquisition(InputStream *stream, QObject *parent)
	: QObject(parent)
	, m_stream(stream)
{
	qRegisterMetaType<AcquiredFrame>();
	// Default selection: what the stream already has enabled, else every element.
	const QList<ScanElement *> elems = scanElements();
	for(ScanElement *el : elems) {
		if(el->isEnabled()) {
			m_enabled.append(int(el->index()));
		}
	}
	if(m_enabled.isEmpty()) {
		for(ScanElement *el : elems) {
			m_enabled.append(int(el->index()));
		}
	}
	if(m_stream && m_stream->samplesCount() > 0) {
		m_samples = m_stream->samplesCount();
	}
}

StreamAcquisition::~StreamAcquisition() = default;

InputStream *StreamAcquisition::stream() const { return m_stream; }

QList<ScanElement *> StreamAcquisition::scanElements() const
{
	if(!m_stream) {
		return {};
	}
	return m_stream->findChildren<ScanElement *>(QString(), Qt::FindDirectChildrenOnly);
}

void StreamAcquisition::setEnabledChannels(const QList<int> &indices)
{
	QList<int> sorted = indices;
	std::sort(sorted.begin(), sorted.end());
	if(sorted == m_enabled) {
		return;
	}
	m_enabled = sorted;
	m_reconfigure = m_running;
}

void StreamAcquisition::setSamples(size_t samples)
{
	if(samples == m_samples) {
		return;
	}
	m_samples = samples;
	m_reconfigure = m_running;
}

void StreamAcquisition::start(bool single)
{
	if(!m_stream || m_running) {
		return;
	}
	m_continuous = !single;
	m_stopRequested = false;
	m_reconfigure = false;
	setRunning(true);
	open();
}

void StreamAcquisition::stop()
{
	if(!m_running) {
		return;
	}
	m_stopRequested = true;
	if(!m_busy) {
		close();
	}
}

void StreamAcquisition::shutdown()
{
	m_stopRequested = true;
	if(m_stream && m_stream->isOpen()) {
		QCoro::waitFor(m_stream->closeAsync());
	}
	m_busy = false;
	setRunning(false);
}

void StreamAcquisition::open()
{
	if(m_enabled.isEmpty()) {
		Q_EMIT errorOccurred("No channel enabled");
		setRunning(false);
		return;
	}
	m_busy = true;
	if(m_stream->isOpen()) {
		// Someone (or a previous run) left it open: reopen with this configuration.
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
	m_config.enabledChannels = m_enabled;
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
		refill();
	});
}

void StreamAcquisition::refill()
{
	m_busy = true;
	QCoro::connect(m_stream->refillAsync(), this, [this](const CommandResponse<void> &r) {
		m_busy = false;
		if(!m_stream || !m_stream->isOpen()) {
			// Closed under us (shutdown): the buffer is gone.
			setRunning(false);
			return;
		}
		if(!r) {
			fail("refill", r.error());
			return;
		}
		AcquiredFrame frame = decode(m_stream);
		frame.index = ++m_frames;
		Q_EMIT frameReady(frame);

		if(!m_continuous || m_stopRequested) {
			close();
		} else {
			// Next buffer from the event loop: a backend that completes synchronously must
			// not starve the GUI.
			QTimer::singleShot(0, this, &StreamAcquisition::next);
		}
	});
}

void StreamAcquisition::next()
{
	if(m_busy) {
		return; // stop() already started closing
	}
	if(!m_stream || !m_stream->isOpen()) {
		setRunning(false);
		return;
	}
	if(m_stopRequested) {
		close();
	} else if(m_reconfigure) {
		m_reconfigure = false;
		open(); // closes, then reopens with the new configuration
	} else {
		refill();
	}
}

void StreamAcquisition::close()
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

void StreamAcquisition::fail(const QString &what, const scopy::Error &error)
{
	const QString msg = what + " failed: " + error.errorString();
	qWarning(CAT_DCDEBUGGER_ACQ) << msg;
	Q_EMIT errorOccurred(msg);
	m_stopRequested = true;
	close();
}

void StreamAcquisition::setRunning(bool running)
{
	if(m_running == running) {
		return;
	}
	m_running = running;
	Q_EMIT runningChanged(running);
}

AcquiredFrame StreamAcquisition::decode(InputStream *stream)
{
	AcquiredFrame frame;
	if(!stream) {
		return frame;
	}
	const StreamFormat &fmt = stream->readFormat();
	// StreamFormat::channels lists the enabled scan elements in child order.
	QStringList ids;
	for(ScanElement *el : stream->findChildren<ScanElement *>(QString(), Qt::FindDirectChildrenOnly)) {
		if(el->isEnabled()) {
			ids.append(el->id());
		}
	}
	StreamView view(fmt);
	for(int c = 0; c < view.channelCount(); ++c) {
		frame.ids.append(c < ids.size() ? ids[c] : QStringLiteral("channel %1").arg(c));
		frame.data.append(view.toDoubles(c));
		frame.raw.append(view.toRawDoubles(c));
		frame.bits.append(fmt.channels.at(c).validBits);
	}
	return frame;
}

int scopy::dcdebugger::findEdge(const QVector<double> &data, double level, bool rising, int from, int to,
				double hysteresis)
{
	if(to < 0 || to > data.size()) {
		to = data.size();
	}
	bool armed = false;
	for(int i = std::max(from, 0); i < to; ++i) {
		const double v = data[i];
		if(rising) {
			if(v < level - hysteresis) {
				armed = true;
			} else if(armed && v >= level) {
				return i;
			}
		} else {
			if(v > level + hysteresis) {
				armed = true;
			} else if(armed && v <= level) {
				return i;
			}
		}
	}
	return -1;
}

#include "moc_streamacquisition.cpp"
