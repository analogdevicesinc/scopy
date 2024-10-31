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

#ifndef PQMDATALOGGER_H
#define PQMDATALOGGER_H

#include <QMap>
#include <QObject>
#include <QQueue>
#include <QFutureWatcher>

namespace scopy::pqm {
class PqmDataLogger : public QObject
{
	Q_OBJECT
public:
	enum ActiveInstrument
	{
		None,
		Waveform,
		Harmonics
	};
	PqmDataLogger(QObject *parent = nullptr);
	~PqmDataLogger();

	void setChnlsName(QVector<QString> chnlsName);
	void acquireBufferData(double val, int chIdx);
	void acquireAttrData(QString attrName, QString value, QString chId);
	void log();
	void writeToFile();
public Q_SLOTS:
	void logPressed(ActiveInstrument instr, const QString &filePath = "");

private:
	void acquireHarmonics(QString value, QString chId);
	void createHeader();

	ActiveInstrument m_crtInstr;
	QString m_filePath;
	QStringList m_chnlsName;

	QQueue<QString> m_logQue;
	QFutureWatcher<void> *m_writeFw;
	QMutex m_mutex;
};

} // namespace scopy::pqm

#endif // PQMDATALOGGER_H
