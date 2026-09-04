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

#include "core/result.h"
#include "core/controller_core_export.h"
#include <QObject>
#include <QUuid>

namespace scopy {

class CONTROLLER_CORE_EXPORT Command : public QObject
{
	Q_OBJECT
public:
	Command(void *resource, QObject *parent = nullptr)
		: QObject(parent)
		, m_id(QUuid::createUuid())
		, m_resource(resource)
	{}
	virtual ~Command() = default;

	virtual void execute()
	{
		Q_EMIT started(this);
		if(!m_cancelled) {
			run();
		}
		Q_EMIT finished(this);
	}

	virtual QString toString() const
	{
		return QString::fromLatin1(metaObject()->className()) + QStringLiteral("(id=") +
			m_id.toString(QUuid::WithoutBraces) + QLatin1Char(')');
	}

	QUuid id() const { return m_id; }
	void *resource() const { return m_resource; }
	void cancel() { m_cancelled = true; }
	bool isCancelled() const { return m_cancelled; }

Q_SIGNALS:
	void started(scopy::Command *cmd);
	void finished(scopy::Command *cmd);

protected:
	virtual void run() = 0;

	QUuid m_id;
	void *m_resource;
	bool m_cancelled = false;
};

} // namespace scopy
