#ifndef PHONEHOME_H
#define PHONEHOME_H
#include "preferences.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTextCodec>

namespace adiscope {
class PhoneHome: public QObject
{
	Q_OBJECT
private:
	QString m_lastRequestContent;
	QString m_scopyVersion;
	QString m_m2kVersion;
	QString m_scopyLink;
	QString m_m2kLink;
	PhoneHome* instance;
	Preferences* preferences;

public:
	PhoneHome();
	void versionsRequest();
	QString getScopyVersion() { return m_scopyVersion; }
	QString getM2kVersion() { return m_m2kVersion; }
	QString getScopyLink() { return m_scopyLink; }
	QString getM2kLink() { return m_m2kLink; }
	void setPreferences(Preferences* preferences);

Q_SIGNALS:
	void scopyVersionChanged();
	void m2kVersionChanged();

public Q_SLOTS:
	void onVersionsRequestFinished(QNetworkReply*);
};
}
#endif // PHONEHOME_H
