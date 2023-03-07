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

#include "toolmenuitem.h"

#include <QHBoxLayout>
#include <QSpacerItem>
#include <QPaintEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include "gui/dynamicWidget.h"
#include "qdebug.h"
#include "gui/utils.h"
#include <QLoggingCategory>

using namespace adiscope;
Q_LOGGING_CATEGORY(CAT_TOOLMENUITEM, "ToolMenuItem")

ToolMenuItem::ToolMenuItem(QString id, QString name, QString iconPath, QWidget *parent):
	BaseMenuItem(parent),
	toolBtn(nullptr),
	toolRunBtn(nullptr),
	id(id),
	name(name),
	iconPath(iconPath)
{
	_buildUI();

	// Load stylesheets
	this->setStyleSheet(Util::loadStylesheetFromFile(":/stylesheets/toolMenuItem.qss"));

#ifdef __ANDROID__
	setDynamicProperty(this, "allowHover", false);
#else
	setDynamicProperty(this, "allowHover", true);
	enableDoubleClick(true);
#endif
}

ToolMenuItem::~ToolMenuItem()
{

}

QPushButton *ToolMenuItem::getToolBtn() const
{
	return toolBtn;
}

QPushButton *ToolMenuItem::getToolRunBtn() const
{
	return toolRunBtn;
}

//void ToolMenuItem::setToolEnabled(bool enabled)
//{
//	BaseMenuItem::setVisible(enabled);
//	Util::retainWidgetSizeWhenHidden(this, enabled);
//	setEnabled(enabled);
//}

void ToolMenuItem::enableDoubleClick(bool enable)
{
	if (enable) {
		toolBtn->installEventFilter(this);
	} else {
		toolBtn->removeEventFilter(this);
		removeEventFilter(this);
	}
}

void ToolMenuItem::setSeparator(bool top, bool bot)
{
	_enableBotSeparator(bot);
	_enableTopSeparator(top);
}

bool ToolMenuItem::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonDblClick) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		if (mouseEvent->button() == Qt::LeftButton) {
			if (isEnabled()) {
				Q_EMIT doubleclick();
				return true;
			}
		}
	}

	return QObject::event(event);
}

void ToolMenuItem::setName(QString str) {
	this->name = str;
	toolBtn->setText(name);
}

void ToolMenuItem::setTemporaryName(QString str)
{
	toolBtn->setText(str);
}

void ToolMenuItem::clearTemporaryName()
{
	toolBtn->setText(name);
}

void ToolMenuItem::hideText(bool hidden)
{	
	if(hidden) {
		toolBtn->setText("");
	} else {
		toolBtn->setText(name);
	}
}

void ToolMenuItem::setDisabled(bool disabled)
{
	BaseMenuItem::setDisabled(disabled);
}

void ToolMenuItem::mouseMoveEvent(QMouseEvent *event)
{
#ifndef __ANDROID__
	BaseMenuItem::mouseMoveEvent(event);
	setDynamicProperty(this, "allowHover", false);
#endif
}

const QString &ToolMenuItem::getId() const
{
	return id;
}

void ToolMenuItem::enterEvent(QEvent *event)
{
#ifndef __ANDROID__
	setDynamicProperty(this, "allowHover", true);
	event->accept();
#endif
}

void ToolMenuItem::leaveEvent(QEvent *event)
{
#ifndef __ANDROID__
	setDynamicProperty(this, "allowHover", false);
	event->accept();
#endif
}

void ToolMenuItem::dragMoveEvent(QDragMoveEvent *event)
{
#ifndef __ANDROID__
	setDynamicProperty(this, "allowHover", false);
	BaseMenuItem::dragMoveEvent(event);
#endif
}

void ToolMenuItem::dragLeaveEvent(QDragLeaveEvent *event)
{
#ifndef __ANDROID__
	setDynamicProperty(this, "allowHover", true);
	BaseMenuItem::dragLeaveEvent(event);
#endif
}

void ToolMenuItem::_buildUI()
{
	QWidget *main = new QWidget(this);
	QVBoxLayout *mainLayout = new QVBoxLayout(main);
	mainLayout->setSpacing(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	QWidget *toolOption = new QWidget(this);
	QHBoxLayout *layout = new QHBoxLayout(toolOption);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	toolBtn = new QPushButton(name);
	toolBtn->setSizePolicy(QSizePolicy::Minimum,
				 QSizePolicy::Minimum);
	toolRunBtn = new CustomPushButton(this);
	layout->addWidget(toolBtn);
	layout->addWidget(toolRunBtn);

	setDynamicProperty(toolRunBtn, "stopButton", true);
	toolRunBtn->setMaximumSize(32, 32);
	toolBtn->setMinimumHeight(42);

	toolBtn->setIcon(QIcon::fromTheme(iconPath));
	toolBtn->setCheckable(true);
	toolBtn->setIconSize(QSize(32, 32));

	toolRunBtn->setCheckable(true);
	toolRunBtn->setText("");

	toolBtn->setFlat(true);
	toolRunBtn->setFlat(true);
	qDebug(CAT_TOOLMENUITEM)<<toolRunBtn;

	mainLayout->addWidget(toolOption);

	setMaximumHeight(44);

	setEnabled(false);
	setVisible(false);

	setWidget(main);
}

#include "moc_toolmenuitem.cpp"
