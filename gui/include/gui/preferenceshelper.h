#ifndef PREFERENCESHELPER_H
#define PREFERENCESHELPER_H
#include "scopy-gui_export.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

#include <pluginbase/preferences.h>

namespace scopy {

class SmallOnOffSwitch;
/**
 * @brief The PreferencesHelper class
 * PreferencesHelper class used to create widgets and connect the widgets to the Preferences singleton
 */
class SCOPY_GUI_EXPORT PreferencesHelper
{

public:
	static QWidget *addPreferenceCheckBox(Preferences *p, QString id, QString description,
						QObject *parent = nullptr);
	static QWidget *addPreferenceEdit(Preferences *p, QString id, QString description, QObject *parent = nullptr);
	static QWidget *addPreferenceCombo(Preferences *p, QString id, QString description, QStringList options,
					   QObject *parent = nullptr);
	static QWidget *addPreferenceComboList(Preferences *p, QString id, QString description,
					       QList<QPair<QString, QVariant>> options, QObject *parent);
};
} // namespace scopy
#endif // PREFERENCESHELPER_H
