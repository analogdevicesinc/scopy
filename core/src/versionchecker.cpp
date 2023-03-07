#include "versionchecker.h"
#include <QApplication>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QNetworkReply>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(CAT_VERSION,"VersionCache");

using namespace adiscope;
VersionCache* VersionCache::pinstance_{nullptr};
VersionCache::VersionCache(QObject *parent) : QObject(parent) {
}

VersionCache::~VersionCache()
{

}

VersionCache *VersionCache::GetInstance()
{
	if (pinstance_ == nullptr)
	{
		pinstance_ = new VersionCache(QApplication::instance()); // singleton has the app as parent
		pinstance_->init();
	}
	return pinstance_;
}

void VersionCache::read() {
	m_cacheOutdated = true;
	QFile f(m_cacheFilePath);
	if(f.exists()) {
		QFileInfo fi(f);
		f.open(QIODevice::ReadOnly);
		QString content = f.readAll();
		m_cache.fromJson(content.toUtf8());
		f.close();
		if(fi.fileTime(QFile::FileModificationTime) < QDateTime::currentDateTime().addDays(1) &&
				fi.fileTime(QFile::FileModificationTime) > QDateTime::currentDateTime()) {
			m_cacheOutdated = false;
		}
		Q_EMIT cacheUpdated();
	} else {
		qDebug(CAT_VERSION) << "cache file does not exist";
	}
	qDebug(CAT_VERSION)<< "m_cacheOutdated " << m_cacheOutdated;
}

void VersionCache::init() {
	m_nam = nullptr;
	m_cacheFilePath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)+"version.json";
	read();
}

QJsonDocument VersionCache::cache() {

	return m_cache;
}

bool VersionCache::cacheOutdated() const
{
	return m_cacheOutdated;
}

void VersionCache::updateCache() {

	if(!m_nam)
		m_nam = new QNetworkAccessManager(this);
	QNetworkReply *reply = m_nam->get(QNetworkRequest(QString(url)));

	connect(m_nam,&QNetworkAccessManager::finished,this,[=](QNetworkReply *r){

		QByteArray bytes = reply->readAll();
		QString str = QString::fromUtf8(bytes.data(), bytes.size());
		int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
		qDebug(CAT_VERSION) << QVariant(statusCode).toString();
		if(statusCode == 301)
		{
			QUrl newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
			qDebug(CAT_VERSION) << "redirected from " + r->request().url().toString() + " to " + newUrl.toString();
			QNetworkRequest newRequest(newUrl);
			m_nam->get(newRequest);
			return;
		}
		qDebug(CAT_VERSION)<<r->request().url();
		qDebug(CAT_VERSION)<<"Received Reply";
		qDebug(CAT_VERSION)<<r->error();
		QString content = QString(r->readAll());
		qDebug(CAT_VERSION)<<"content" << content;
		QFile f(m_cacheFilePath);
		f.open(QFile::ReadWrite);
		f.write(content.toUtf8());
		f.close();
		read();
	});
}
