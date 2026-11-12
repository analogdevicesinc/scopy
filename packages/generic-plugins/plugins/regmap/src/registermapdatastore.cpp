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

#include "registermapdatastore.hpp"

using namespace scopy;
using namespace scopy::component;
using namespace scopy::regmap;

namespace {
// a range can name a register twice; the signals name it once
QList<uint32_t> unique(const QVector<uint32_t> &addresses)
{
	QList<uint32_t> out;
	QSet<uint32_t> seen;
	for(uint32_t address : addresses) {
		if(!seen.contains(address)) {
			seen.insert(address);
			out.append(address);
		}
	}
	return out;
}
} // namespace

RegisterMapDataStore::RegisterMapDataStore(QObject *parent)
	: QObject(parent)
{}

const RegisterData *RegisterMapDataStore::get(uint32_t address) const
{
	auto it = m_data.constFind(address);
	return it == m_data.constEnd() ? nullptr : &it.value();
}

QList<uint32_t> RegisterMapDataStore::addresses() const { return m_data.keys(); }

bool RegisterMapDataStore::isPending(uint32_t address) const { return m_before.contains(address); }

bool RegisterMapDataStore::isAnyPending(const QVector<uint32_t> &addresses) const
{
	for(uint32_t address : addresses) {
		if(m_before.contains(address)) {
			return true;
		}
	}
	return false;
}

bool RegisterMapDataStore::setPending(const QVector<uint32_t> &addresses)
{
	if(isAnyPending(addresses)) {
		return false;
	}
	const QList<uint32_t> pending = unique(addresses);
	for(uint32_t address : pending) {
		auto it = m_data.find(address);
		if(it == m_data.end()) {
			m_before.insert(address, std::nullopt);
			m_data.insert(address, RegisterData{address});
		} else {
			m_before.insert(address, it.value());
			it.value().status = RegisterAccessStatus::Pending;
		}
	}
	if(!pending.isEmpty()) {
		Q_EMIT pendingChanged(pending, true);
	}
	return true;
}

void RegisterMapDataStore::store(const QVector<RegisterData> &results)
{
	QVector<uint32_t> stored;
	for(const RegisterData &result : results) {
		if(result.status == RegisterAccessStatus::Pending) {
			continue;
		}
		// in place: the node keeps its address in memory
		m_data[result.address] = result;
		stored.append(result.address);
	}
	if(stored.isEmpty()) {
		return;
	}
	const QList<uint32_t> changed = unique(stored);
	QList<uint32_t> ended;
	for(uint32_t address : changed) {
		if(m_before.remove(address)) {
			ended.append(address);
		}
	}
	// the new value first, then the buttons
	Q_EMIT dataChanged(changed);
	if(!ended.isEmpty()) {
		Q_EMIT pendingChanged(ended, false);
	}
}

void RegisterMapDataStore::restore(const QVector<uint32_t> &addresses)
{
	QList<uint32_t> ended;
	for(uint32_t address : unique(addresses)) {
		auto before = m_before.find(address);
		if(before == m_before.end()) {
			continue;
		}
		if(before.value()) {
			m_data[address] = *before.value();
		} else {
			m_data.remove(address);
		}
		m_before.erase(before);
		ended.append(address);
	}
	if(!ended.isEmpty()) {
		Q_EMIT pendingChanged(ended, false);
	}
}

void RegisterMapDataStore::clear()
{
	m_data.clear();
	m_before.clear();
}
