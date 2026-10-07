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

#include "digitalio_api.h"

#include "adalm2000digitaliotool.h"
#include "diogroupwidget.h"
#include "digitaliomath.h"
#include "m2kdigitaliocontroller.h"

#include "component/attribute.h"

#include <QLoggingCategory>

#include <qcoro/qcorotask.h>

Q_LOGGING_CATEGORY(CAT_DIGITALIO_API, "DigitalIO_API")

using namespace scopy;
using namespace scopy::adalm2000;

namespace {

// Blocks: a script expects its write to have landed before the next statement.
// Safe only here, where the call arrives from the script engine, not a UI slot.
bool writeSync(component::Attribute *attr, const QString &value)
{
	if(!attr || !attr->writeCapability()) {
		qWarning(CAT_DIGITALIO_API) << "attribute is missing or read-only";
		return false;
	}
	auto r = QCoro::waitFor(attr->writeCapability()->writeAsync(value));
	if(!r) {
		qWarning(CAT_DIGITALIO_API) << "write of" << value << "failed:" << r.error().message;
		return false;
	}
	return true;
}

} // namespace

DigitalIO_API::DigitalIO_API(Adalm2000DigitalIoTool *tool)
	: ApiObject()
	, m_tool(tool)
{}

DigitalIO_API::~DigitalIO_API() {}

QList<bool> DigitalIO_API::grouped() const
{
	QList<bool> list;
	for(int g = 0; g < dio::GROUP_COUNT; ++g) {
		DioGroupWidget *w = m_tool ? m_tool->groupWidget(g) : nullptr;
		list.append(w && w->isGrouped());
	}
	return list;
}

void DigitalIO_API::setGrouped(const QList<bool> &grouped)
{
	const int count = qMin<int>(grouped.size(), dio::GROUP_COUNT);
	for(int g = 0; g < count; ++g) {
		if(DioGroupWidget *w = m_tool ? m_tool->groupWidget(g) : nullptr) {
			w->setGrouped(grouped.at(g));
		}
	}
}

QList<bool> DigitalIO_API::direction() const
{
	QList<bool> list;
	M2kDigitalIoController *c = m_tool ? m_tool->m_controller : nullptr;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		component::Attribute *attr = c ? c->pinDirection(pin) : nullptr;
		list.append(attr && attr->cachedValue() == QLatin1String("out"));
	}
	return list;
}

void DigitalIO_API::setDirection(const QList<bool> &list)
{
	M2kDigitalIoController *c = m_tool ? m_tool->m_controller : nullptr;
	if(!c) {
		return;
	}
	const int count = qMin<int>(list.size(), dio::PIN_COUNT);
	for(int pin = 0; pin < count; ++pin) {
		writeSync(c->pinDirection(pin), list.at(pin) ? QStringLiteral("out") : QStringLiteral("in"));
	}
}

QList<bool> DigitalIO_API::output() const
{
	QList<bool> list;
	M2kDigitalIoController *c = m_tool ? m_tool->m_controller : nullptr;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		component::Attribute *attr = c ? c->pinValue(pin) : nullptr;
		list.append(attr && attr->cachedValue().toInt() != 0);
	}
	return list;
}

void DigitalIO_API::setOutput(const QList<bool> &list)
{
	M2kDigitalIoController *c = m_tool ? m_tool->m_controller : nullptr;
	if(!c) {
		return;
	}
	const int count = qMin<int>(list.size(), dio::PIN_COUNT);
	for(int pin = 0; pin < count; ++pin) {
		writeSync(c->pinValue(pin), list.at(pin) ? QStringLiteral("1") : QStringLiteral("0"));
	}
}

QList<bool> DigitalIO_API::gpi() const
{
	QList<bool> list;
	M2kDigitalIoController *c = m_tool ? m_tool->m_controller : nullptr;
	// What the poll last saw, not a fresh read: a script must allow a poll interval
	// after driving a pin before reading it back.
	const quint16 word = c ? c->gpi() : 0;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		list.append(dio::bit(word, pin));
	}
	return list;
}

QList<bool> DigitalIO_API::locked() const
{
	// Always false: the Pattern Generator lock is not implemented in this port.
	QList<bool> list;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		list.append(false);
	}
	return list;
}

bool DigitalIO_API::running() const { return m_tool && m_tool->isRunning(); }

void DigitalIO_API::run(bool en)
{
	if(m_tool) {
		m_tool->setRunningBlocking(en);
	}
}

#include "moc_digitalio_api.cpp"
