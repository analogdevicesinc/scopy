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

#include "toolmenuitem.h"
#include "dynamicWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QVBoxLayout>
#include <utils.h>
#include <pluginbase/toolmenuentry.h>
#include <QLoggingCategory>
#include "gui/dynamicWidget.h"
#include "gui/utils.h"
#include "qdebug.h"
#include "style_properties.h"

#include <QHBoxLayout>
#include <QLoggingCategory>
#include <QSpacerItem>
#include <style.h>

Q_LOGGING_CATEGORY(CAT_TOOLMENUITEM, "ToolMenuItem")

using namespace scopy;

ToolMenuItem::ToolMenuItem(QString uuid, QString name, QString icon, QWidget *parent)
	: QPushButton(parent)
	, m_uuid(uuid)
	, m_name(name)
	, m_icon(icon)
{
	setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QVBoxLayout *lay = new QVBoxLayout(this);
	setLayout(lay);
	setFixedHeight(Style::getDimension(json::global::unit_3));
	lay->setSpacing(0);
	lay->setContentsMargins(0, 0, 0, 0);

	QWidget *toolOption = new QWidget(this);
	toolOption->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	QHBoxLayout *toolLay = new QHBoxLayout(toolOption);
	toolLay->setSpacing(0);
	toolLay->setContentsMargins(0, 0, 0, 0);
	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
	m_toolRunBtn = new CustomPushButton(toolOption);
	toolLay->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Fixed));
	toolLay->addWidget(m_toolRunBtn);

	m_toolRunBtn->setMaximumWidth(Style::getDimension(json::global::unit_3));

	setIcon(QIcon::fromTheme(m_icon));
	setCheckable(true);
	setIconSize(QSize(Style::getDimension(json::global::unit_2_5), Style::getDimension(json::global::unit_2_5)));

	m_toolRunBtn->setCheckable(true);
	m_toolRunBtn->setText("");

	m_toolRunBtn->setFlat(true);

	lay->addWidget(toolOption);

	setAttribute(Qt::WA_StyledBackground, true);
#ifdef __ANDROID__
	setDynamicProperty(this, "allowHover", false);
#else
	setStyleSheet("text-align:left;");
	Style::setStyle(m_toolRunBtn, style::properties::button::stopButton);
	Style::setStyle(m_toolRunBtn, style::properties::widget::notInteractive);
	Style::setStyle(this, style::properties::button::toolButton);

	enableDoubleClick(true);
#endif
}

ToolMenuItem::~ToolMenuItem() {}

QPushButton *ToolMenuItem::getToolRunBtn() const { return m_toolRunBtn; }

void ToolMenuItem::enableDoubleClick(bool enable)
{
	if(enable) {
		installEventFilter(this);
	} else {
		removeEventFilter(this);
		removeEventFilter(this);
	}
}

bool ToolMenuItem::eventFilter(QObject *watched, QEvent *event)
{
	if(event->type() == QEvent::MouseButtonDblClick) {
		QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
		if(mouseEvent->button() == Qt::LeftButton) {
			if(isEnabled()) {
				Q_EMIT doubleclick();
				return true;
			}
		}
	}

	return QObject::event(event);
}

void ToolMenuItem::setName(QString str)
{
	m_name = str;
	setText(m_name);
}

void ToolMenuItem::setSelected(bool en) { setDynamicProperty(this, "selected", en); }

void ToolMenuItem::setDisabled(bool disabled) { setDisabled(disabled); }

void ToolMenuItem::updateItem()
{
	ToolMenuEntry *tme = dynamic_cast<ToolMenuEntry *>(QObject::sender());
	Q_ASSERT(tme);
	QSignalBlocker sb(m_toolRunBtn);
	setVisible(tme->visible());
	setEnabled(tme->enabled());
	setName(tme->name());
	m_toolRunBtn->setEnabled(tme->runEnabled());
	m_toolRunBtn->setEnabled(tme->runBtnVisible());
	m_toolRunBtn->setChecked(tme->running());
	qDebug(CAT_TOOLMENUITEM) << "updating toolmenuentry for " << tme->name() << " - " << tme->uuid();
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

QString ToolMenuItem::getId() const { return m_uuid; }

#include "moc_toolmenuitem.cpp"
