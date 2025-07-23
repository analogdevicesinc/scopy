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

#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QMap>
#include <QObject>
#include <plotmanager/datareader.h>
#include <outputconfig.h>
#include <common/scopyconfig.h>
#include <QDir>
#include <outputinfo.h>

namespace scopy::extprocplugin {

class DataManagerMap : public QObject
{
	Q_OBJECT
public:
	// Insert a key-value pair
	void insert(const QString &key, const QVector<double> &value)
	{
		bool keyExist = map.contains(key);
		map.insert(key, value);
		if(!keyExist) {
			Q_EMIT keysChanged(map.keys());
		}
	}

	// Get a value (returns default if not found)
	QVector<double> get(const QString &key, const QVector<double> &defaultValue = QVector<double>()) const
	{
		return map.value(key, defaultValue);
	}

	// Check if a key exists
	bool contains(const QString &key) const { return map.contains(key); }

	// Remove a key
	bool remove(const QString &key) { return map.remove(key) > 0; }

Q_SIGNALS:
	void keysChanged(const QList<QString> &keys);

private:
	// Implement a frequency list to track active entries.
	// Entries that are not updated for a certain period should be removed from DataManager.
	QMap<QString, QVector<double>> map;
};

class DataManager : public QObject
{
	Q_OBJECT
public:
	DataManager(QObject *parent = nullptr);
	~DataManager();

	void config(const QStringList &chnlsName, const QStringList &chnlsFormat, const int channelCount);
	void onConfigAnalysis(const QString &type, const QVariantMap &config, const OutputInfo &info);
	void readData(int64_t startSample, int64_t sampleCount);

	void setSamplingFreq(int samplingFreq);
	QVector<double> dataForKey(const QString &key);

	double sampleCount() const;

	double samplingFreq() const;

Q_SIGNALS:
	void dataIsReady();
	void newDataEntries(const QList<QString> &entries);

public Q_SLOTS:
	void onInputData(QVector<QVector<double>> bufferData);

private Q_SLOTS:
	void onDataReady(QMap<QString, QVector<double>> &data);

private:
	void setupConnections();
	void computeXTime(int samplingFreq, int samples);

	double m_samplingFreq;
	double m_sampleCount;
	DataReader *m_dataReader;
	DataManagerMap m_plotsData;
};

} // namespace scopy::extprocplugin

#endif // DATAMANAGER_H
