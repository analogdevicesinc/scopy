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
		Harmonics,
		Rms
	};
	PqmDataLogger(QObject *parent = nullptr);
	~PqmDataLogger();

	void setChnlsName(QStringList chnlsName);
	void acquireBufferData(double val, int chIdx);
	void acquireAttrData(QMap<QString, QMap<QString, QString>> pqmAttr);
	void acquirePqEvents(QString event);
	void log();
	void writeToFile();
public Q_SLOTS:
	void logPressed(ActiveInstrument instr, const QString &filePath = "");

private:
	void acquireHarmonics(QMap<QString, QMap<QString, QString>> pqmAttr);
	void acquireRmsChnlAttr(QMap<QString, QMap<QString, QString>> pqmAttr);
	void acquireRmsDeviceAttr(QMap<QString, QMap<QString, QString>> pqmAttr);
	void createHeader();

	ActiveInstrument m_crtInstr;
	QString m_filePath;
	QStringList m_chnlsName;

	QQueue<QString> m_logQue;
	QFutureWatcher<void> *m_writeFw;
	QMutex m_mutex;
	const QString ATTR_HARMONICS = "harmonics";
	const QString PQM_DEVICE = "pqm";
	const QStringList m_rmsHeader{"rms", "angle", "deviation_under", "deviation_over", "pinst", "pst", "plt"};
	const QMap<QString, QStringList> m_rmsDeviceAttr{
		{"voltage", {"u2", "u0", "sneg_voltage", "spos_voltage", "szro_voltage"}},
		{"current", {"i2", "i0", "sneg_current", "spos_current", "szro_current"}}};
};

} // namespace scopy::pqm

#endif // PQMDATALOGGER_H
