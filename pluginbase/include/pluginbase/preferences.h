#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QObject>
#include <QMap>
#include <QSettings>
#include "scopypluginbase_export.h"

namespace adiscope {
/**
 * @brief The Preferences class
 * Singleton preferences class. Maps QString keys to QVariant values.
 */
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

	/**
	 * @brief init
	 * Initializes a preferences value. If preference already exist, the value is not updated
	 */
	void init(QString, QVariant);
	/**
	 * @brief clear
	 * clears preferences table
	 */
	void clear();
	/**
	 * @brief get
	 * @return gets preferences value
	 */
	QVariant get(QString);
	/**
	 * @brief set
	 * Updates preferences value
	 */
	void set(QString, QVariant);

	QMap<QString, QVariant> getPreferences() const;
	void setPreferences(QMap<QString, QVariant> s);

	/**
	 * @brief save
	 * Save preferences to file
	 */
	void save();
	/**
	 * @brief load
	 * Loads preferences from file
	 */
	void load();
	/**
	 * @brief setPreferencesFilename
	 * @param s
	 * Sets preferences filename
	 */
	void setPreferencesFilename(QString s);

Q_SIGNALS:
	/**
	 * @brief preferenceChanged
	 * Signal emitted when a preference is changed
	 * Useful to monitor preferences
	 */
	void preferenceChanged(QString, QVariant);

private:

	QMap<QString,QVariant> p;
	QSettings *s;
	static Preferences * pinstance_;

};
}

#endif // PREFERENCES_H
