/*
 * Copyright (c) 2025 Analog Devices Inc.
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

#ifndef MENUITEMFACTORY_H
#define MENUITEMFACTORY_H

#include <QWidget>
#include <QVariantMap>
#include <QString>

namespace scopy::extprocplugin {

class MenuItemFactory : public QObject
{
	Q_OBJECT

public:
	MenuItemFactory(QObject *parent = nullptr);
	~MenuItemFactory();

	QWidget *createWidget(const QString &field, const QVariantMap &params);
	QVariantMap toVariantMap();

Q_SIGNALS:
	void itemParamChanged(const QString &field, const QVariant &value);

private:
	void onItemParamChanged(const QString &field, const QVariant &value);

	QWidget *createSpinBox(const QString &field, const QVariantMap &params);
	QWidget *createLineEdit(const QString &field, const QVariantMap &params);
	QWidget *createCombo(const QString &field, const QVariantMap &params);
	QWidget *createCheckBox(const QString &field, const QVariantMap &params);

	QVariantMap m_currentParams;
};

} // namespace scopy::extprocplugin

#endif // MENUITEMFACTORY_H
