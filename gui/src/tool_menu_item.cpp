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

#include "dynamic_widget.hpp"
#include "utils.hpp"

#include <QHBoxLayout>
#include <QPaintEvent>
#include <QPainter>
#include <QSpacerItem>
#include <QStyle>
#include <QStyleOption>

#include <scopy/gui/tool_menu_item.hpp>

using namespace scopy::gui;

ToolMenuItem::ToolMenuItem(QString name, QString iconPath, QWidget* parent)
	: BaseMenuItem(parent)
	, m_toolBtn(nullptr)
	, m_toolStopBtn(nullptr)
	, m_name(name)
	, m_iconPath(iconPath)
	, m_disabled(false)
	, m_detached(false)
{
	_buildUI();

	// Load stylesheets
	this->setStyleSheet(Util::loadStylesheetFromFile(":stylesheets/tool_menu_item.qss"));

	DynamicWidget::setDynamicProperty(this, "allowHover", true);

	connect(m_toolBtn, &QPushButton::toggled, [=](bool on) {
		if (!m_detached) {
			DynamicWidget::setDynamicProperty(this, "selected", on);
		}
	});
}

ToolMenuItem::~ToolMenuItem() {}

QPushButton* ToolMenuItem::getToolBtn() const { return m_toolBtn; }

QPushButton* ToolMenuItem::getToolStopBtn() const { return m_toolStopBtn; }

void ToolMenuItem::setToolDisabled(bool disabled)
{
	m_disabled = disabled;
	BaseMenuItem::setVisible(!disabled);
	Util::retainWidgetSizeWhenHidden(this, !disabled);
}

void ToolMenuItem::setVisible(bool visible)
{
	if (!m_disabled) {
		BaseMenuItem::setVisible(visible);
	}
}

void ToolMenuItem::setDetached(bool detached)
{
	m_detached = detached;
	Q_EMIT toggleButtonGroup(detached);
}

bool ToolMenuItem::isDetached() const { return m_detached; }

void ToolMenuItem::enableDoubleClickToDetach(bool enable)
{
	if (enable) {
		m_toolBtn->installEventFilter(this);
	} else {
		m_toolBtn->removeEventFilter(this);
		removeEventFilter(this);
	}
}

bool ToolMenuItem::eventFilter(QObject* watched, QEvent* event)
{
	if (event->type() == QEvent::MouseButtonDblClick) {
		QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
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
		m_toolBtn->setText(tr("Calibrating..."));
	} else {
		m_toolBtn->setText(m_name);
	}
}

void ToolMenuItem::hideText(bool hidden)
{
	if (hidden) {
		m_toolBtn->setText("");
	} else {
		m_toolBtn->setText(m_name);
	}
}

void ToolMenuItem::setDisabled(bool disabled) { BaseMenuItem::setDisabled(disabled); }

void ToolMenuItem::mouseMoveEvent(QMouseEvent* event)
{
	BaseMenuItem::mouseMoveEvent(event);
	DynamicWidget::setDynamicProperty(this, "allowHover", false);
}

void ToolMenuItem::enterEvent(QEvent* event)
{
	DynamicWidget::setDynamicProperty(this, "allowHover", true);
	event->accept();
}

void ToolMenuItem::leaveEvent(QEvent* event)
{
	DynamicWidget::setDynamicProperty(this, "allowHover", false);
	event->accept();
}

void ToolMenuItem::dragMoveEvent(QDragMoveEvent* event)
{
	DynamicWidget::setDynamicProperty(this, "allowHover", false);
	BaseMenuItem::dragMoveEvent(event);
}

void ToolMenuItem::dragLeaveEvent(QDragLeaveEvent* event)
{
	DynamicWidget::setDynamicProperty(this, "allowHover", true);
	BaseMenuItem::dragLeaveEvent(event);
}

void ToolMenuItem::_buildUI()
{
	QWidget* main = new QWidget(this);
	QVBoxLayout* mainLayout = new QVBoxLayout(main);
	mainLayout->setSpacing(0);
	mainLayout->setContentsMargins(0, 0, 0, 0);

	QWidget* toolOption = new QWidget(this);
	QHBoxLayout* layout = new QHBoxLayout(toolOption);
	layout->setSpacing(0);
	layout->setContentsMargins(0, 0, 0, 0);
	m_toolBtn = new QPushButton(m_name);
	m_toolBtn->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	m_toolStopBtn = new CustomPushButton(this);
	layout->addWidget(m_toolBtn);
	layout->addWidget(m_toolStopBtn);

	DynamicWidget::setDynamicProperty(m_toolStopBtn, "stopButton", true);
	m_toolStopBtn->setMaximumSize(32, 32);
	m_toolBtn->setMinimumHeight(42);

	m_toolBtn->setIcon(QIcon::fromTheme(m_iconPath));
	m_toolBtn->setCheckable(true);
	m_toolBtn->setIconSize(QSize(32, 32));

	m_toolStopBtn->setCheckable(true);

	m_toolBtn->setFlat(true);
	m_toolStopBtn->setFlat(true);

	mainLayout->addWidget(toolOption);

	setMaximumHeight(44);

	setEnabled(false);
	setVisible(false);

	setWidget(main);
}
