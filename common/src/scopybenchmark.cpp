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

#include "scopybenchmark.h"
#include <QDate>
#include <QFile>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_BENCHMARK, "Benchmark")
using namespace scopy;

ScopyBenchmark::ScopyBenchmark() {}

ScopyBenchmark::~ScopyBenchmark() {}

void ScopyBenchmark::startTimer() { m_timer.start(); }

void ScopyBenchmark::restartTimer() { m_timer.restart(); }

void ScopyBenchmark::log(const QString &msg, const char *function, const char *file, int line)
{
	QMessageLogger(file, line, function).info(CAT_BENCHMARK) << function << msg << m_timer.elapsed() << "ms";
}

void ScopyBenchmark::log(const QString &filePath, const QString &msg, const char *function, const char *file, int line)
{
	QFile f(filePath);
	if(f.open(QIODevice::WriteOnly | QIODevice::Append)) {
		QTextStream stream(&f);
		stream << QDateTime::currentDateTime().toString("dd:MM:yyyy hh:mm:ss.zzz") << "\t" << file << ":"
		       << line << "\t" << function << "\t" << msg << "\t" << m_timer.elapsed() << "\t"
		       << "ms"
		       << "\n";
	}
}
