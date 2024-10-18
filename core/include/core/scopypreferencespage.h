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

#ifndef SCOPYPREFERENCESPAGE_H
#define SCOPYPREFERENCESPAGE_H

#include "scopy-core_export.h"

#include <QTabWidget>
#include <QVBoxLayout>
#include <QMap>
#include <widgets/menusectionwidget.h>

namespace scopy {
class SCOPY_CORE_EXPORT ScopyPreferencesPage : public QWidget
{
	Q_OBJECT
public:
	ScopyPreferencesPage(QWidget *parent = nullptr);
	~ScopyPreferencesPage();
	QWidget *buildGeneralPreferencesPage();
	void addHorizontalTab(QWidget *w, QString text);
	void initSessionDevices();
public Q_SLOTS:
	void updateSessionDevices(QMap<QString, QStringList> devices);
Q_SIGNALS:
	void refreshDevicesPressed();
private Q_SLOTS:
	void resetScopyPreferences();

private:
	QTabWidget *tabWidget;
	void initRestartWidget();
	QWidget *restartWidget;
	QMap<QString, QWidget *> m_connDevices;
	QPushButton *m_devRefresh;
	QWidget *buildSaveSessionPreference();
	QWidget *buildResetScopyDefaultButton();
	QVBoxLayout *layout;
	MenuSectionCollapseWidget *m_autoConnectWidget;
	void removeIniFiles(bool backup = true);
	void initUI();
};
} // namespace scopy
#endif // SCOPYPREFERENCESPAGE_H
