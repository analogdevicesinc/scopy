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

#include "m2kvoltmetercsvlogger.h"

#include <QDateTime>
#include <QLoggingCategory>

#include <pluginbase/statusbarmanager.h>

Q_LOGGING_CATEGORY(CAT_M2KVOLTMETER_CSVLOGGER, "M2kVoltmeterCsvLogger")

using namespace scopy::adalm2000;

M2kVoltmeterCsvLogger::M2kVoltmeterCsvLogger(QObject *parent)
	: QObject(parent)
{}

M2kVoltmeterCsvLogger::~M2kVoltmeterCsvLogger() { close(); }

bool M2kVoltmeterCsvLogger::open(const QString &path)
{
	close();
	m_file.setFileName(path);
	if(!m_file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		scopy::StatusBarManager::pushUrgentMessage(
			QStringLiteral("Voltmeter: could not open log file: %1").arg(m_file.errorString()));
		return false;
	}
	m_stream.setDevice(&m_file);
	m_stream << "Time,CH1 (V),CH2 (V)\n";
	m_stream.flush();
	return true;
}

void M2kVoltmeterCsvLogger::append(double ch0, double ch1)
{
	if(!m_file.isOpen()) {
		return;
	}
	m_stream << QDateTime::currentDateTime().toString("dd/MM/yyyy hh:mm:ss.zzz") << ','
		 << QString::number(ch0, 'g', 6) << ',' << QString::number(ch1, 'g', 6) << '\n';
	m_stream.flush();
}

void M2kVoltmeterCsvLogger::close()
{
	if(m_file.isOpen()) {
		m_stream.flush();
		m_file.close();
	}
}

bool M2kVoltmeterCsvLogger::isOpen() const { return m_file.isOpen(); }

#include "moc_m2kvoltmetercsvlogger.cpp"
