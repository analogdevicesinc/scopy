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

#ifndef PLUGINSTAB_H
#define PLUGINSTAB_H

#include "cardwidget.h"
#include <QFileInfoList>
#include <QWidget>
#include <gridwidget.h>
#include <scopy-core_export.h>

namespace scopy {
class SCOPY_CORE_EXPORT PluginsTab : public QWidget
{
	Q_OBJECT
public:
	PluginsTab(QWidget *parent = nullptr);
	~PluginsTab();

private:
	void addCards();
	QLabel *createStatusLabel(const QString &text, const QString &status);
	CardWidget *createPluginCard(const QString &id, const QString &title, const QString &subtitle,
				     const QString &description, const QString &version = "");

	GridWidget *m_pluginsGrid;
};
} // namespace scopy

#endif // PLUGINSTAB_H
