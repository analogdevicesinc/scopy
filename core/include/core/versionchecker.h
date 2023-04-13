#ifndef VERSIONCHECKER_H
#define VERSIONCHECKER_H

#include <QObject>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include "scopycore_export.h"

namespace scopy {
class SCOPYCORE_EXPORT VersionCache : public QObject
{
	Q_OBJECT

protected:
	VersionCache(QObject *parent = nullptr);
	void init();
	~VersionCache();

public:
	VersionCache(VersionCache &other) = delete;
	void operator=(const VersionCache &) = delete;

	static VersionCache *GetInstance();
	void updateCache();
	QJsonDocument cache();
	bool cacheOutdated() const;
	void read();
Q_SIGNALS:
	void cacheUpdated();
private:
	static VersionCache * pinstance_;
	const QString url = "http://swdownloads.analog.com/cse/sw_versions.json";

	QJsonDocument m_cache;
	QString m_cacheFilePath;
	QNetworkAccessManager *m_nam;
	bool m_cacheOutdated;

};
}
#endif // VERSIONCHECKER_H
