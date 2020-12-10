/*
 * Copyright (c) 2020 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef PHONEHOME_H
#define PHONEHOME_H
#include "preferences.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextCodec>
#include <QJsonDocument>
#include <QJsonObject>

namespace adiscope {
class PhoneHome : public ApiObject
{
	Q_OBJECT
private:
	QString m_lastRequestContent;
	QString m_scopyVersion;
	QString m_m2kVersion;
	QString m_scopyLink;
	QString m_m2kLink;
	qint64 m_timestamp;
	QString m_versionsJson;
	PhoneHome* instance;
	Preferences* preferences;
	QSettings *settings;


	Q_PROPERTY(QString timestamp READ getTimestamp WRITE setTimestamp)
	Q_PROPERTY(QString versionsJson READ getVersionsJson WRITE setVersionsJson)

public:
	PhoneHome(QSettings *settings, Preferences *preferences);
	~PhoneHome();
	void versionsRequest(bool force = false);
	void extractVersionStringsFromJson(const QJsonDocument &doc);
	QString getScopyVersion() { return m_scopyVersion; }
	QString getM2kVersion() { return m_m2kVersion; }
	QString getScopyLink() { return m_scopyLink; }
	QString getM2kLink() { return m_m2kLink; }
	void setPreferences(Preferences* preferences);
	QString getTimestamp() const;
	void setTimestamp(QString);
	QString getVersionsJson() const;
	void setVersionsJson(const QString &val);

Q_SIGNALS:
	void scopyVersionChanged();
	void m2kVersionChanged();
	void checkUpdatesFinished(qint64);

public Q_SLOTS:
	void onVersionsRequestFinished(QNetworkReply*);
};
}
#endif // PHONEHOME_H
