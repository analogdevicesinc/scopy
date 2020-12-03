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
	void versionsRequest();
	void extractVersionStringsFromJson(QJsonDocument doc);
	QString getScopyVersion() { return m_scopyVersion; }
	QString getM2kVersion() { return m_m2kVersion; }
	QString getScopyLink() { return m_scopyLink; }
	QString getM2kLink() { return m_m2kLink; }
	void setPreferences(Preferences* preferences);
	QString getTimestamp() const;
	void setTimestamp(QString);
	QString getVersionsJson() const;
	void setVersionsJson(QString);

Q_SIGNALS:
	void scopyVersionChanged();
	void m2kVersionChanged();

public Q_SLOTS:
	void onVersionsRequestFinished(QNetworkReply*);
};
}
#endif // PHONEHOME_H
