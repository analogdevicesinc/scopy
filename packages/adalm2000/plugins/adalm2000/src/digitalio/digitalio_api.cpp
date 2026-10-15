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

	// The read-after-write that Attribute::addWriteCapability fires is async, so
	// cachedValue() would still hold the old value when this returns. A script
	// expects the next statement to see the write, so settle it here.
	if(attr->readCapability()) {
		QCoro::waitFor(attr->readCapability()->readAsync());
	}
	return true;
}

} // namespace

DigitalIO_API::DigitalIO_API(Adalm2000DigitalIoTool *tool)
	: ApiObject()
	, m_tool(tool)
{}

DigitalIO_API::~DigitalIO_API() {}

M2kDigitalIoController *DigitalIO_API::controller() const { return m_tool ? m_tool->m_controller : nullptr; }

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

bool DigitalIO_API::setGroupMode(int group, bool grouped)
{
	DioGroupWidget *w = m_tool ? m_tool->groupWidget(group) : nullptr;
	if(!w) {
		qWarning(CAT_DIGITALIO_API) << "no group widget for group" << group;
		return false;
	}
	w->setGrouped(grouped);
	return true;
}

bool DigitalIO_API::isGroupMode(int group) const
{
	DioGroupWidget *w = m_tool ? m_tool->groupWidget(group) : nullptr;
	return w && w->isGrouped();
}

bool DigitalIO_API::setGroupDirection(int group, bool output)
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return false;
	}
	return writeSync(c->groupDirection(group),
			 output ? QLatin1String(dio::DIRECTION_OPTIONS[0]) : QLatin1String(dio::DIRECTION_OPTIONS[1]));
}

bool DigitalIO_API::isGroupOutput(int group) const
{
	M2kDigitalIoController *c = controller();
	component::Attribute *attr = c ? c->groupDirection(group) : nullptr;
	return attr && attr->cachedValue() == QLatin1String(dio::DIRECTION_OPTIONS[0]);
}

bool DigitalIO_API::setGroupValue(int group, int value)
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return false;
	}
	if(value < 0 || value > dio::GROUP_VALUE_MAX) {
		qWarning(CAT_DIGITALIO_API) << "group value" << value << "is outside 0.." << dio::GROUP_VALUE_MAX;
		return false;
	}
	return writeSync(c->groupValue(group), QString::number(value));
}

int DigitalIO_API::groupOutput(int group) const
{
	M2kDigitalIoController *c = controller();
	component::Attribute *attr = c ? c->groupValue(group) : nullptr;
	return attr ? attr->cachedValue().toInt() : -1;
}

int DigitalIO_API::groupInput(int group) const
{
	M2kDigitalIoController *c = controller();
	if(!c || !dio::validGroup(group)) {
		return -1;
	}
	return static_cast<int>(dio::groupValue(c->gpi(), group));
}

bool DigitalIO_API::setPinDirection(int pin, bool output)
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return false;
	}
	return writeSync(c->pinDirection(pin),
			 output ? QLatin1String(dio::DIRECTION_OPTIONS[0]) : QLatin1String(dio::DIRECTION_OPTIONS[1]));
}

bool DigitalIO_API::setPinOutput(int pin, bool high)
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return false;
	}
	return writeSync(c->pinValue(pin),
			 high ? QLatin1String(dio::VALUE_OPTIONS[0]) : QLatin1String(dio::VALUE_OPTIONS[1]));
}

int DigitalIO_API::pinOutput(int pin) const
{
	M2kDigitalIoController *c = controller();
	component::Attribute *attr = c ? c->pinValue(pin) : nullptr;
	return attr ? (attr->cachedValue().toInt() != 0 ? 1 : 0) : -1;
}

int DigitalIO_API::pinInput(int pin) const
{
	M2kDigitalIoController *c = controller();
	if(!c || !dio::validPin(pin)) {
		return -1;
	}
	return dio::bit(c->gpi(), pin) ? 1 : 0;
}

bool DigitalIO_API::setDirectionWord(int word)
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return false;
	}
	if(word < 0 || word > 0xFFFF) {
		qWarning(CAT_DIGITALIO_API) << "direction word" << word << "is outside 0..0xFFFF";
		return false;
	}
	bool ok = true;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		const bool out = dio::bit(static_cast<quint16>(word), pin);
		ok = writeSync(c->pinDirection(pin),
			       out ? QLatin1String(dio::DIRECTION_OPTIONS[0])
				   : QLatin1String(dio::DIRECTION_OPTIONS[1])) &&
			ok;
	}
	return ok;
}

bool DigitalIO_API::setOutputWord(int word)
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return false;
	}
	if(word < 0 || word > 0xFFFF) {
		qWarning(CAT_DIGITALIO_API) << "output word" << word << "is outside 0..0xFFFF";
		return false;
	}
	bool ok = true;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		const bool high = dio::bit(static_cast<quint16>(word), pin);
		ok = writeSync(c->pinValue(pin),
			       high ? QLatin1String(dio::VALUE_OPTIONS[0]) : QLatin1String(dio::VALUE_OPTIONS[1])) &&
			ok;
	}
	return ok;
}

int DigitalIO_API::directionWord() const
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return -1;
	}
	quint16 word = 0;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		component::Attribute *attr = c->pinDirection(pin);
		word = dio::withBit(word, pin, attr && attr->cachedValue() == QLatin1String(dio::DIRECTION_OPTIONS[0]));
	}
	return word;
}

int DigitalIO_API::outputWord() const
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return -1;
	}
	quint16 word = 0;
	for(int pin = 0; pin < dio::PIN_COUNT; ++pin) {
		component::Attribute *attr = c->pinValue(pin);
		word = dio::withBit(word, pin, attr && attr->cachedValue().toInt() != 0);
	}
	return word;
}

int DigitalIO_API::readInputWord()
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return -1;
	}
	if(!QCoro::waitFor(c->refreshInputs())) {
		return -1;
	}
	return static_cast<int>(c->gpi());
}

int DigitalIO_API::inputWord() const
{
	M2kDigitalIoController *c = controller();
	return c ? static_cast<int>(c->gpi()) : -1;
}

int DigitalIO_API::shortedWord() const
{
	M2kDigitalIoController *c = controller();
	return c ? static_cast<int>(c->shorted()) : -1;
}

bool DigitalIO_API::setRunning(bool on)
{
	if(!m_tool) {
		return false;
	}
	m_tool->setRunningBlocking(on);
	return m_tool->isRunning() == on;
}

bool DigitalIO_API::setPolling(bool on)
{
	M2kDigitalIoController *c = controller();
	if(!c) {
		return false;
	}
	if(on) {
		c->startPolling();
	} else {
		c->stopPolling();
	}
	return c->isPolling() == on;
}

bool DigitalIO_API::isUsable() const
{
	M2kDigitalIoController *c = controller();
	return c && c->isUsable();
}

#include "moc_digitalio_api.cpp"
