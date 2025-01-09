/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#include "debugtimer.h"
#include <QFile>
#include <QDate>
#include <QFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_BENCHMARK, "Benchmark")
using namespace scopy;

DebugTimer::DebugTimer(QString filePath)
{

	m_filePath = filePath;
	f.setFileName(m_filePath);
	if(!m_filePath.isEmpty() && !f.open(QIODevice::WriteOnly | QIODevice::Append)) {
		qWarning(CAT_BENCHMARK) << "file " + m_filePath + "cannot be opened";
		m_filePath.clear();
	}

	m_timer.start();
}

DebugTimer::~DebugTimer() { f.close(); }

void DebugTimer::startTimer() { m_timer.start(); }

void DebugTimer::restartTimer() { m_timer.restart(); }

void DebugTimer::setSingleMode(bool b) { m_singleMode = b; }

bool DebugTimer::singleMode() { return m_singleMode; }

void DebugTimer::log(const QString &msg, const char *function, const char *file, int line)
{
	if(f.isOpen()) {
		QTextStream stream(&f);

		stream << QDateTime::currentDateTime().toString("yyyy:MM:dd hh:mm:ss.zzz") << "\t" << file << ":"
		       << line << "\t" << function << "\t" << msg << "\t" << m_timer.elapsed() << "\t"
		       << "ms"
		       << "\n";
	} else {
		QMessageLogger(file, line, function).info(CAT_BENCHMARK)
			<< function << msg << m_timer.elapsed() << "ms";
	}

	if(m_singleMode) {
		m_timer.restart();
	}
}
