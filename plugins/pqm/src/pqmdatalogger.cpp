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

#include "pqmdatalogger.h"

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QtConcurrentRun>
#include <QLoggingCategory>
#include <QTime>
#include <QDir>

Q_LOGGING_CATEGORY(CAT_PQMLOG, "PQMLog");
using namespace scopy::pqm;

PqmDataLogger::PqmDataLogger(QObject *parent)
	: QObject(parent)
	, m_crtInstr(None)
	, m_writeFw(nullptr)
{
	m_writeFw = new QFutureWatcher<void>(this);
}

PqmDataLogger::~PqmDataLogger()
{
	if(m_writeFw) {
		m_writeFw->waitForFinished();
		m_writeFw->deleteLater();
		m_writeFw = nullptr;
	}
}

void PqmDataLogger::setChnlsName(QStringList chnlsName) { m_chnlsName = chnlsName; }

void PqmDataLogger::acquireBufferData(double val, int chIdx)
{
	if(m_crtInstr != Waveform) {
		return;
	}
	QMutexLocker locker(&m_mutex);
	if(chIdx == 0) {
		m_logQue.enqueue("\n" + QTime::currentTime().toString("hh:mm:ss.zzz") + ",");
	}
	m_logQue.enqueue(QString::number(val) + ",");
}

void PqmDataLogger::acquireHarmonics(QString attrName, QString value, QString chId)
{
	if(attrName.compare("harmonics") != 0) {
		return;
	}
	QMutexLocker locker(&m_mutex);
	m_logQue.enqueue(QTime::currentTime().toString("hh:mm:ss.zzz") + "," + chId + "," + value.split(" ").join(",") +
			 "\n");
}

void PqmDataLogger::acquireAttrData(QString attrName, QString value, QString chId)
{
	if(m_crtInstr == Harmonics) {
		acquireHarmonics(attrName, value, chId);
	}
}

void PqmDataLogger::acquirePqEvents(QString event)
{
	if(m_crtInstr == None) {
		return;
	}
	QMutexLocker locker(&m_mutex);
	m_logQue.enqueue("\n" + QTime::currentTime().toString("hh:mm:ss.zzz") + ", PQEvents \n ," + event + "\n");
}

void PqmDataLogger::log()
{
	if(m_crtInstr == None) {
		return;
	}
	if(!m_writeFw->isRunning()) {
		QFuture<void> f = QtConcurrent::run(this, &PqmDataLogger::writeToFile);
		m_writeFw->setFuture(f);
	}
}

void PqmDataLogger::writeToFile()
{
	QFile f(m_filePath);
	if(f.open(QIODevice::WriteOnly | QIODevice::Append)) {
		QTextStream stream(&f);
		QMutexLocker locker(&m_mutex);
		while(!m_logQue.isEmpty()) {
			stream << m_logQue.dequeue();
		}
	} else {
		qWarning(CAT_PQMLOG) << m_filePath << "cannot be opened!";
	}
}

void PqmDataLogger::logPressed(ActiveInstrument instr, const QString &filePath)
{
	QMutexLocker locker(&m_mutex);
	m_logQue.clear();
	m_crtInstr = instr;
	QDir logDir(filePath);
	switch(m_crtInstr) {
	case Waveform:
		m_filePath = logDir.filePath("waveform_" +
					     QDateTime::currentDateTime().toString("dd-MM-yyyy_hh-mm-ss") + ".csv");
		break;
	case Harmonics:
		m_filePath = logDir.filePath("harmonics_" +
					     QDateTime::currentDateTime().toString("dd-MM-yyyy_hh-mm-ss") + ".csv");
		break;
	case Rms:
		m_filePath =
			logDir.filePath("rms_" + QDateTime::currentDateTime().toString("dd-MM-yyyy_hh-mm-ss") + ".csv");
		break;
	default:
		m_filePath = "";
		qDebug(CAT_PQMLOG) << "The log is not enabled!";
		break;
	}
	createHeader();
}

void PqmDataLogger::createHeader()
{
	if(m_crtInstr == None) {
		return;
	}
	QFile f(m_filePath);
	if(f.open(QIODevice::WriteOnly)) {
		QTextStream stream(&f);
		stream << "Time ,";
		switch(m_crtInstr) {
		case Waveform:
			stream << m_chnlsName.join(",");
			break;
		case Harmonics:
			stream << "Phase ,";
			for(int i = 0; i <= 50; i++) {
				stream << i << ",";
			}
			stream << "\n";
			break;
		default:
			break;
		}
	} else {
		qWarning(CAT_PQMLOG) << m_filePath << "cannot be opened!";
	}
}

#include "moc_pqmdatalogger.cpp"
