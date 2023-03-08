#ifndef PREFERENCESHELPER_H
#define PREFERENCESHELPER_H
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include "preferences.h"
#include "scopypluginbase_export.h"

namespace adiscope {
/**
 * @brief The PreferencesHelper class
 * PreferencesHelper class used to create widgets and connect the widgets to the Preferences singleton
 */
class SCOPYPLUGINBASE_EXPORT PreferencesHelper {

public:
	static QCheckBox *addPreferenceCheckBox(Preferences *p, QString id, QString description, QObject *parent = nullptr);
	static QLineEdit *addPreferenceEdit(Preferences *p, QString id, QString description, QObject *parent = nullptr);
	static QComboBox *addPreferenceCombo(Preferences *p, QString id, QString description, QStringList options, QObject *parent = nullptr);
};
}
#endif // PREFERENCESHELPER_H
