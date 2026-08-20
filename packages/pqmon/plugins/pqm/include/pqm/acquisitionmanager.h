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

#ifndef ACQUISITIONMANAGER_H
#define ACQUISITIONMANAGER_H

#include <QObject>
#include <QMap>
#include <QFutureWatcher>

#include <iioutil/commandqueue.h>
#include <iioutil/pingtask.h>

#include <pqmdatalogger.h>

#include <component/context.h>
#include <component/controller.h>
#include <component/device.h>

#include <component/backends/iio/iioinputstream.h>

#define MAX_ATTR_SIZE 1024
#define BUFFER_SIZE 256
#define DEVICE_PQM "pqm"
namespace scopy::pqm {
class AcquisitionManager : public QObject
{
	Q_OBJECT
public:
	AcquisitionManager(component::ContextHandle ctx, QObject *parent = nullptr);
	~AcquisitionManager();

	bool hasFwVers() const;
	QString getLogFilePath() const;

public Q_SLOTS:
	void toolEnabled(bool en, QString toolName);
	void setConfigAttr(QMap<QString, QMap<QString, QString>>);
	void startPing();
	void stopPing();
Q_SIGNALS:
	void pqmAttrsAvailable(QMap<QString, QMap<QString, QString>>);
	void bufferDataAvailable(QMap<QString, QVector<double>>);
	void logData(PqmDataLogger::ActiveInstrument instr, const QString &filePath);
	void pqEvent();

private Q_SLOTS:
	void futureReadData();
	void onReadFinished();

private:
	double convertFromHwToHost(int value, QString chnlId);
	QList<int> enableBufferChnls(component::Device *dev);
	void readData();
	void readAttrData();
	void readBuffData();
	bool readPqmAttributes();
	bool readBufferedData();
	void setData(QMap<QString, QMap<QString, QString>>);
	void setProcessData(bool val);
	void storeProcessData();
	void handlePQEvents();
	void adjustMap(const QString &attr, std::function<void(QString &)> adjuster);
	static void computeAdjustedAngle(QString &angle);

	component::ContextHandle m_ctx;
	component::iio::IIOInputStream *m_inputStream = nullptr;
	PqmDataLogger *m_pqmLog;

	QFutureWatcher<void> *m_readFw;
	QFutureWatcher<void> *m_setFw;

	QMutex m_mutex;
	QStringList m_buffChnls;
	QStringList m_eventsChnls;
	QMap<QString, QMap<QString, QString>> m_pqmAttr;
	QMap<QString, QVector<double>> m_bufferData;
	QMap<QString, bool> m_tools = {{"rms", false}, {"harmonics", false}, {"waveform", false}, {"settings", false}};

	std::atomic<bool> m_processData = false;
	bool m_attrHaveBeenRead = false;
	bool m_buffHaveBeenRead = false;
	bool m_hasFwVers = false;
	bool m_concurrentAcq = false;
	bool m_alternateExecution = false;
	const int THREAD_FINISH_TIMEOUT = 10000;
};
} // namespace scopy::pqm

#endif // ACQUISITIONMANAGER_H
