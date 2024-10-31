/*
 * Copyright (c) 2024 Analog Devices Inc.
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

#ifndef COMMAND_H
#define COMMAND_H

#include "scopy-iioutil_export.h"

#include <QDebug>
#include <QObject>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT CommandResult
{
public:
	ssize_t errorCode;
	void *results = nullptr;
};

class SCOPY_IIOUTIL_EXPORT Command : public QObject
{
	Q_OBJECT
public:
	virtual ~Command()
	{
		qDebug() << "Command deleted";
		delete m_cmdResult;
		m_cmdResult = nullptr;
	};
	virtual void execute() = 0;
	virtual ssize_t getReturnCode() { return m_cmdResult->errorCode; }
	virtual bool isOverwrite() { return m_overwrite; }
Q_SIGNALS:
	void started(scopy::Command *command = nullptr);
	void finished(scopy::Command *command = nullptr);

protected:
	void setOverwrite(bool overwrite) { m_overwrite = overwrite; }

protected:
	CommandResult *m_cmdResult = nullptr;
	bool m_overwrite = false;
};
} // namespace scopy

Q_DECLARE_METATYPE(scopy::Command *)
#endif // COMMAND_H
