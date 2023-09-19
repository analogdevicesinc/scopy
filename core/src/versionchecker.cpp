#include "versionchecker.h"
#include <QApplication>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QLoggingCategory>
#include <QNetworkRequest>
#include <pluginbase/preferences.h>
#include <common/scopyconfig.h>
Q_LOGGING_CATEGORY(CAT_VERSION, "VersionCache")

using namespace scopy;

VersionChecker* VersionChecker::pinstance_{nullptr};

VersionChecker::VersionChecker(QObject *parent) : QObject(nullptr),
						  m_nam(nullptr),
						  m_cacheOutdated(true),
						  m_onlineCheckInProgress(false),
						  m_ttl(10),
						  m_thread(new QThread(this)),
						  m_cacheFilePath(scopy::config::settingsFolderPath() + "/version.json")
{}

VersionChecker::~VersionChecker()
{
	m_thread->quit();
	m_thread->wait();
}

void VersionChecker::init() {
	m_onlineCheckInProgress = true;
	m_cacheOutdated = checkCacheOutdated();
	bool checkOnline = Preferences::get("general_check_online_version").toBool();
	if (m_cacheOutdated) {
		if (!checkOnline) {
			qWarning(CAT_VERSION) << "Cache is outdated, but Scopy is not allowed to check online.";
			return;
		}
		moveToThread(m_thread);
		connect(m_thread, &QThread::started, this, &VersionChecker::pullNewCache, Qt::QueuedConnection);
		m_thread->start();
	} else {
		readCache();
	}
}

VersionChecker *VersionChecker::GetInstance()
{
	if (pinstance_ == nullptr)
	{
		pinstance_ = new VersionChecker(QApplication::instance()); // singleton has the app as parent
		connect(pinstance_, &VersionChecker::cacheUpdated, pinstance_, &VersionChecker::updateSubscriptions);
		connect(pinstance_, &VersionChecker::addedNewSubscription, pinstance_, &VersionChecker::updateSubscriptions);
		pinstance_->init();
	}
	return pinstance_;
}

void VersionChecker::readCache() {
	qInfo(CAT_VERSION) << "Reading cache.";

	QFile f(m_cacheFilePath);
	bool checkOnline = Preferences::get("general_check_online_version").toBool();
	if(f.exists()) {
		f.open(QIODevice::ReadOnly);
		QString content = f.readAll();
		m_cache = QJsonDocument::fromJson(content.toUtf8());
		f.close();

		if (m_cache.isEmpty() && checkOnline) {
			// This can happen if the file is created, but Scopy does not have the ability to pull online
			// versions.
			pullNewCache();
		}

		m_onlineCheckInProgress = false;
		Q_EMIT cacheUpdated();
	} else {
		qDebug(CAT_VERSION) << "Cache file does not exist. Creating one...";
		bool isOpened = f.open(QIODevice::WriteOnly);
		if (isOpened) {
			if (checkOnline) {
				pullNewCache();
			} else {
				qWarning(CAT_VERSION) << "The file was created, but Scopy is not allowed to check for versions online.";
			}
		} else {
			// on the first ever run of Scopy on a machine, there might be a warning that the file cannot be
			// created, this is because the path to the cache is created only when Scopy shuts down, this
			// should be addressed on the second run of Scopy, when the directory exists.
			qWarning(CAT_VERSION) << "Could not create cache file.";
		}
	}
}

bool VersionChecker::checkCacheOutdated() {
	QFileInfo fi(m_cacheFilePath);

	if (fi.exists()) {
		return fi.fileTime(QFile::FileModificationTime).addDays(1) < QDateTime::currentDateTime();
	}

	return true;
}

void VersionChecker::pullNewCache() {
	qInfo(CAT_VERSION) << "Updating cache.";

	if(!m_nam)
		m_nam = new QNetworkAccessManager(this);

	// we might receive ssl errors saying that the QNetworkAccessManager does not 'like' the peer certificate, in which
	// case we ignore these ssl errors for the current ssl/tls session. QNetworkAccessManager::sslErrors is called
	// before QNetworkAccessManager::finished.
	connect(m_nam, &QNetworkAccessManager::sslErrors, this, [] (QNetworkReply *reply, const QList<QSslError> &errors) {
		qDebug(CAT_VERSION) << "Ignoring ssl errors:" << errors;

		reply->ignoreSslErrors({QSslError::NoPeerCertificate});
	});

	connect(m_nam, &QNetworkAccessManager::finished, this, [this] (QNetworkReply *r){
		int statusCode = r->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		qDebug(CAT_VERSION) << "Status code:" << QVariant(statusCode).toString();

		if (statusCode == 301 && m_ttl > 0) { // REDIRECT
			--m_ttl;
			QUrl newUrl = r->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
			qDebug(CAT_VERSION) << "Redirected from" << r->request().url().toString() << "to" << newUrl.toString();
			QNetworkRequest newRequest(newUrl);
			m_nam->get(newRequest);
			return;
		}

		// 200 -> OK, 304 -> NOT MODIFIED
		if (statusCode != 200 && statusCode != 304) {
			qWarning(CAT_VERSION) <<  "Reply errors:" << r->errorString();
		}

		QByteArray bytes = r->readAll();
		// m_cache needs no further data processing as of this point
		m_cache = QJsonDocument::fromJson(bytes);

		qDebug(CAT_VERSION) << "Received Reply:" << bytes;
		QFile f(m_cacheFilePath);
		f.open(QFile::WriteOnly);
		f.write(QString(bytes).toUtf8());
		f.close();

		m_onlineCheckInProgress = false;
		Q_EMIT cacheUpdated();
	});

	m_nam->get(QNetworkRequest(m_url));
}

void VersionChecker::updateSubscriptions() {
	if (!m_onlineCheckInProgress) {
		std::lock_guard<std::mutex> lock(m_mutex);
		for (auto& function: qAsConst(m_subscriptions)) {
			std::invoke(function, m_cache);
		}

		// the functions that were called should not be called again, in case a new class subscribes
		m_subscriptions.clear();
	}
}

#include "moc_versionchecker.cpp"
