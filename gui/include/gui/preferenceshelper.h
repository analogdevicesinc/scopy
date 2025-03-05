/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef PREFERENCESHELPER_H
#define PREFERENCESHELPER_H
#include "scopy-gui_export.h"

#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

#include <pluginbase/preferences.h>
#include <widgets/filebrowserwidget.h>

#define PREFERENCE_CHECK_BOX(p, id, title, description, parent)                                                        \
	PreferencesHelper::addPreferenceCheckBox(p, id, title, description, parent)

#define PREFERENCE_EDIT(p, id, title, description, parent)                                                             \
	PreferencesHelper::addPreferenceEdit(p, id, title, description, parent)

#define PREFERENCE_EDIT_VALIDATION(p, id, title, description, validator, parent)                                       \
	PreferencesHelper::addPreferenceEditValidation(p, id, title, description, validator, parent)

#define PREFERENCE_COMBO(p, id, title, description, options, parent)                                                   \
	PreferencesHelper::addPreferenceCombo(p, id, title, description, options, parent)

#define PREFERENCE_COMBO_LIST(p, id, title, description, options, parent)                                              \
	PreferencesHelper::addPreferenceComboList(p, id, title, description, options, parent)

#define PREFERENCE_FILE_BROWSER(p, id, title, description, type, parent)                                               \
	PreferencesHelper::addPreferenceFileBrowser(p, id, title, description, type, parent)

namespace scopy {

class SmallOnOffSwitch;
/**
 * @brief The PreferencesHelper class
 * PreferencesHelper class used to create widgets and connect the widgets to the Preferences singleton
 */
class SCOPY_GUI_EXPORT PreferencesHelper
{

public:
	static QWidget *addPreferenceCheckBox(Preferences *p, QString id, QString title, QString description,
					      QObject *parent = nullptr);
	static QWidget *addPreferenceEdit(Preferences *p, QString id, QString title, QString description,
					  QObject *parent = nullptr);
	static QWidget *addPreferenceEditValidation(Preferences *p, QString id, QString title, QString description,
						    std::function<bool(const QString &)> validator,
						    QObject *parent = nullptr);
	static QWidget *addPreferenceCombo(Preferences *p, QString id, QString title, QString description,
					   QStringList options, QObject *parent = nullptr);
	static QWidget *addPreferenceComboList(Preferences *p, QString id, QString title, QString description,
					       QList<QPair<QString, QVariant>> options, QObject *parent);
	static QWidget *addPreferenceFileBrowser(Preferences *p, QString id, QString title, QString description,
						 FileBrowserWidget::BrowserDialogType type, QObject *parent = nullptr);
	static QWidget *setupDescriptionButton(QString id, QString description, QObject *parent);
};
} // namespace scopy
#endif // PREFERENCESHELPER_H
