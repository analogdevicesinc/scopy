#include "versionchecker.h"

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStandardPaths>
#include <QtConcurrent>

#include <common/scopyconfig.h>
#include <preferences.h>

Q_LOGGING_CATEGORY(CAT_VERSION, "VersionCache")

using namespace scopy;

VersionChecker *VersionChecker::pinstance_{nullptr};

VersionChecker::VersionChecker(QObject *parent)
	: QObject(nullptr)
	, m_nam(nullptr)
	, m_ttl(10)
	, m_cacheFilePath(scopy::config::settingsFolderPath() + "/version.json")
{
	currentState = NOT_INIT;
}

VersionChecker::~VersionChecker() {}

void VersionChecker::init()
{
	bool checkOnline = Preferences::get("general_check_online_version").toBool();

	if(!checkOnline) {
		qInfo(CAT_VERSION) << "update checks disabled";
		return;
	}
	currentState = IN_PROGRESS;

	if(cacheOutdated()) {
		pullNewCache();
	} else {
		readCache();
	}
}

VersionChecker *VersionChecker::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new VersionChecker(QApplication::instance()); // singleton has the app as parent
		connect(pinstance_, &VersionChecker::cacheUpdated, pinstance_, &VersionChecker::updateSubscriptions);
		connect(pinstance_, &VersionChecker::addedNewSubscription, pinstance_,
			&VersionChecker::updateSubscriptions);
		pinstance_->init();
	}
	return pinstance_;
}

void VersionChecker::readCache()
{
	qInfo(CAT_VERSION) << "Reading cache.";

	QFile f(m_cacheFilePath);
	f.open(QIODevice::ReadOnly);
	QString content = f.readAll();
	m_cache = QJsonDocument::fromJson(content.toUtf8());
	f.close();
	currentState = DONE;
	Q_EMIT cacheUpdated();
}

bool VersionChecker::cacheOutdated()
{
	QFileInfo fi(m_cacheFilePath);

	if(fi.exists()) {
		return fi.fileTime(QFile::FileModificationTime).addDays(1) < QDateTime::currentDateTime();
	}

	return true;
}

void VersionChecker::pullNewCache()
{
	qInfo(CAT_VERSION) << "Updating cache.";

	if(!m_nam)
		m_nam = new QNetworkAccessManager(this);

	// we might receive ssl errors saying that the QNetworkAccessManager does not 'like' the peer certificate, in
	// which case we ignore these ssl errors for the current ssl/tls session. QNetworkAccessManager::sslErrors is
	// called before QNetworkAccessManager::finished.
	connect(m_nam, &QNetworkAccessManager::sslErrors, this,
		[](QNetworkReply *reply, const QList<QSslError> &errors) {
			qDebug(CAT_VERSION) << "Ignoring ssl errors:" << errors;
			reply->ignoreSslErrors({QSslError::NoPeerCertificate});
		});

	connect(m_nam, &QNetworkAccessManager::finished, this, [this](QNetworkReply *r) {
		int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		qDebug(CAT_VERSION) << "Status code:" << QVariant(statusCode).toString();

		if(statusCode == 301 && m_ttl > 0) { // REDIRECT
			--m_ttl;
			QUrl newUrl = r->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
			qDebug(CAT_VERSION)
				<< "Redirected from" << r->request().url().toString() << "to" << newUrl.toString();
			QNetworkRequest newRequest(newUrl);
			m_nam->get(newRequest);
			return;
		}

		// 200 -> OK, 304 -> NOT MODIFIED
		if(statusCode != 200 && statusCode != 304) {
			qWarning(CAT_VERSION) << "Reply errors:" << r->errorString();
			currentState = NOT_INIT;
			return;
		}

		QByteArray bytes = r->readAll();
		// m_cache needs no further data processing as of this point
		m_cache = QJsonDocument::fromJson(bytes);

		qDebug(CAT_VERSION) << "Received Reply:" << bytes;
		QFile f(m_cacheFilePath);
		f.open(QFile::WriteOnly);
		f.write(QString(bytes).toUtf8());
		f.close();

		currentState = DONE;

		Q_EMIT cacheUpdated();
	});

	m_nam->get(QNetworkRequest(m_url));
}

void VersionChecker::updateSubscriptions()
{
	switch(currentState) {
	case NOT_INIT:
		qWarning(CAT_VERSION) << "VersionChecker is not initialized.";
		break;
	case IN_PROGRESS:
		qDebug(CAT_VERSION) << "VersionChecker is in progress";
		break;
	case DONE:
		for(auto &function : qAsConst(m_subscriptions)) {
			std::invoke(function, m_cache);
		}

		// the functions that were called should not be called again, in case a new class subscribes
		m_subscriptions.clear();
		break;
	}
}

#include "moc_versionchecker.cpp"
