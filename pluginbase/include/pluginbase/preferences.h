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

#ifndef PREFERENCES_H
#define PREFERENCES_H

#include "scopy-pluginbase_export.h"

#include <QMap>
#include <QObject>
#include <QSettings>

namespace scopy {
/**
 * @brief The Preferences class
 * Singleton preferences class. Maps QString keys to QVariant values.
 */
class SCOPY_PLUGINBASE_EXPORT Preferences : public QObject
{
	Q_OBJECT
protected:
	Preferences(QObject *parent = nullptr);
	~Preferences();

public:
	// singleton
	Preferences(Preferences &other) = delete;
	void operator=(const Preferences &) = delete;
	static Preferences *GetInstance();

	/**
	 * @brief init
	 * Initializes a preferences value. If preference already exist, the value is not updated
	 */
	static void init(QString, QVariant);
	void _init(QString, QVariant);
	/**
	 * @brief clear
	 * clears preferences table
	 */
	void clear();
	/**
	 * @brief get
	 * @return gets preferences value
	 */
	static QVariant get(QString val);
	QVariant _get(QString);
	/**
	 * @brief set
	 * Updates preferences value
	 */
	static void set(QString, QVariant);
	void _set(QString, QVariant);

	QMap<QString, QVariant> getPreferences() const;
	void setPreferences(QMap<QString, QVariant> s);

public Q_SLOTS:
	/**
	 * @brief setPreferencesFilename
	 * @param s
	 * Sets preferences filename
	 */
	void setPreferencesFilename(QString s);

public Q_SLOTS:
	/**
	 * @brief save
	 * Save preferences to file
	 */
	void save();
	/**
	 * @brief load
	 * Loads preferences from file
	 */
	void load();

Q_SIGNALS:
	/**
	 * @brief preferenceChanged
	 * Signal emitted when a preference is changed
	 * Useful to monitor preferences
	 */
	void preferenceChanged(QString, QVariant);

	/**
	 * @brief restartRequired
	 * Signal emitted when a preference was changed and app needs to be restarted
	 */
	void restartRequired();

private:
	QMap<QString, QVariant> p;
	QSettings *s;
	static Preferences *pinstance_;
};
} // namespace scopy

#endif // PREFERENCES_H
