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

#ifndef DEVICEICONBUILDER_H
#define DEVICEICONBUILDER_H

#include "hoverwidget.h"
#include "scopy-gui_export.h"
#include <QDomElement>
#include <QLabel>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT DeviceIconBuilder : public QObject
{
	Q_OBJECT
public:
	typedef enum
	{
		SQUARE,
		ROUNDED_SQUARE

	} IconShape;

	DeviceIconBuilder(QObject *parent = nullptr);
	~DeviceIconBuilder();

	/**
	 * @brief Builds the device icon.
	 * @return QWidget*
	 */
	QWidget *build();

	/**
	 * @brief Sets the IconShape.
	 * @param s - Related to this shape, the corresponding svg icon will be selected.
	 */
	DeviceIconBuilder &shape(IconShape s);

	/**
	 * @brief Sets the icon bgColor.
	 * @param bgColor - A QString with the desired color code.
	 */
	DeviceIconBuilder &color(QString bgColor);

	/**
	 * @brief Sets the header widget and the header hover widget offset.
	 * @param w - Header widget.
	 * @param offset - The offset related to the top-center position of the icon. By default is set in the middle of
	 * the header section.
	 */
	DeviceIconBuilder &headerWidget(QWidget *w, QPoint offset = {0, 24});

	/**
	 * @brief Sets the footer widget and the footer hover widget offset.
	 * @param w - Footer widget.
	 * @param offset - The offset related to the bottom-center position of the icon. By default is set in the middle
	 * of the footer section.
	 */
	DeviceIconBuilder &footerWidget(QWidget *w, QPoint offset = {0, -10});

private:
	void createHover(QWidget *icon, QWidget *w, HoverPosition anchor, HoverPosition content, QPoint offset);
	void setIconShape(QLabel *icon, QString shapePath, QList<Util::SVGSpec> specs);

	IconShape m_shape;
	QString m_bgColor;
	QWidget *m_headerWidget;
	QPoint m_headerOffset;
	QWidget *m_footerWidget;
	QPoint m_footerOffset;

	const int DEVICE_BTN_HEIGHT = 80;
};
} // namespace scopy

#endif // DEVICEICONBUILDER_H
