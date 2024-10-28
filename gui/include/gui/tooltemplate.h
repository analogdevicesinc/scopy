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

#ifndef TOOLBUILDER_H
#define TOOLBUILDER_H

#include <QWidget>
#include <QSpacerItem>
#include "mapstackedwidget.h"
#include "scopy-gui_export.h"

#include <QWidget>

namespace Ui {
class ToolTemplate;
}

namespace scopy {

enum ToolTemplateAlignment
{
	TTA_LEFT,
	TTA_RIGHT
};
enum ToolTemplateContainer
{
	TTC_LEFT,
	TTC_RIGHT,
	TTC_TOP,
	TTC_BOT
};

class SCOPY_GUI_EXPORT ToolTemplate : public QWidget
{
public:
	ToolTemplate(QWidget *parent = nullptr);
	~ToolTemplate();

	QWidget *bottomContainer();
	QWidget *topContainer();
	QWidget *topCentral();
	QWidget *bottomCentral();
	QWidget *leftContainer();
	QWidget *rightContainer();
	QWidget *topContainerMenuControl();
	QWidget *centralContainer();
	MapStackedWidget *leftStack();
	MapStackedWidget *rightStack();
	MapStackedWidget *topStack();
	MapStackedWidget *bottomStack();

	MapStackedWidget *m_leftStack;
	MapStackedWidget *m_rightStack;
	MapStackedWidget *m_topStack;
	MapStackedWidget *m_bottomStack;

	void setTopContainerHeight(int h);
	void setBottomContainerHeight(int h);
	void setLeftContainerWidth(int w);
	void setRightContainerWidth(int w);
	void openLeftContainerHelper(bool open = true);
	void openRightContainerHelper(bool open = true);
	void openTopContainerHelper(bool open = true);
	void openBottomContainerHelper(bool open = true);
	QSpacerItem *getContainerSpacer(QWidget *);

	void addWidgetToTopContainerHelper(QWidget *w, enum ToolTemplateAlignment);
	void addWidgetToTopContainerMenuControlHelper(QWidget *w, ToolTemplateAlignment a);
	void addWidgetToBottomContainerHelper(QWidget *w, ToolTemplateAlignment a);
	void addWidgetToCentralContainerHelper(QWidget *w);
public Q_SLOTS:
	void requestMenu(QString);

private:
	Ui::ToolTemplate *m_ui;
};
} // namespace scopy

#endif // TOOLBUILDER_H
