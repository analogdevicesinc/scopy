#ifndef PREFERENCESHELPER_H
#define PREFERENCESHELPER_H
#include "preferences.h"
#include "scopy-pluginbase_export.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

namespace scopy {
/**
 * @brief The PreferencesHelper class
 * PreferencesHelper class used to create widgets and connect the widgets to the Preferences singleton
 */
class SCOPY_PLUGINBASE_EXPORT PreferencesHelper
{

public:
	static QCheckBox *addPreferenceCheckBox(Preferences *p, QString id, QString description,
						QObject *parent = nullptr);
	static QWidget *addPreferenceEdit(Preferences *p, QString id, QString description, QObject *parent = nullptr);
	static QWidget *addPreferenceCombo(Preferences *p, QString id, QString description, QStringList options,
					   QObject *parent = nullptr);
	static QWidget *addPreferenceComboList(Preferences *p, QString id, QString description,
					       QList<QPair<QString, QVariant>> options, QObject *parent);
};
} // namespace scopy
#endif // PREFERENCESHELPER_H
