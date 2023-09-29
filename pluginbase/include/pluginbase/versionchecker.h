#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <functional>
#include <mutex>
#include <QThread>
#include "scopy-pluginbase_export.h"

namespace scopy {
class SCOPY_PLUGINBASE_EXPORT VersionChecker : public QObject
{
	Q_OBJECT

protected:
	explicit VersionChecker(QObject *parent = nullptr);
	void init();
	~VersionChecker() override;

public:
	VersionChecker(VersionChecker &other) = delete;
	void operator=(const VersionChecker &) = delete;

	static VersionChecker *GetInstance();

	typedef enum {
		NOT_INIT,
		IN_PROGRESS,
		DONE
	} state;

	template<typename T, typename R>
	void subscribe(T* object, R(T::* function)(QJsonDocument)){
		auto f = std::bind(function, object, std::placeholders::_1);
		m_subscriptions.push_back(f);
		Q_EMIT addedNewSubscription();
	}

Q_SIGNALS:
	void cacheUpdated();
	void addedNewSubscription();

private Q_SLOTS:
	void updateSubscriptions();

private:

	/**
	 * @brief Pull the json file with the current version from m_url and save it in the m_cache variable.
	 * */
	void pullNewCache();

	/**
	 * @brief Reads the cache from the file m_cacheFilePath.
	 * */
	void readCache();

	/**
	 * @brief Checks whether the cache needs to be pulled from m_url by verifying that the file from m_cacheFilePath
	 * has not been altered in the last 24h.
	 * @return bool
	 * */
	bool cacheOutdated();

	static VersionChecker * pinstance_;
	const QString m_url = "https://swdownloads.analog.com/cse/sw_versions.json";

	QList< std::function<void(QJsonDocument)> > m_subscriptions;
	QJsonDocument m_cache;
	QString m_cacheFilePath;
	QNetworkAccessManager *m_nam;
	int m_ttl; // maximum number of redirects allowed
	state currentState;

};
}
#endif // VERSIONCHECKER_H
