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

#include "component/scopedexecutor.h"

#include "core/command.h"

using namespace scopy;
using namespace scopy::component;

ScopedExecutor::ScopedExecutor(ICmdExecutor *inner, QObject *parent)
	: QObject(parent)
	, m_inner(inner)
{}

ScopedExecutor::~ScopedExecutor() = default;

QFuture<void> ScopedExecutor::execute(Command *cmd)
{
	if(!cmd) {
		return {};
	}
	{
		QMutexLocker lock(&m_mutex);
		if(m_closing || m_rejecting || !m_inner) {
			cmd->cancel();
			return QtFuture::makeReadyVoidFuture();
		}
		m_live.insert(cmd);
	}
	// Sync: runs on the worker that finished cmd, before the caller's continuation resumes, so
	// cmd is still alive here.
	// onCanceled: an inner future dropped unfinished must not stall shutdown().
	return m_inner->execute(cmd)
		.then(QtFuture::Launch::Sync, [this, cmd]() { onFinished(cmd); })
		.onCanceled([this, cmd]() { onFinished(cmd); });
}

void ScopedExecutor::onFinished(Command *cmd)
{
	std::shared_ptr<QPromise<void>> drained;
	{
		QMutexLocker lock(&m_mutex);
		m_live.remove(cmd);
		if(m_closing && m_live.isEmpty()) {
			drained = std::move(m_drained);
		}
	}
	if(drained) {
		drained->finish();
	}
}

void ScopedExecutor::cancelById(const QUuid &id)
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_live)) {
		if(cmd->id() == id) {
			cmd->cancel();
			break;
		}
	}
}

void ScopedExecutor::cancelByResource(void *resource)
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_live)) {
		if(cmd->resource() == resource) {
			cmd->cancel();
		}
	}
}

void ScopedExecutor::cancelAll()
{
	QMutexLocker lock(&m_mutex);
	for(Command *cmd : std::as_const(m_live)) {
		cmd->cancel();
	}
}

int ScopedExecutor::pendingCount() const
{
	QMutexLocker lock(&m_mutex);
	return m_live.size();
}

QFuture<void> ScopedExecutor::shutdown()
{
	QMutexLocker lock(&m_mutex);
	if(m_drained) {
		return m_drained->future();
	}
	m_closing = true;
	// Queued ones are skipped by the inner executor; a running one only sees the flag.
	for(Command *cmd : std::as_const(m_live)) {
		cmd->cancel();
	}
	if(m_live.isEmpty()) {
		return QtFuture::makeReadyVoidFuture();
	}
	m_drained = std::make_shared<QPromise<void>>();
	m_drained->start();
	return m_drained->future();
}

bool ScopedExecutor::isClosing() const
{
	QMutexLocker lock(&m_mutex);
	return m_closing;
}

void ScopedExecutor::reject()
{
	QMutexLocker lock(&m_mutex);
	m_rejecting = true;
	for(Command *cmd : std::as_const(m_live)) {
		cmd->cancel();
	}
}

bool ScopedExecutor::isRejecting() const
{
	QMutexLocker lock(&m_mutex);
	return m_rejecting;
}
