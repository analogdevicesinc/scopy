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

namespace scopy::extprocplugin {

class DataManagerMap : public QObject
{
	Q_OBJECT
public:
	// Insert a key-value pair
	void insert(const QString &key, const QVector<float> &value)
	{
		bool keyExist = map.contains(key);
		map.insert(key, value);
		if(!keyExist) {
			Q_EMIT keysChanged(map.keys());
		}
	}

	// Get a value (returns default if not found)
	QVector<float> get(const QString &key, const QVector<float> &defaultValue = QVector<float>()) const
	{
		return map.value(key, defaultValue);
	}

	// Check if a key exists
	bool contains(const QString &key) const { return map.contains(key); }

	// Remove a key
	bool remove(const QString &key) { return map.remove(key) > 0; }

	// Clear all
	void clear() { return map.clear(); }

	// Get available keys
	QStringList availableKeys() { return map.keys(); }

Q_SIGNALS:
	void keysChanged(const QList<QString> &keys);

private:
	// Implement a frequency list to track active entries.
	// Entries that are not updated for a certain period should be removed from DataManager.
	QMap<QString, QVector<float>> map;
};

class DataManager : public QObject
{
	Q_OBJECT
protected:
	explicit DataManager(QObject *parent = nullptr);
	~DataManager();

public:
	DataManager(DataManager &other) = delete;
	void operator=(const DataManager &) = delete;
	static DataManager *GetInstance();

	void registerData(const QString &key, const QVector<float> &data);
	void registerData(const QMap<QString, QVector<float>> &data);

	void setSamplingFreq(int samplingFreq);
	void setSampleCount(int sampleCount);
	QVector<float> dataForKey(const QString &key);

	double sampleCount() const;
	double samplingFreq() const;

	void clearData();
	bool hasDataForKey(const QString &key) const;
	QStringList availableKeys();

Q_SIGNALS:
	void dataIsReady(const QStringList &keys);
	void newDataEntries(const QList<QString> &entries);

private:
	void setupConnections();

	static DataManager *pinstance_;

	double m_samplingFreq;
	double m_sampleCount;
	DataManagerMap m_plotsData;
};

} // namespace scopy::extprocplugin

#endif // DATAMANAGER_H
