#include "logdatatofile.hpp"

#include <filemanager.h>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>

using namespace scopy;
using namespace datamonitor;

LogDataToFile::LogDataToFile(DataAcquisitionManager *dataAcquisitionManager, QObject *parent)
	: QObject{parent}
{
	m_dataAcquisitionManager = dataAcquisitionManager;
}

void LogDataToFile::logData(QString path)
{
	// TODO HOW SHOULD WE TREAT WRITE ?
	QFile file(path);
	QTextStream exportStream(&file);
	if(!file.isOpen()) {
		file.open(QIODevice::Append);

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
				QString time = QString(auxTime.toString("hh:mm:ss"));

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
}
