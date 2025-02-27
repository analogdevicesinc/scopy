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

#ifndef BROWSEMENU_H
#define BROWSEMENU_H

#include "toolmenu.h"

#include <QLabel>
#include <QPushButton>
#include <QWidget>
#include <scopy-core_export.h>

namespace scopy {

class SCOPY_CORE_EXPORT BrowseMenu : public QWidget
{
	Q_OBJECT
public:
	enum MenuAlignment
	{
		MA_TOPLAST,
		MA_BOTTOMLAST
	};

	BrowseMenu(QWidget *parent = nullptr);
	~BrowseMenu();

	ToolMenu *toolMenu() const;

public Q_SLOTS:
	void onToolStackChanged(QString id);

Q_SIGNALS:
	void requestTool(QString tool);
	void requestSave();
	void requestLoad();
	void collapsed(bool collapsed);

private:
	void add(QWidget *w, QString name, MenuAlignment position);
	void toggleCollapsed();
	void hideBtnText(QPushButton *btn, QString name, bool hide);
	QPushButton *createBtn(QString name, QString iconPath, QWidget *parent = nullptr);
	QFrame *createHLine(QWidget *parent = nullptr);
	QWidget *createHeader(QWidget *parent = nullptr);
	QLabel *createScopyLogo(QWidget *parent = nullptr);

	QWidget *m_content;
	QVBoxLayout *m_contentLay;
	QSpacerItem *m_spacer;
	ToolMenu *m_toolMenu;
	QPushButton *m_btnCollapse;
	QMap<QString, QPushButton *> m_btnsMap;
	bool m_collapsed;

	const QString HOME_ID = "home";
	const QString PACKAGE_ID = "package";
	const QString PREFERENCES_ID = "preferences";
	const QString ABOUT_ID = "about";
};
} // namespace scopy

#endif // BROWSEMENU_H
