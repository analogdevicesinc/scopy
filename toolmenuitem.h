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

#ifndef TOOLMENUITEM_H
#define TOOLMENUITEM_H

#include "gui/basemenuitem.h"
#include "gui/customPushButton.h"

namespace adiscope {
/**
 * @brief The ToolMenuItem class
 */
class ToolMenuItem : public BaseMenuItem
{
	Q_OBJECT
public:
	explicit ToolMenuItem(QString id, QString name, QString iconPath, QWidget *parent = nullptr);
	virtual ~ToolMenuItem();

	QPushButton *getToolBtn() const;
	QPushButton *getToolRunBtn() const;

	void setToolEnabled(bool disabled);
	void enableDoubleClick(bool enable);

	bool eventFilter(QObject *watched, QEvent *event);

	void setTemporaryName(QString str);
	void clearTemporaryName();
	void hideText(bool hidden);

	const QString &getId() const;

Q_SIGNALS:
	void doubleclick();


public Q_SLOTS:
	void setDisabled(bool disabled);

protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

	void dragMoveEvent(QDragMoveEvent *event);
	void dragLeaveEvent(QDragLeaveEvent *event);

	void mouseMoveEvent(QMouseEvent *event);
private:
	QPushButton *toolBtn;
	CustomPushButton *toolRunBtn;

	QString id;
	QString name;
	QString iconPath;

private:
	void _buildUI();
};
}
#endif // TOOLMENUITEM_H
