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
#include "dynamicWidget.hpp"
#include "dynamicWidget.hpp"
#include "utils.h"

using namespace adiscope;

ToolMenuItem::ToolMenuItem(QString name, QString iconPath, QWidget *parent):
	BaseMenuItem(parent),
	d_toolBtn(nullptr),
	d_toolStopBtn(nullptr),
	d_name(name),
	d_iconPath(iconPath),
	d_disabled(false),
	d_detached(false)
{
	_buildUI();

	// Load stylesheets
	this->setStyleSheet(Util::loadStylesheetFromFile(":stylesheets/stylesheets/toolMenuItem.qss"));

	setDynamicProperty(this, "allowHover", true);

	connect(d_toolBtn, &QPushButton::toggled, [=](bool on){
		if (!d_detached) {
			setDynamicProperty(this, "selected", on);
		}
	});
}

ToolMenuItem::~ToolMenuItem()
{

}

QPushButton *ToolMenuItem::getToolBtn() const
{
	return d_toolBtn;
}

QPushButton *ToolMenuItem::getToolStopBtn() const
{
	return d_toolStopBtn;
}

void ToolMenuItem::setToolDisabled(bool disabled)
{
	d_disabled = disabled;
	BaseMenuItem::setVisible(!disabled);
	Util::retainWidgetSizeWhenHidden(this, !disabled);
}

void ToolMenuItem::setVisible(bool visible)
{
	if (!d_disabled) {
		BaseMenuItem::setVisible(visible);
	}
}

void ToolMenuItem::setDetached(bool detached)
{
	d_detached = detached;
	Q_EMIT toggleButtonGroup(detached);
}

bool ToolMenuItem::isDetached() const
{
	return  d_detached;
}

void ToolMenuItem::enableDoubleClickToDetach(bool enable)
{
	if (enable) {
		d_toolBtn->installEventFilter(this);
	} else {
		d_toolBtn->removeEventFilter(this);
		removeEventFilter(this);
	}
}

bool ToolMenuItem::eventFilter(QObject *watched, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonDblClick) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		if (mouseEvent->button() == Qt::LeftButton) {
			if (isEnabled()) {
				detach();
				return true;
			}
		}
	}

	return QObject::event(event);
}

void ToolMenuItem::setCalibrating(bool calibrating)
{
	if (calibrating) {
		d_toolBtn->setText(tr("Calibrating..."));
	} else {
		d_toolBtn->setText(d_name);
	}
}

void ToolMenuItem::hideText(bool hidden)
{
	if(hidden) {
		d_toolBtn->setText("");
	} else {
		d_toolBtn->setText(d_name);
	}
}

void ToolMenuItem::setDisabled(bool disabled)
{
	BaseMenuItem::setDisabled(disabled);
}

void ToolMenuItem::mouseMoveEvent(QMouseEvent *event)
{
	BaseMenuItem::mouseMoveEvent(event);
	setDynamicProperty(this, "allowHover", false);
}

void ToolMenuItem::enterEvent(QEvent *event)
{
	setDynamicProperty(this, "allowHover", true);
	event->accept();
}

void ToolMenuItem::leaveEvent(QEvent *event)
{
	setDynamicProperty(this, "allowHover", false);
	event->accept();
}

void ToolMenuItem::dragMoveEvent(QDragMoveEvent *event)
{
	setDynamicProperty(this, "allowHover", false);
	BaseMenuItem::dragMoveEvent(event);
}

void ToolMenuItem::dragLeaveEvent(QDragLeaveEvent *event)
{
	setDynamicProperty(this, "allowHover", true);
	BaseMenuItem::dragLeaveEvent(event);
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
	d_toolBtn = new QPushButton(d_name);
	d_toolBtn->setSizePolicy(QSizePolicy::Minimum,
				 QSizePolicy::Minimum);
	d_toolStopBtn = new CustomPushButton(this);
	layout->addWidget(d_toolBtn);
	layout->addWidget(d_toolStopBtn);

	setDynamicProperty(d_toolStopBtn, "stopButton", true);
	d_toolStopBtn->setMaximumSize(32, 32);
	d_toolBtn->setMinimumHeight(42);

	d_toolBtn->setIcon(QIcon::fromTheme(d_iconPath));
	d_toolBtn->setCheckable(true);
	d_toolBtn->setIconSize(QSize(32, 32));

	d_toolStopBtn->setCheckable(true);

	d_toolBtn->setFlat(true);
	d_toolStopBtn->setFlat(true);

	mainLayout->addWidget(toolOption);

	setMaximumHeight(44);

	setEnabled(false);
	setVisible(false);

	setWidget(main);
}
