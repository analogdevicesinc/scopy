#ifndef PREFERENCE_OPTION_H
#define PREFERENCE_OPTION_H

/* Qt includes */
#include <QWidget>
#include <QString>
#include <QSettings>

/* Local includes */
#include "apiObject.hpp"

namespace Ui {
class PreferenceOption;
}

namespace adiscope {
class PreferenceOption_API;

class PreferenceOption : public QWidget
{
	Q_OBJECT

public:
	explicit PreferenceOption(QWidget *parent = 0, QString description = "",
				  QSettings *settings = 0);
	~PreferenceOption();

	const QString getDescription() const;
	const bool isChecked() const;
	void setChecked(bool checked);
	void load();

Q_SIGNALS:
	void enabled(bool);

private:
	Ui::PreferenceOption *p_ui;
	QString p_description;
	PreferenceOption_API *p_api;
	QSettings *p_settings;
};

class PreferenceOption_API : public ApiObject
{
	Q_OBJECT

	Q_PROPERTY(bool enabled READ isChecked WRITE setChecked);

	const bool isChecked() const;
	void setChecked(bool checked);

public:
	explicit PreferenceOption_API(PreferenceOption *po) : ApiObject(), po(po) {}
	~PreferenceOption_API() {}

private:
	PreferenceOption *po;
};
}

#endif // PREFERENCE_OPTION_H
