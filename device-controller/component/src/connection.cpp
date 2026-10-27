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

#include "component/connection.h"

#include "component/context.h"

using namespace scopy::component;

Connection::Connection(QString uri, ConnectionKind kind, QObject *parent)
	: QObject(parent)
	, m_uri(std::move(uri))
	, m_kind(std::move(kind))
{}

Connection::~Connection() = default;

QList<Context *> Connection::contexts() const
{
	QList<Context *> list;
	for(const auto &e : m_contexts) {
		list.append(e.second);
	}
	return list;
}

QList<BackendId> Connection::builtBackends() const
{
	QList<BackendId> list;
	for(const auto &e : m_contexts) {
		list.append(e.first);
	}
	return list;
}

Context *Connection::context(const BackendId &id) const
{
	for(const auto &e : m_contexts) {
		if(e.first == id) {
			return e.second;
		}
	}
	return nullptr;
}

BackendId Connection::backendOf(const Context *ctx) const
{
	for(const auto &e : m_contexts) {
		if(e.second == ctx) {
			return e.first;
		}
	}
	return {};
}

void Connection::add(const BackendId &id, Context *ctx)
{
	ctx->setParent(this);
	ctx->setBackend(id);
	ctx->setExecutor(m_executor);
	m_contexts.append({id, ctx});
	Q_EMIT contextCreated(ctx);
}

Context *Connection::take(const BackendId &id)
{
	for(int i = 0; i < m_contexts.size(); ++i) {
		if(m_contexts[i].first == id) {
			Context *ctx = m_contexts[i].second;
			m_contexts.removeAt(i);
			Q_EMIT contextDestroyed(ctx);
			ctx->setParent(nullptr);
			return ctx;
		}
	}
	return nullptr;
}
