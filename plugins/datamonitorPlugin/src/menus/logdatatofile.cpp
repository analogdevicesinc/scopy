#include "logdatatofile.hpp"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QRandomGenerator>
#include <QTextStream>
#include <QwtDate>
#include <datamonitorutils.hpp>
#include <filemanager.h>
#include <readabledatamonitormodel.hpp>
#include <stylehelper.h>

#include <pluginbase/preferences.h>
#include <pluginbase/statusbarmanager.h>

using namespace scopy;
using namespace datamonitor;

LogDataToFile::LogDataToFile(DataAcquisitionManager *dataAcquisitionManager, QObject *parent)
	: QObject{parent}
{
	m_dataAcquisitionManager = dataAcquisitionManager;
	currentFileHeader = new QString("");
}

void LogDataToFile::continuousLogData(QString path)
{
	if(!m_dataAcquisitionManager->getActiveMonitors().isEmpty()) {
		QString dateTimeFormat = DataMonitorUtils::getLoggingDateTimeFormat();

		QString fileHeader = "Time";
		foreach(QString monitor, m_dataAcquisitionManager->getActiveMonitors()) {
			if(!monitor.contains("Import:")) {
				fileHeader += "," + monitor;
			}
		}

		// if a channels is added or removed we need to recreate the file using logData()
		if(currentFileHeader != fileHeader) {
			logData(path);

			currentFileHeader = new QString(fileHeader);
		} else {
			QFile file(path);
			QTextStream exportStream(&file);
			if(!file.isOpen()) {
				if(!file.open(QIODevice::Append)) {
					StatusBarManager::pushMessage("Can't open file!", 3000);
					Q_EMIT logDataError();
				} else {
					auto monitors = currentFileHeader->split(",");

					// get last time value
					// the time of the last read value will be the same for all active monitors
					auto lastReadValueTime = m_dataAcquisitionManager->getDataMonitorMap()
									 ->value(monitors[1])
									 ->getXdata()
									 ->last();

					QDateTime auxTime = QDateTime::fromMSecsSinceEpoch(lastReadValueTime);
					QString time = QString(auxTime.toString(dateTimeFormat));

					QString values = "";

					for(int i = 1; i < monitors.length(); i++) {
						QString monitor = monitors[i];

						auto auxVal = m_dataAcquisitionManager->getDataMonitorMap()
								      ->value(monitor)
								      ->getValueAtTime(lastReadValueTime);

						QString val = ", ";
						if(lastReadValueTime != -Q_INFINITY) {
							val += QString::number(auxVal);
						}

						values += val;
					}

					exportStream << time << values << "\n";
				}
			} else {
				Q_EMIT logDataError();
				qDebug() << "File already opened! ";
			}

			if(file.isOpen())
				file.close();
		}
	}
}

void LogDataToFile::logData(QString path)
{
	Q_EMIT startLogData();

	QString dateTimeFormat = DataMonitorUtils::getLoggingDateTimeFormat();

	QFile file(path);
	QTextStream exportStream(&file);
	if(!file.isOpen()) {
		if(!file.open(QIODevice::WriteOnly)) {
			StatusBarManager::pushMessage("Can't open file!", 3000);
			Q_EMIT logDataError();
		} else {
			QString tableHead = "Time";
			QMap<QString, QString> values;

			// cover all the time values recorded
			// use a QSet to avoid having duplicate time values
			QSet<double> timeValues;
			foreach(QString monitor, m_dataAcquisitionManager->getActiveMonitors()) {
				if(qobject_cast<ReadableDataMonitorModel *>(
					   m_dataAcquisitionManager->getDataMonitorMap()->value(monitor))) {
					tableHead += ", " + monitor;
					auto xData = m_dataAcquisitionManager->getDataMonitorMap()
							     ->value(monitor)
							     ->getXdata();
					for(int i = 0; i < xData->length(); i++) {
						timeValues.insert(xData->at(i));
					}
				}
			}

			QSet<double>::const_iterator i;
			for(i = timeValues.begin(); i != timeValues.end(); ++i) {
				QDateTime auxTime = QDateTime::fromMSecsSinceEpoch(*i);
				QString time = QString(auxTime.toString(dateTimeFormat));

				foreach(QString monitor, m_dataAcquisitionManager->getActiveMonitors()) {
					if(qobject_cast<ReadableDataMonitorModel *>(
						   m_dataAcquisitionManager->getDataMonitorMap()->value(monitor))) {
						auto auxVal = m_dataAcquisitionManager->getDataMonitorMap()
								      ->value(monitor)
								      ->getValueAtTime(*i);

						// verify if monitor has a value this time value if no value is found
						// leave an empty space in the file
						QString val = ", ";
						if(auxVal != -Q_INFINITY) {
							val += QString::number(auxVal);
						}

						if(values.contains(time)) {
							values[time] += val;
						} else {
							values.insert(time, val);
						}
					}
				}
			}

			// write the data to file
			exportStream << tableHead << "\n";
			auto iterator = values.begin();
			while(iterator != values.end()) {
				exportStream << iterator.key() << iterator.value() << "\n";
				iterator++;
			}
			Q_EMIT logDataCompleted();
		}
	} else {
		Q_EMIT logDataError();
		qDebug() << "File already opened! ";
	}

	if(file.isOpen())
		file.close();
}

void LogDataToFile::loadData(QString path)
{
	Q_EMIT startLoadData();

	QString dateTimeFormat = DataMonitorUtils::getLoggingDateTimeFormat();

	QFile file(path);

	if(!file.isOpen()) {
		file.open(QIODevice::ReadOnly);

		QTextStream in(&file);
		// first line of the file contains the value "Time" and all monitors stored each monitor represents a
		// column
		auto channels = in.readLine().split(",");

		QVector<double> timeVector;
		QMap<QString, QVector<double> *> valueMap;

		while(!in.atEnd()) {
			QString line = in.readLine();
			QStringList values = line.split(",");
			// first column is always the time value
			QString time = values[0];
			QDateTime dateTime = QDateTime::fromString(time, dateTimeFormat);

			timeVector.push_back(QwtDate::toDouble(dateTime));
			//
			for(int i = 1; i < channels.length(); i++) {
				QString ch = channels[i].simplified();
				// make sure there is no extra spaces in the name
				ch.replace(" ", "");
				// we store data to coresponding monitor
				if(values[i] != " ") {
					if(!valueMap.contains(ch)) {
						valueMap.insert(ch, new QVector<double>);
					}
					valueMap.value(ch)->push_back(values[i].toDouble());
				}
			}
		}

		QMap<QString, QVector<double> *>::iterator it;

		// we don't override data we create dummy monitors for all channels from file
		for(it = valueMap.begin(); it != valueMap.end(); ++it) {
			QString fileTitle("Import: " + file.fileName().splitRef("/").last());
			QString monitorName(fileTitle + ":" + it.key());
			if(m_dataAcquisitionManager->getDataMonitorMap()->contains(monitorName)) {

				m_dataAcquisitionManager->getDataMonitorMap()->value(monitorName)->setXdata(timeVector);
				m_dataAcquisitionManager->getDataMonitorMap()
					->value(monitorName)
					->setYdata(*it.value());

			} else {
				DataMonitorModel *channelModel = new DataMonitorModel(
					monitorName,
					StyleHelper::getColor(
						"CH" + QString::number(QRandomGenerator::global()->bounded(0, 7))));

				channelModel->setShortName(it.key());
				channelModel->setDeviceName(fileTitle);
				channelModel->setXdata(timeVector);
				channelModel->setYdata(*it.value());
				m_dataAcquisitionManager->addMonitor(channelModel);
			}
		}

	} else {
		qDebug() << "File already opened! ";
	}

	if(file.isOpen())
		file.close();

	Q_EMIT loadDataCompleted();
}
