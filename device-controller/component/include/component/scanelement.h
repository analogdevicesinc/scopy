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

#include <QObject>
#include <QString>

#include <qcoro/qcorotask.h>
#include "component/component_export.h"

namespace scopy::component {

// Membership of one channel in its stream's sample mask (abstract base). Child of an
// InputStream / OutputStream, one per streamable channel. The stream's StreamFormat::channels
// lists the enabled elements in child order, so entry i is the i-th enabled ScanElement.
// `id` matches the id of the Channel component of the same direction on the parent Device.
class COMPONENT_EXPORT ScanElement : public QObject
{
	Q_OBJECT
	Q_PROPERTY(long index READ index CONSTANT)
	Q_PROPERTY(QString id READ id CONSTANT)
	Q_PROPERTY(bool isOutput READ isOutput CONSTANT)
	Q_PROPERTY(bool isEnabled READ isEnabled NOTIFY enabledChanged)
public:
	explicit ScanElement(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~ScanElement() override = default;

	// The index StreamConfig::enabledChannels refers to.
	long index() const { return m_index; }
	QString id() const { return m_id; }
	bool isOutput() const { return m_isOutput; }
	void setIndex(long index)
	{
		m_index = index;
		setObjectName("Scan element " + QString::number(m_index));
	}
	void setId(const QString &id) { m_id = id; }
	void setIsOutput(bool isOutput) { m_isOutput = isOutput; }

	virtual bool isEnabled() const = 0;
	Q_INVOKABLE virtual QCoro::Task<CommandResponse<void>> enableAsync(bool en) = 0;

Q_SIGNALS:
	void enabledChanged(bool en);
	void enableFailed(const scopy::Error &error);

protected:
	long m_index = -1;
	QString m_id;
	bool m_isOutput = false;
};

} // namespace scopy::component
