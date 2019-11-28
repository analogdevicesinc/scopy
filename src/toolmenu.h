/*
 * Copyright (c) 2019 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see http://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TOOLMENU_H
#define TOOLMENU_H

#include "basemenu.h"
#include "filter.hpp"
#include "toolmenuitem.h"
#include "preferences.h"

#include <QString>
#include <QLabel>
#include <QButtonGroup>

namespace adiscope {
class ToolMenu : public BaseMenu
{
	Q_OBJECT
public:
	explicit ToolMenu(Preferences *preferences, QWidget *parent = nullptr);
	~ToolMenu();

	void loadToolsFromFilter(Filter *filter);
	ToolMenuItem *getToolMenuItemFor(enum tool tool);

	QButtonGroup *getButtonGroup();

Q_SIGNALS:
	void toolSelected(tool);
	void enableInfoWidget(bool);

private Q_SLOTS:
	void _updateToolList(short from, short to);

private:
	void _buildAllAvailableTools();
	void _saveState();
	void _loadState();
	void _readPreferences();
private:

	static const QStringList d_availableTools;
	static const QStringList d_availableIcons;

	QVector<QPair<BaseMenuItem *, tool>> d_tools;
	QVector<int> d_positions;
	QVector<BaseMenuItem *> d_compatibleTools;
	QString d_current_hw_name;
	QButtonGroup *d_buttonGroup;
	Preferences *d_preferences;
};
}

#endif // TOOLMENU_H
