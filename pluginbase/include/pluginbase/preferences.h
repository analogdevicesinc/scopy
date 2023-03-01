#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QObject>
#include <QMap>
#include <QSettings>
#include "scopypluginbase_export.h"

namespace adiscope {
class SCOPYPLUGINBASE_EXPORT Preferences : public QObject
{
	Q_OBJECT
protected:
	Preferences(QObject *parent = nullptr);
	~Preferences();

public:
	// singleton
	Preferences(Preferences &other) = delete;
	void operator=(const Preferences &) = delete;
	static Preferences *GetInstance();

	void init(QString, QVariant);
	void clear();
	QVariant get(QString);
	void set(QString, QVariant);

	QMap<QString, QVariant> getPreferences() const;
	void setPreferences(QMap<QString, QVariant> s);

	void save();
	void load();
	void setPreferencesFilename(QString s);

Q_SIGNALS:
	void preferenceChanged(QString, QVariant);

private:

	QMap<QString,QVariant> p;
	QSettings *s;
	static Preferences * pinstance_;

};
}

#endif // PREFERENCES_H
