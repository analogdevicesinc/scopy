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

#ifndef TOOLMENUMANAGER_H
#define TOOLMENUMANAGER_H

#include "detachedtoolwindowmanager.h"
#include "toolmenu.h"
#include "toolmenuitem.h"

#include <QMap>
#include <QObject>
#include <menusectionwidget.h>
#include "toolstack.h"
#include <pluginbase/toolmenuentry.h>
#include "scopy-core_export.h"

namespace scopy {
class SCOPY_CORE_EXPORT ToolMenuManager : public QObject
{
	Q_OBJECT
public:
	ToolMenuManager(ToolStack *ts, DetachedToolWindowManager *dtm, ToolMenu *toolMenu, QObject *parent = nullptr);
	~ToolMenuManager();

	void addMenuItem(QString deviceId, QString devName, QList<ToolMenuEntry *> tools, int itemIndex = -1);
	void removeMenuItem(QString deviceId);
	void changeToolListContents(QString deviceId, QList<ToolMenuEntry *> tools);

	void showMenuItem(QString id);
	void hideMenuItem(QString id);

public Q_SLOTS:
	void deviceConnected(QString id);
	void deviceDisconnected(QString id);
	void onDisplayNameChanged(QString id, QString devName);

Q_SIGNALS:
	void requestToolSelect(QString id);

private Q_SLOTS:
	void updateTool(QWidget *old);
	void updateToolAttached(bool oldAttach, ToolMenuItem *toolMenuItem);

private:
	void loadToolAttachedState(ToolMenuEntry *tme);
	void saveToolAttachedState(ToolMenuEntry *tme);
	void detachSuccesful(ToolMenuItem *toolMenuItem);
	void attachSuccesful(ToolMenuItem *toolMenuItem);
	void showTool(ToolMenuItem *toolMenuItem);
	void selectTool(ToolMenuItem *toolMenuItem, bool on);
	void setTmeAttached(ToolMenuEntry *tme);
	void createMenuSectionLabel(MenuSectionCollapseWidget *section, QString uri);
	ToolMenuItem *createToolMenuItem(ToolMenuEntry *tme, QWidget *parent = nullptr);

	QString m_prevItem;
	QStringList m_connectedDev;
	ToolStack *m_ts;
	DetachedToolWindowManager *m_dtm;
	ToolMenu *m_toolMenu;
	QMap<QString, MenuSectionCollapseWidget *> m_itemMap;
};
} // namespace scopy

#endif // TOOLMENUMANAGER_H
