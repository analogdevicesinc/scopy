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

#pragma once

#include "scopy-adalm2000_export.h"

#include <QList>
#include <QString>

#include <pluginbase/apiobject.h>

namespace scopy::adalm2000 {
class Adalm2000DigitalIoTool;

/**
 * @brief The JS global `dio`: scriptable access to the Digital I/O pins.
 */
class SCOPY_ADALM2000_EXPORT DigitalIO_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(QList<bool> group READ grouped WRITE setGrouped)
	Q_PROPERTY(QList<bool> dir READ direction WRITE setDirection)
	// The buffered output level, true == high.
	Q_PROPERTY(QList<bool> out READ output WRITE setOutput)
	// The live input word from the last poll.
	Q_PROPERTY(QList<bool> gpi READ gpi STORED false)
	Q_PROPERTY(QList<bool> locked READ locked STORED false)
	Q_PROPERTY(bool running READ running WRITE run STORED false)

public:
	explicit DigitalIO_API(Adalm2000DigitalIoTool *tool);
	~DigitalIO_API() override;

	QList<bool> grouped() const;
	void setGrouped(const QList<bool> &grouped);

	QList<bool> direction() const;
	void setDirection(const QList<bool> &list);

	QList<bool> output() const;
	void setOutput(const QList<bool> &list);

	QList<bool> gpi() const;
	QList<bool> locked() const;

	bool running() const;
	void run(bool en);

private:
	Adalm2000DigitalIoTool *m_tool;
};

} // namespace scopy::adalm2000
