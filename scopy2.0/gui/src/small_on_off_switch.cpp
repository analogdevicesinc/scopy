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

#include <QDebug>
#include <QResizeEvent>
#include <QStylePainter>

#include <scopy/gui/small_on_off_switch.hpp>

using namespace scopy::gui;

SmallOnOffSwitch::SmallOnOffSwitch(QWidget* parent)
	: QPushButton(parent)
	, m_colorStart("grey")
	, m_colorEnd("blue")
	, m_on(this)
	, m_off(this)
	, m_handle(this)
	, m_anim(&m_handle, "geometry")
	, m_colorAnim(this, "color")
	, m_showIcon(false)
	, m_bothValid(false)
{
	m_handle.setObjectName("handle");
	m_on.setObjectName("on");
	m_off.setObjectName("off");

	setFlat(true);
	setCheckable(true);
	setDuration(100);

	QFile file(":/stylesheets/small_on_off_switch.qss");
	file.open(QFile::ReadOnly);
	m_stylesheet = QString::fromLatin1(file.readAll());
	this->setStyleSheet(m_stylesheet);

	m_on.raise();
	m_off.raise();
	connect(this, SIGNAL(toggled(bool)), SLOT(toggleAnim(bool)));
}

SmallOnOffSwitch::~SmallOnOffSwitch() {}

void SmallOnOffSwitch::setDuration(int ms)
{
	m_durationMs = ms;
	m_anim.setDuration(ms);
	m_colorAnim.setDuration(ms);
}

void SmallOnOffSwitch::setHandleColor(const QColor& color)
{
	QString ss(m_stylesheet + QString("QWidget#handle { background-color: %1; }").arg(color.name()));
	this->setStyleSheet(ss);
}

void SmallOnOffSwitch::toggleAnim(bool enabled)
{
	if (!isVisible())
		return;

	QRect off_rect(0, m_handle.y(), m_handle.width(), m_handle.height());
	QRect on_rect(width() - m_handle.width(), m_handle.y(), m_handle.width(), m_handle.height());

	m_anim.stop();
	m_colorAnim.stop();

	if (!enabled) {
		m_anim.setStartValue(off_rect);
		m_anim.setEndValue(on_rect);
		m_colorAnim.setStartValue(m_colorStart);
		if (m_bothValid)
			m_colorAnim.setEndValue(m_colorStart);
		else
			m_colorAnim.setEndValue(m_colorEnd);
	} else {
		m_anim.setStartValue(on_rect);
		m_anim.setEndValue(off_rect);
		if (m_bothValid)
			m_colorAnim.setStartValue(m_colorStart);
		else
			m_colorAnim.setStartValue(m_colorEnd);

		m_colorAnim.setEndValue(m_colorStart);
	}

	m_anim.start();
	m_colorAnim.start();
}

bool SmallOnOffSwitch::event(QEvent* e)
{
	if (e->type() == QEvent::DynamicPropertyChange) {
		QDynamicPropertyChangeEvent* const propEvent = static_cast<QDynamicPropertyChangeEvent*>(e);
		QString propName = propEvent->propertyName();
		if (propName == "leftText" && property("leftText").isValid())
			m_on.setText(property("leftText").toString());
		if (propName == "rightText" && property("rightText").isValid())
			m_off.setText(property("rightText").toString());
		if (propName == "bothValid" && property("bothValid").isValid())
			m_bothValid = property("bothValid").toBool();
		if (propName == "duration" && property("duration").isValid())
			setDuration(property("duration").toInt());
	}
	return QPushButton::event(e);
}
void SmallOnOffSwitch::paintEvent(QPaintEvent* e)
{
	QPushButton::paintEvent(e);
	m_showIcon = DynamicWidget::getDynamicProperty(this, "use_icon");

	if (!m_showIcon) {
		return;
	}

	QIcon locked(":/icons/ic-locked.svg");
	QIcon unlocked(":/icons/ic-unlocked.svg");
	QPixmap pixmap;

	QStylePainter p(this);
	int w, h;
	int left = 4, top = 4;

	if (isChecked()) {
		w = 8;
		h = 12;
		pixmap = locked.pixmap(w, h);
		p.drawPixmap(left + m_handle.x() + m_handle.width(), m_handle.y() + top, w, h, pixmap);
	} else {
		w = 10;
		h = 12;
		pixmap = unlocked.pixmap(w, h);
		p.drawPixmap(left, m_handle.y() + top, w, h, pixmap);
	}
}

void SmallOnOffSwitch::showEvent(QShowEvent* event)
{
	if (!isChecked()) {
		m_handle.setGeometry(
			QRect(width() - m_handle.width(), m_handle.y(), m_handle.width(), m_handle.height()));
		if (m_bothValid) {
			setHandleColor(m_colorStart);
		} else {
			setHandleColor(m_colorEnd);
		}
	} else {
		setHandleColor(m_colorStart);
		m_handle.setGeometry(0, m_handle.y(), m_handle.width(), m_handle.height());
	}
}

void SmallOnOffSwitch::updateOnOffLabels()
{
	if (!m_bothValid) {
		m_on.setEnabled(isChecked());
		m_off.setEnabled(!isChecked());
	}
}
