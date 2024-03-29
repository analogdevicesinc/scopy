#include "logdatatofile.hpp"

#include <filemanager.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include <QwtDate>

using namespace scopy;
using namespace datamonitor;

LogDataToFile::LogDataToFile(DataAcquisitionManager *dataAcquisitionManager, QObject *parent)
	: QObject{parent}
{
	m_dataAcquisitionManager = dataAcquisitionManager;
}

void LogDataToFile::logData(QString path)
{
	Q_EMIT startLogData();

	QFile file(path);
	QTextStream exportStream(&file);
	if(!file.isOpen()) {
		file.open(QIODevice::WriteOnly);

		QString tableHead = "Time";
		QMap<QString, QString> values;
		foreach(QString monitor, m_dataAcquisitionManager->getActiveMonitors()) {
			tableHead += ", " + monitor;
			auto xData = m_dataAcquisitionManager->getDataMonitorMap()->value(monitor)->getXdata();

			for(int i = 0; i < xData->length(); i++) {

				auto val =
					m_dataAcquisitionManager->getDataMonitorMap()->value(monitor)->getValueAtTime(
						xData->at(i));
				QDateTime auxTime = QDateTime::fromMSecsSinceEpoch(xData->at(i));
				QString time = QString(auxTime.toString(dateTimeFormat));

				if(values.contains(time)) {
					values[time] += QString(", " + QString::number(val));
				} else {
					values.insert(time, "," + QString::number(val));
				}
			}
		}

		exportStream << tableHead << "\n";

		auto iterator = values.begin();

		while(iterator != values.end()) {

			exportStream << iterator.key() << iterator.value() << "\n";
			iterator++;
		}

	} else {
		qDebug() << "File already opened! ";
	}

	if(file.isOpen())
		file.close();

	Q_EMIT logDataCompleted();
}

void LogDataToFile::loadData(QString path)
{
	Q_EMIT startLoadData();

	QFile file(path);

	if(!file.isOpen()) {
		file.open(QIODevice::ReadOnly);

		QTextStream in(&file);
		auto channels = in.readLine().split(",");
		while(!in.atEnd()) {
			QString line = in.readLine();
			auto values = line.split(",");
			QString time = values[0];
			QDateTime dateTime = QDateTime::fromString(time, dateTimeFormat);
			for(int i = 1; i < channels.length(); i++) {
				// there is an extra space in each name we need to remove it
				QString ch = channels[i].simplified();
				ch.replace(" ", "");
				if(m_dataAcquisitionManager->getDataMonitorMap()->contains(ch)) {
					m_dataAcquisitionManager->getDataMonitorMap()->value(ch)->updateValue(
						QwtDate::toDouble(dateTime), values[i].toDouble());
				}
			}
		}
	} else {
		qDebug() << "File already opened! ";
	}

	if(file.isOpen())
		file.close();

	Q_EMIT loadDataCompleted();
}
