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

#include <QButtonGroup>
#include <QLabel>
#include <QString>

#include <scopy/gui/base_menu.hpp>
#include <scopy/gui/filter.hpp>
#include <scopy/gui/preferences.hpp>
#include <scopy/gui/tool_menu_item.hpp>

namespace scopy {
namespace gui {

class ToolMenu : public BaseMenu
{
	Q_OBJECT
public:
	explicit ToolMenu(Preferences* preferences, QWidget* parent = nullptr);
	~ToolMenu();

	void loadToolsFromFilter(Filter* filter);
	ToolMenuItem* getToolMenuItemFor(enum tool tool);

	QButtonGroup* getButtonGroup();

	void hideMenuText(bool hidden);

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
	QStringList m_availableTools;
	QStringList m_availableIcons;

	QVector<QPair<BaseMenuItem*, tool>> m_tools;
	QVector<int> m_positions;
	QVector<BaseMenuItem*> m_compatibleTools;
	QString m_currentHwName;
	QButtonGroup* m_buttonGroup;
	Preferences* m_preferences;
};
} // namespace gui
} // namespace scopy

#endif // TOOLMENU_H
