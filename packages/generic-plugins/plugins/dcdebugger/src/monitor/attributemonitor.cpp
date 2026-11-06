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

#include "attributemonitor.h"

#include <QRegularExpression>
#include <QTimer>

#include <algorithm>
#include <limits>

#include <component/attribute.h>
#include <component/attributereader.h>
#include <qcoro/qcorotask.h>

using namespace scopy;
using namespace scopy::dcdebugger;
using namespace scopy::component;

AttributeMonitor::AttributeMonitor(QObject *parent)
	: QObject(parent)
	, m_timer(new QTimer(this))
{
	m_timer->setInterval(m_periodMs);
	connect(m_timer, &QTimer::timeout, this, &AttributeMonitor::tick);
}

AttributeMonitor::~AttributeMonitor() = default;

void AttributeMonitor::addItem(const QString &key, Attribute *source, Attribute *scale, Attribute *offset)
{
	if(!m_items.contains(key)) {
		m_order.append(key);
	}
	Item item;
	item.source = source;
	item.scale = scale;
	item.offset = offset;
	m_items.insert(key, item);
}

void AttributeMonitor::setSource(const QString &key, Attribute *source, Attribute *scale, Attribute *offset)
{
	auto it = m_items.find(key);
	if(it == m_items.end()) {
		return;
	}
	it->source = source;
	it->scale = scale;
	it->offset = offset;
	it->history.clear();
	it->error.clear();
	if(isRunning() && it->enabled) {
		refreshScale(*it);
	}
}

Attribute *AttributeMonitor::source(const QString &key) const { return m_items.value(key).source; }

void AttributeMonitor::setEnabled(const QString &key, bool enabled)
{
	auto it = m_items.find(key);
	if(it == m_items.end() || it->enabled == enabled) {
		return;
	}
	it->enabled = enabled;
	if(enabled && isRunning()) {
		refreshScale(*it);
	}
}

bool AttributeMonitor::isEnabled(const QString &key) const { return m_items.value(key).enabled; }

QStringList AttributeMonitor::enabledKeys() const
{
	QStringList out;
	for(const QString &key : m_order) {
		if(m_items.value(key).enabled) {
			out.append(key);
		}
	}
	return out;
}

void AttributeMonitor::setPeriod(int ms)
{
	m_periodMs = std::max(1, ms);
	m_timer->setInterval(m_periodMs);
}

void AttributeMonitor::setHistory(double seconds) { m_historyS = std::max(0.1, seconds); }

void AttributeMonitor::setScaled(bool scaled)
{
	if(scaled == m_scaled) {
		return;
	}
	m_scaled = scaled;
	// Mixed units in one history would be meaningless.
	for(Item &item : m_items) {
		item.history.clear();
	}
}

bool AttributeMonitor::isRunning() const { return m_timer->isActive(); }

double AttributeMonitor::now() const { return m_clock.isValid() ? m_clock.nsecsElapsed() / 1e9 : 0; }

void AttributeMonitor::start()
{
	if(isRunning()) {
		return;
	}
	if(!m_clock.isValid()) {
		m_clock.start();
	}
	for(const Item &item : std::as_const(m_items)) {
		if(item.enabled) {
			refreshScale(item);
		}
	}
	m_timer->start();
	Q_EMIT runningChanged(true);
	tick();
}

void AttributeMonitor::stop()
{
	if(!isRunning()) {
		return;
	}
	m_timer->stop();
	Q_EMIT runningChanged(false);
}

void AttributeMonitor::clear()
{
	for(Item &item : m_items) {
		item.history.clear();
		item.error.clear();
	}
	m_clock.invalidate();
	if(isRunning()) {
		m_clock.start();
	}
	Q_EMIT ticked();
}

void AttributeMonitor::tick()
{
	for(const QString &key : std::as_const(m_order)) {
		const Item &item = m_items[key];
		if(item.enabled && !item.inFlight && item.source && item.source->readCapability()) {
			poll(key);
		}
	}
	const double t = now();
	for(Item &item : m_items) {
		trim(item, t);
	}
	Q_EMIT ticked();
}

void AttributeMonitor::poll(const QString &key)
{
	Item &item = m_items[key];
	item.inFlight = true;
	QPointer<Attribute> source = item.source;
	QCoro::connect(
		source->readCapability()->readAsync(), this, [this, key, source](const CommandResponse<QByteArray> &r) {
			auto it = m_items.find(key);
			if(it == m_items.end()) {
				return;
			}
			it->inFlight = false;
			if(it->source != source) {
				return; // source changed while this read was in flight
			}
			if(!r) {
				it->error = r.error().errorString();
				Q_EMIT readFailed(key, it->error);
				return;
			}
			double v = 0;
			if(!parseValue(QString::fromUtf8(r.value()), v)) {
				it->error = QStringLiteral("not a number: ") + QString::fromUtf8(r.value()).trimmed();
				Q_EMIT readFailed(key, it->error);
				return;
			}
			it->error.clear();
			it->history.append(QPointF(now(), scaledValue(*it, v)));
			Q_EMIT sampled(key);
		});
}

void AttributeMonitor::refreshScale(const Item &item)
{
	// Scale / offset change rarely: read them when polling (re)starts, use the cached values.
	for(const QPointer<Attribute> &a : {item.scale, item.offset}) {
		if(a && a->readCapability()) {
			a->readCapability()->readAsync();
		}
	}
}

void AttributeMonitor::trim(Item &item, double now)
{
	const double oldest = now - m_historyS;
	auto firstKept = std::find_if(item.history.begin(), item.history.end(),
				      [oldest](const QPointF &p) { return p.x() >= oldest; });
	if(firstKept != item.history.begin()) {
		item.history.erase(item.history.begin(), firstKept);
	}
}

double AttributeMonitor::scaledValue(const Item &item, double raw) const
{
	if(!m_scaled || (!item.scale && !item.offset)) {
		return raw;
	}
	double scale = 1;
	double offset = 0;
	if(item.scale) {
		parseValue(item.scale->cachedValue(), scale);
	}
	if(item.offset) {
		parseValue(item.offset->cachedValue(), offset);
	}
	return (raw + offset) * scale;
}

QVector<QPointF> AttributeMonitor::samples(const QString &key) const { return m_items.value(key).history; }

AttributeMonitor::Stats AttributeMonitor::stats(const QString &key) const
{
	Stats s;
	const QVector<QPointF> h = m_items.value(key).history;
	if(h.isEmpty()) {
		return s;
	}
	s.value = h.last().y();
	s.min = std::numeric_limits<double>::max();
	s.max = std::numeric_limits<double>::lowest();
	double sum = 0;
	for(const QPointF &p : h) {
		s.min = std::min(s.min, p.y());
		s.max = std::max(s.max, p.y());
		sum += p.y();
	}
	s.count = h.size();
	s.mean = sum / s.count;
	return s;
}

QString AttributeMonitor::lastError(const QString &key) const { return m_items.value(key).error; }

bool AttributeMonitor::parseValue(const QString &text, double &value)
{
	static const QRegularExpression number(
		QStringLiteral("^\\s*([-+]?(?:\\d+\\.?\\d*|\\.\\d+)(?:[eE][-+]?\\d+)?)"));
	const QRegularExpressionMatch m = number.match(text);
	if(!m.hasMatch()) {
		return false;
	}
	bool ok = false;
	value = m.captured(1).toDouble(&ok);
	return ok;
}

#include "moc_attributemonitor.cpp"
