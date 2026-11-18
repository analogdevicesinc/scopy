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

#pragma once

#include "core/icmdexecutor.h"

#include <QFuture>
#include <QMutex>
#include <QObject>
#include <QPromise>
#include <QSet>
#include <memory>

#include "component/component_export.h"

namespace scopy::component {

// One per view. Forwards commands to the connection's executor and tracks the ones it submitted,
// so a view can be drained without touching the other views of the connection. Parallelism is
// still the inner executor's.
class COMPONENT_EXPORT ScopedExecutor : public QObject, public ICmdExecutor
{
	Q_OBJECT
public:
	explicit ScopedExecutor(ICmdExecutor *inner, QObject *parent = nullptr);
	~ScopedExecutor() override;

	QFuture<void> execute(Command *cmd) override;
	// The cancel* calls only reach this view's commands.
	void cancelById(const QUuid &id) override;
	void cancelByResource(void *resource) override;
	void cancelAll() override;
	int pendingCount() const override;

	// Rejects new commands, cancels the queued ones and resolves once the running ones return.
	// Idempotent.
	QFuture<void> shutdown();
	bool isClosing() const;


private:
	void onFinished(Command *cmd);

	ICmdExecutor *m_inner;
	mutable QMutex m_mutex;
	QSet<Command *> m_live; // queued + running
	bool m_closing = false;
	std::shared_ptr<QPromise<void>> m_drained;
};

} // namespace scopy::component
