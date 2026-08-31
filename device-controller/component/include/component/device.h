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

#include <QObject>
#include <QString>

namespace scopy::component {

// Identity component: a device within a context. Plain QObject — its attributes
// and channels are composed under it via Qt parent-child. Backend-agnostic:
// holds only shared identity (id/name/label). Concrete backends subclass this
// (IIODevice, ...) to carry backend handles; the base holds no backend types.
// Shared identity is exposed through Q_PROPERTY for uniform client binding.
class Device : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QString id READ id CONSTANT)
	Q_PROPERTY(QString name READ name CONSTANT)
	Q_PROPERTY(QString label READ label CONSTANT)
public:
	explicit Device(QObject *parent = nullptr)
		: QObject(parent)
	{}
	~Device() override = default;

	QString id() const { return m_id; }
	QString name() const { return m_name; }
	QString label() const { return m_label; }

	void setId(const QString &id) { m_id = id; }
	void setName(const QString &name)
	{
		m_name = name;
		setObjectName(name);
	}
	void setLabel(const QString &label) { m_label = label; }

protected:
	QString m_id;
	QString m_name;
	QString m_label;
};

} // namespace scopy::component
