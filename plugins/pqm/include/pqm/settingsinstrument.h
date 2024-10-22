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

#ifndef SETTINGSINSTRUMENT_H
#define SETTINGSINSTRUMENT_H

#include "qcombobox.h"
#include <QWidget>
#include <QMap>

#define DEVICE_NAME "pqm"
namespace scopy::pqm {
class SettingsInstrument : public QWidget
{
	Q_OBJECT
public:
	SettingsInstrument(QWidget *parent = nullptr);
	~SettingsInstrument();

public Q_SLOTS:
	void attributeAvailable(QMap<QString, QMap<QString, QString>> configAttr);
Q_SIGNALS:
	void updateUi();
	void setAttributes(QMap<QString, QMap<QString, QString>> configAttr);
	void enableTool(bool en, QString tool = "settings");

private Q_SLOTS:
	void onSetBtnPressed();
	void onReadBtnPressed(bool en);
	void setDateTimeAttr(QDateTime dateTime, QString attrName);

private:
	void initConfigSection(QWidget *parent);
	void initSystemTimeSection(QWidget *parent);
	void initTimestampSection(QWidget *parent);
	void initCalibSection(QWidget *parent);
	QWidget *createMenuCombo(QString name, QString attr);
	QWidget *createConfigEdit(QString name, QString attr);
	void updateCbValues(QComboBox *cb, QString attr);

	bool m_readEnabled;
	QMap<QString, QMap<QString, QString>> m_pqmAttr;
	const QString SYSTEM_TIME_ATTR = "system_time";
	const QString LOG_START_ATTR = "log_start_time";
	const QString LOG_STOP_ATTR = "log_stop_time";
};
} // namespace scopy::pqm

#endif // SETTINGSINSTRUMENT_H
