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

#include "gui/basemenu.h"
#include "toolmenuitem.h"

#include <QString>
#include <QLabel>
#include <QButtonGroup>
#include "scopycore_export.h"

namespace scopy {
/**
 * @brief The ToolMenu class
 */
class SCOPYCORE_EXPORT ToolMenu : public BaseMenu
{
	Q_OBJECT
public:
	explicit ToolMenu(QWidget *parent = nullptr);
	~ToolMenu();	

	ToolMenuItem *getToolMenuItemFor(QString toolId);
	ToolMenuItem *addTool(QString id, QString name, QString icon, int position = -1);
	ToolMenuItem *createTool(QString id, QString name, QString icon, int position = -1);
	bool removeTool(QString id);
	bool removeTool(ToolMenuItem *tmi);

	void hideMenuText(bool hidden);
	const QVector<ToolMenuItem *> &getTools() const;

	QButtonGroup *getButtonGroup() const;

Q_SIGNALS:
	void toggleAttach(QString);
	void requestToolSelect(QString);

public Q_SLOTS:
	void detachSuccesful(QString);
	void attachSuccesful(QString);

private:
	QVector<ToolMenuItem*> tools;
	QButtonGroup *buttonGroup;


private Q_SLOTS:
	void _updateToolList(short from, short to);

private:
	void _saveState();
	void _loadState();
};
}

#endif // TOOLMENU_H
