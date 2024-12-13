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

#include "deviceiconbuilder.h"

#include <QLabel>
#include <QSvgRenderer>
#include <QVBoxLayout>
#include <style.h>
#include <utils.h>

using namespace scopy;

DeviceIconBuilder::DeviceIconBuilder(QObject *parent)
	: QObject(parent)
	, m_shape(SQUARE)
	, m_bgColor("")
	, m_headerWidget(nullptr)
	, m_footerWidget(nullptr)
	, m_headerOffset({0, 0})
	, m_footerOffset({0, 0})
{}

DeviceIconBuilder::~DeviceIconBuilder() {}

QWidget *DeviceIconBuilder::build()
{
	QLabel *icon = new QLabel();
	QList<Util::SVGSpec> specs;
	QString shapePath = "";
	switch(m_shape) {
	case SQUARE:
		specs.append({"rect", "icon-rect", "fill", m_bgColor});
		shapePath = ":/gui/icons/device-shapes/square.svg";
		break;
	case ROUNDED_SQUARE:
		specs.append({"use", "icon-path-color", "fill", m_bgColor});
		specs.append({"polygon", "", "fill", m_bgColor});
		shapePath = ":/gui/icons/device-shapes/rounded_square.svg";
		break;
	default:
		break;
	}
	createHover(icon, m_headerWidget, HP_TOP, HP_BOTTOM, m_headerOffset);
	createHover(icon, m_footerWidget, HP_BOTTOM, HP_TOP, m_footerOffset);
	setIconShape(icon, shapePath, specs);
	return icon;
}

DeviceIconBuilder &DeviceIconBuilder::shape(IconShape s)
{
	m_shape = s;
	return *this;
}

DeviceIconBuilder &DeviceIconBuilder::color(QString bgColor)
{
	m_bgColor = bgColor;
	return *this;
}

DeviceIconBuilder &DeviceIconBuilder::headerWidget(QWidget *w, QPoint offset)
{
	m_headerWidget = w;
	m_headerOffset = offset;
	return *this;
}

DeviceIconBuilder &DeviceIconBuilder::footerWidget(QWidget *w, QPoint offset)
{
	m_footerWidget = w;
	m_footerOffset = offset;
	return *this;
}

void DeviceIconBuilder::createHover(QWidget *icon, QWidget *w, HoverPosition anchor, HoverPosition content,
				    QPoint offset)
{
	if(!w) {
		return;
	}
	HoverWidget *hover = new HoverWidget(w, icon, icon);
	hover->setStyleSheet("background-color: transparent; border: 0px;");
	hover->setAnchorPos(anchor);
	hover->setContentPos(content);
	hover->setVisible(true);
	hover->setAnchorOffset(offset);
	hover->raise();
}

void DeviceIconBuilder::setIconShape(QLabel *icon, QString shapePath, QList<Util::SVGSpec> specs)
{
	QPixmap pixmap(shapePath);
	if(!m_bgColor.isEmpty()) {
		pixmap = Util::ChangeSVGAttr(shapePath, specs);
	}
	pixmap = pixmap.scaledToHeight(DEVICE_BTN_HEIGHT, Qt::SmoothTransformation);
	icon->setPixmap(pixmap);
}

#include "moc_deviceiconbuilder.cpp"
