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
#include "channel_widget.hpp"
#include "ui_channel.h"
#include "dynamicWidget.h"

#include <QButtonGroup>
#include <utils.h>

using namespace scopy;

/*
 * Class ChannelWidget - is a composite widget that contains a button with the
 * name of the channel that can be toggle to indicate that the channel has been
 * selected, a checkbox indicating whether the channel is enabled or not, a menu
 * button, a delete button that is visible only for deletable channels and a
 * thin line which is visible only when the channel is selected, thus increasing
 * the visibility of the selected state of the channel.
 */
ChannelWidget::ChannelWidget(int id, bool deletable, bool simplified,
		QColor color, QWidget *parent) :
	QWidget(parent),
	m_ui(new Ui::Channel),
	m_id(id),
	m_deletable(deletable),
	m_simplified(simplified),
	m_color(color),
	m_math(false),
	m_function(""),
	m_ref(false),
	m_isMainChannel(false),
	m_isPhysicalChannel(false)
{
	init();
	nameButton()->installEventFilter(this);
	m_channelWSpacer = new QSpacerItem(1,1,QSizePolicy::Expanding,QSizePolicy::Fixed);
	setStyleSheet("scopy--ChannelWidget QPushButton#btn:hover:!pressed:!checked {		      border-image: url(:/gui/icons/setup_btn_hover.svg)		     }");
}

ChannelWidget::~ChannelWidget()
{
	setButtonNoGroup(m_ui->box);
	setButtonNoGroup(m_ui->name);
	setButtonNoGroup(m_ui->btn);

	delete m_channelWSpacer;
	delete m_ui;
}

void ChannelWidget::init()
{
	m_ui->setupUi(this);
	m_ui->toggleChannels->hide();
	m_ui->toggleChannels->setChecked(true);
	m_ui->toggleChannels->setProperty("subsection_arrow_button", true);
	setId(m_id);
	m_ui->delBtn->setVisible(m_deletable);
	setColor(m_color);

	if (m_simplified) {
		m_ui->name->hide();
		m_ui->line->hide();
		QString stylesheet = styleSheet();
		stylesheet += QString("QCheckBox#box { spacing: 12px; }");
		setStyleSheet(stylesheet);
	} else {
		connect(m_ui->name, &QPushButton::toggled,
			[=] (bool checked) {
				setDynamicProperty(m_ui->widget, "selected",
					checked);
				setDynamicProperty(m_ui->line, "selected",
					checked);
			}
		);
	}
	m_ui->horizontalLayout->setAlignment(Qt::AlignLeft);
	m_ui->horizontalLayout->setAlignment(m_ui->btn,Qt::AlignRight);
	Util::setWidgetNrOfChars(m_ui->name,6,15);
//	m_ui->verticalSpacer->changeSize(0, 0, QSizePolicy::Fixed,
//		QSizePolicy::Fixed);
}

QAbstractButton* ChannelWidget::enableButton() const
{
	return m_ui->box;
}

QAbstractButton* ChannelWidget::nameButton() const
{
	return m_ui->name;
}

QAbstractButton* ChannelWidget::menuButton() const
{
	return m_ui->btn;
}

QAbstractButton* ChannelWidget::deleteButton() const
{
	return m_ui->delBtn;
}

QAbstractButton* ChannelWidget::toggleChannelsButton() const
{
	return m_ui->toggleChannels;
}

int ChannelWidget::id() const
{
	return m_id;
}

void ChannelWidget::setId(int id)
{
	m_ui->box->setProperty("id", QVariant(id));
	m_ui->name->setProperty("id", QVariant(id));
	m_ui->btn->setProperty("id", QVariant(id));
	m_ui->delBtn->setProperty("id", QVariant(id));

	m_id = id;
}

QColor ChannelWidget::color() const
{
	return m_color;
}

void ChannelWidget::setColor(QColor color)
{
	QString boxStyleSheet = QString(
		"QCheckBox#box::indicator {"
		"border: 2px solid %1;"
		"}"
		"QCheckBox#box::indicator:checked {"
		"background-color: %1;"
		"}"
	).arg(color.name());
	m_ui->box->setStyleSheet(boxStyleSheet);

	QString lineStyleSheet = QString(
		"QFrame#line[selected=true] {"
		"border: 2px solid %1;"
		"}"
	).arg(color.name());
	m_ui->line->setStyleSheet(lineStyleSheet);

	m_color = color;
}

QString ChannelWidget::fullName() const
{
	return m_fullName;
}

void ChannelWidget::setFullName(const QString& name)
{
	m_fullName = name;
}

QString ChannelWidget::shortName() const
{
	return m_shortName;
}

void ChannelWidget::setShortName(const QString &name)
{
	m_shortName = name;
}

QString ChannelWidget::function() const
{
	return m_function;
}

void ChannelWidget::setFunction(const QString &function)
{
	m_function = function;
	setProperty("function", QVariant(function));
}

bool ChannelWidget::isMathChannel() const
{
	return m_math;
}

void ChannelWidget::setMathChannel(const bool &math)
{
	m_math = math;
}

bool ChannelWidget::isReferenceChannel() const
{
	return m_ref;
}

void ChannelWidget::setReferenceChannel(const bool &ref)
{
	m_ref = ref;
}

void ChannelWidget::setMenuFloating(bool floating) { m_floatingMenu = floating; }

bool ChannelWidget::eventFilter(QObject *object, QEvent *event)
{
	if (event->type() == QEvent::MouseButtonPress) {
		if (!m_ui->box->isChecked()) {
			m_ui->box->setChecked(true);
			m_ui->name->setChecked(true);
		} else if (!m_ui->btn->isChecked() &&
			   m_ui->name->isChecked()){
			m_ui->btn->setChecked(true);
		}
	} else if (event->type() == QEvent::MouseButtonDblClick) {
		if (!m_ui->box->isChecked()) {
			m_ui->box->setChecked(true);
			m_ui->name->setChecked(true);
		}
		m_ui->btn->setChecked(true);
	}

	return QObject::eventFilter(object, event);
}

void ChannelWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
	if (!m_ui->box->isChecked()) {
		m_ui->box->setChecked(true);
		m_ui->name->setChecked(true);
	}
	m_ui->btn->setChecked(true);

}

void ChannelWidget::on_box_toggled(bool checked)
{
	if (checked) {
		if (!m_simplified)
			m_ui->name->setEnabled(true);
		m_ui->btn->setEnabled(true);
		// When enabling ChannelWidget select it as well
		if (!m_simplified)
			m_ui->name->setChecked(true);

	} else {
		// Unselect the ChannelWidget when it's about to be disabled
		if (!m_simplified) {
			QButtonGroup *group = m_ui->name->group();
			bool exclusive;
			if (group) {
				exclusive = group->exclusive();
				if (m_ui->name->isChecked()) {
					group->setExclusive(false);
				}
			}
			m_ui->name->setChecked(false);
			if (group) {
				group->setExclusive(exclusive);
			}
		}

		// Uncheck the menu button when disabling the ChannelWidget
		m_ui->btn->setChecked(false);

		if (!m_simplified)
			m_ui->name->setEnabled(false);

		m_ui->btn->setEnabled(false);
	}

	Q_EMIT enabled(checked);
}

void ChannelWidget::on_name_toggled(bool checked)
{
	Q_EMIT selected(checked);
}

void ChannelWidget::on_btn_toggled(bool checked)
{
	Q_EMIT menuToggled(checked);
}

void ChannelWidget::on_delBtn_clicked()
{
	setButtonNoGroup(m_ui->box);
	setButtonNoGroup(m_ui->name);
	setButtonNoGroup(m_ui->btn);

	Q_EMIT deleteClicked();
}

bool ChannelWidget::isMainChannel() const
{
	return m_isMainChannel;
}

bool ChannelWidget::isPhysicalChannel() const
{
	return m_isPhysicalChannel;
}

void ChannelWidget::setButtonNoGroup(QAbstractButton *btn)
{
	QButtonGroup *group = btn->group();
	if (group) {
		group->removeButton(btn);
	}
}

void ChannelWidget::setMenuButtonVisibility(bool visible)
{
	m_ui->btn->setVisible(visible);
}

void ChannelWidget::setBottomLineVIsibility(bool visible)
{
	m_ui->line->setVisible(visible);
}

void ChannelWidget::toggleChannel(bool toggled)
{
	m_ui->name->setVisible(!toggled);
	m_ui->delBtn->setVisible(!toggled && m_deletable);
}

void ChannelWidget::setIsMainChannel(bool mainChannel)
{
	if (mainChannel) {
		m_isMainChannel = true;
		m_ui->box->hide();
		m_ui->toggleChannels->show();
		connect(m_ui->toggleChannels, &QPushButton::toggled, this, [=](bool toggled){
			Q_EMIT enabled(toggled);
		});
	}
}

void ChannelWidget::setIsPhysicalChannel(bool physChannel)
{
	m_isPhysicalChannel = physChannel;
}
