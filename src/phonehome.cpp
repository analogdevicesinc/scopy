#include "phonehome.h"

#include <QJsonDocument>
#include <QJsonObject>

using namespace adiscope;

PHONEHOME_H

PhoneHome::PhoneHome()
{
}

void PhoneHome::setPreferences(Preferences* preferences)
{
	PhoneHome::preferences = preferences;
}

void PhoneHome::versionsRequest()
{
	QNetworkAccessManager* manager = new QNetworkAccessManager();
	connect(manager, &QNetworkAccessManager::finished, this, &PhoneHome::onVersionsRequestFinished);
	connect(manager, &QNetworkAccessManager::finished, manager, &QNetworkAccessManager::deleteLater);

	manager->get(QNetworkRequest(QUrl(preferences->getCheck_updates_url())));
}

void PhoneHome::onVersionsRequestFinished(QNetworkReply* reply)
{
	if (reply->error() == QNetworkReply::NoError) {
		auto content = QJsonDocument::fromJson(reply->readAll()).object().toVariantMap();
		m_scopyVersion = content["scopy"].toMap()["version"].toString();
		m_m2kVersion = content["m2k-fw"].toMap()["version"].toString();
		m_scopyLink = content["scopy"].toMap()["link"].toString();
		m_m2kLink = content["m2k-fw"].toMap()["link"].toString();
		Q_EMIT scopyVersionChanged();
		Q_EMIT m2kVersionChanged();
	} else {
		qDebug() << "Wasn't able to access versions database!\n";
	}
}

