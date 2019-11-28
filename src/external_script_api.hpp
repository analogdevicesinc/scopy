/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef SCOPY_EXT_SCRIPT_HPP
#define SCOPY_EXT_SCRIPT_HPP

#include <QObject>
#include <QProcess>

class QJSEngine;

namespace adiscope {

class ExternalScript_API : public QObject
{
	Q_OBJECT

public:
	explicit ExternalScript_API(QObject *parent = 0);
	~ExternalScript_API() {}
	Q_INVOKABLE QString start(const QString& cmd);
	Q_INVOKABLE void setWorkingDir(const QString& root);
	Q_INVOKABLE QString getWorkingDir();
	Q_INVOKABLE void setProcessTimeout(int);
	Q_INVOKABLE int getProcessTimeout();


private:
	QString run(const QString&, bool, bool);
	QProcess *m_external_process;
	QString m_working_dir;
	int m_process_timeout;
};

}

#endif /* SCOPY_EXT_SCRIPT_HPP */

