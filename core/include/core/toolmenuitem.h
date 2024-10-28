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

#ifndef TOOLMENUITEM_H
#define TOOLMENUITEM_H

#include <QPushButton>
#include <QWidget>
#include <customPushButton.h>

namespace scopy {
class ToolMenuItem : public QWidget
{
	Q_OBJECT
public:
	ToolMenuItem(QString uuid, QString name, QString icon, QWidget *parent = nullptr);
	~ToolMenuItem();

	QPushButton *getToolBtn() const;
	QPushButton *getToolRunBtn() const;

	void enableDoubleClick(bool enable);
	bool eventFilter(QObject *watched, QEvent *event);

	void setName(QString str);
	void setSelected(bool en);

	QString getId() const;
Q_SIGNALS:
	void doubleclick();

public Q_SLOTS:
	void setDisabled(bool disabled);
	void updateItem();

protected:
	void enterEvent(QEvent *event);
	void leaveEvent(QEvent *event);

private:
	QPushButton *m_toolBtn;
	CustomPushButton *m_toolRunBtn;

	QString m_uuid;
	QString m_name;
	QString m_icon;
};
} // namespace scopy

#endif // TOOLMENUITEM_H
