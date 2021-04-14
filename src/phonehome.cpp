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


#include "phonehome.h"
#include <QDateTime>
#include <QTime>

using namespace adiscope;

bool PhoneHome::getDone() const
{
	return done;
}

PhoneHome::PhoneHome(QSettings *settings, Preferences *pref) :
	ApiObject(), m_timestamp(0), m_versionsJson(""),
	preferences(pref), settings(settings), done(false)
{
	setObjectName("phonehome");
	manager = new QNetworkAccessManager(this);
	load(*settings);
}

PhoneHome::~PhoneHome() {
	save(*settings);
	delete manager;
}

void PhoneHome::setPreferences(Preferences* preferences)
{
	PhoneHome::preferences = preferences;
}

void PhoneHome::versionsRequest(bool force)
{
	const qint64 now = QDateTime::currentMSecsSinceEpoch();
	const qint64 timeout = 24 * 60 * 60 * 1000; // 24 hours
	//qint64 timeout = 5 * 1000;  // 5 seconds for testing

	if(m_timestamp +  timeout < now  || force) {
		// from swdownloads
		connect(manager, &QNetworkAccessManager::finished, this, &PhoneHome::onVersionsRequestFinished);

		manager->get(QNetworkRequest(QUrl(preferences->getCheck_updates_url())));
		Q_EMIT scopyVersionCheckRequested();
	} else {
		// from cache
		if(!m_versionsJson.isEmpty()) {
			const QJsonDocument doc = QJsonDocument::fromJson(m_versionsJson.toUtf8());
			extractVersionStringsFromJson(doc);
		}
		Q_EMIT checkUpdatesFinished(m_timestamp);
	}
}

void PhoneHome::extractVersionStringsFromJson(const QJsonDocument &doc)
{
	auto content = doc.object().toVariantMap();
	m_scopyVersion = content["scopy"].toMap()["version"].toString();
	m_m2kVersion = content["m2k-fw"].toMap()["version"].toString();
	m_scopyLink = content["scopy"].toMap()["link"].toString();
	m_m2kLink = content["m2k-fw"].toMap()["link"].toString();
	done = true;
	Q_EMIT scopyVersionChanged();
	Q_EMIT m2kVersionChanged();

}

void PhoneHome::onVersionsRequestFinished(QNetworkReply* reply)
{
	const auto err = reply->error();
	if (err == QNetworkReply::NoError) {
		const auto data = reply->readAll().trimmed();
		const auto doc = QJsonDocument::fromJson(data);
		extractVersionStringsFromJson(doc);
		m_versionsJson = QString(data.trimmed());
		m_timestamp = QDateTime::currentMSecsSinceEpoch();
		Q_EMIT checkUpdatesFinished(m_timestamp);

	} else {
		qDebug() << "Wasn't able to access versions database!\n";
		Q_EMIT checkUpdatesFinished(0);
	}
}

QString PhoneHome::getTimestamp() const
{
	return QString::number(m_timestamp);
}
void PhoneHome::setTimestamp(QString val)
{
	m_timestamp = val.toLongLong();
}
QString PhoneHome::getVersionsJson() const
{
	return m_versionsJson;
}
void PhoneHome::setVersionsJson(const QString &val)
{
	m_versionsJson = val;

}

