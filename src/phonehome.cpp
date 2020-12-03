#include "phonehome.h"
#include <QDateTime>
#include <QTime>

using namespace adiscope;

PHONEHOME_H

PhoneHome::PhoneHome(QSettings *settings, Preferences *pref) :
	ApiObject(), m_timestamp(0), m_versionsJson(""),
	preferences(pref), settings(settings)
{
	setObjectName("phonehome");
	load(*settings);
}

PhoneHome::~PhoneHome() {
	save(*settings);
}

void PhoneHome::setPreferences(Preferences* preferences)
{
	PhoneHome::preferences = preferences;
}

void PhoneHome::versionsRequest()
{
	qint64 timeout = 24 * 60 * 60 * 1000; // 24 hours
	//qint64 timeout = 5 * 1000;  // 5 seconds
	qint64 now = QDateTime::currentMSecsSinceEpoch();

	if(m_timestamp +  timeout < now )
	{
		// from swdownloads
		QNetworkAccessManager* manager = new QNetworkAccessManager();
		connect(manager, &QNetworkAccessManager::finished, this, &PhoneHome::onVersionsRequestFinished);
		connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);

		manager->get(QNetworkRequest(QUrl(preferences->getCheck_updates_url())));
	}
	else
	{
		// from cache
		if(!m_versionsJson.isEmpty())
		{
			QJsonDocument doc = QJsonDocument::fromJson(m_versionsJson.toUtf8());
			extractVersionStringsFromJson(doc);
		}
	}
}

void PhoneHome::extractVersionStringsFromJson(QJsonDocument doc)
{
	auto content = doc.object().toVariantMap();
	m_scopyVersion = content["scopy"].toMap()["version"].toString();
	m_m2kVersion = content["m2k-fw"].toMap()["version"].toString();
	m_scopyLink = content["scopy"].toMap()["link"].toString();
	m_m2kLink = content["m2k-fw"].toMap()["link"].toString();
	Q_EMIT scopyVersionChanged();
	Q_EMIT m2kVersionChanged();
}

void PhoneHome::onVersionsRequestFinished(QNetworkReply* reply)
{
	auto err = reply->error();
	if (err == QNetworkReply::NoError) {
		auto data = reply->readAll().trimmed();
		auto doc = QJsonDocument::fromJson(data);
		extractVersionStringsFromJson(doc);
		m_versionsJson = QString(data.trimmed());
		m_timestamp = QDateTime::currentMSecsSinceEpoch();

	} else {
		qDebug() << "Wasn't able to access versions database!\n";
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
void PhoneHome::setVersionsJson(QString val)
{
	m_versionsJson = val;

}

