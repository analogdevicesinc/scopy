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

#ifndef ANALYSISMENU_H
#define ANALYSISMENU_H

#include <QWidget>
#include <QVariantMap>
#include "menuitemfactory.h"

namespace scopy::extprocplugin {
class AnalysisMenu : public QWidget
{
	Q_OBJECT

public:
	AnalysisMenu(QWidget *parent = nullptr);
	~AnalysisMenu();

	QVariantMap getAnalysisConfig();
	void createMenu(const QVariantMap &params);

public Q_SLOTS:
	void updateAnalysisConfig(const QString &field, const QVariant &value);

Q_SIGNALS:
	void applyPressed();

private:
	QWidget *m_w;
	MenuItemFactory *m_factory;
	QVariantMap m_analysisConfig;

	void setupUI();
};
} // namespace scopy::extprocplugin

#endif // ANALYSISMENU_H
