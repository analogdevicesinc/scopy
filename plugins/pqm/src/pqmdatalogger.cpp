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
{
	m_writeFw = new QFutureWatcher<void>(this);
}

PqmDataLogger::~PqmDataLogger() {}

void PqmDataLogger::setChnlsName(QVector<QString> chnlsName) { m_chnlsName = chnlsName.toList(); }

void PqmDataLogger::acquireBufferData(double val, int chIdx)
{
	if(m_crtInstr != Waveform) {
		return;
	}
	QMutexLocker locker(&m_mutex);
	if(chIdx == 0) {
		m_logQue.enqueue("\n" + QTime::currentTime().toString("hh:mm:ss.zzz") + "\t");
	}
	m_logQue.enqueue(QString::number(val) + "\t");
}

void PqmDataLogger::acquireHarmonics(QString value, QString chId)
{
	if(m_crtInstr != Harmonics) {
		return;
	}
	QMutexLocker locker(&m_mutex);
	m_logQue.enqueue(QTime::currentTime().toString("hh:mm:ss.zzz") + "\t" + chId + "\t" +
			 value.split(" ").join("\t") + "\n");
}

void PqmDataLogger::acquireAttrData(QString attrName, QString value, QString chId)
{
	if(attrName.compare("harmonics") == 0) {
		acquireHarmonics(value, chId);
	}
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
					     QDateTime::currentDateTime().toString("dd.MM.yyyy_hh:mm:ss") + ".csv");
		break;
	case Harmonics:
		m_filePath = logDir.filePath("harmonics_" +
					     QDateTime::currentDateTime().toString("dd.MM.yyyy_hh:mm:ss") + ".csv");
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
		stream << "Time \t";
		switch(m_crtInstr) {
		case Waveform:
			stream << m_chnlsName.join("\t");
			break;
		case Harmonics:
			stream << "Phase \t";
			for(int i = 0; i <= 50; i++) {
				stream << i << "\t";
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
