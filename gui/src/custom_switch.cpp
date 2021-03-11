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

#include <QDebug>
#include <QResizeEvent>

#include <scopy/gui/custom_switch.hpp>

using namespace scopy::gui;

CustomSwitch::CustomSwitch(QWidget* parent)
	: QPushButton(parent)
	, m_on(this)
	, m_off(this)
	, m_handle(this)
	, m_anim(&m_handle, "geometry")
	, m_polarity(false)
{
	m_on.setObjectName("on");
	m_off.setObjectName("off");
	m_handle.setObjectName("handle");

	setFlat(true);
	setCheckable(true);
	setDuration(100);

	QFile file(":/stylesheets/custom_switch.qss");
	file.open(QFile::ReadOnly);
	QString styleSheet = QString::fromLatin1(file.readAll());
	this->setStyleSheet(styleSheet);

	connect(this, SIGNAL(toggled(bool)), SLOT(toggleAnim(bool)));
	m_on.raise();
	m_off.raise();
	m_on.setText(tr("on"));
	m_off.setText(tr("off"));
	updateOnOffLabels();
}

void CustomSwitch::updateOnOffLabels()
{
	m_on.setEnabled(isChecked() ^ m_polarity);
	m_off.setEnabled(!isChecked() ^ m_polarity);
}

bool CustomSwitch::event(QEvent* e)
{
	if (e->type() == QEvent::DynamicPropertyChange) {
		QDynamicPropertyChangeEvent* const propEvent = static_cast<QDynamicPropertyChangeEvent*>(e);
		QString propName = propEvent->propertyName();
		if (propName == "leftText" && property("leftText").isValid())
			m_on.setText(property("leftText").toString());
		if (propName == "rightText" && property("rightText").isValid())
			m_off.setText(property("rightText").toString());
		if (propName == "polarity" && property("polarity").isValid())
			m_polarity = property("polarity").toBool();
		if (propName == "duration" && property("duration").isValid())
			setDuration(property("duration").toInt());
		if (propName == "bigBtn" && property("bigBtn").isValid()) {
			if (property("bigBtn").toBool()) {
				QFile file(":/stylesheets/big_custom_switch.qss");
				file.open(QFile::ReadOnly);
				QString styleSheet = QString::fromLatin1(file.readAll());
				this->setStyleSheet(styleSheet);
			}
		}
	}
	return QPushButton::event(e);
}

CustomSwitch::~CustomSwitch() {}

void CustomSwitch::setDuration(int ms)
{
	m_durationMs = ms;
	m_anim.setDuration(ms);
}

void CustomSwitch::toggleAnim(bool enabled)
{
	if (!isVisible())
		return;

	QRect on_rect(0, m_handle.y(), m_handle.width(), m_handle.height());
	QRect off_rect(width() - m_handle.width(), m_handle.y(), m_handle.width(), m_handle.height());

	m_anim.stop();

	if (enabled ^ m_polarity) {
		m_anim.setStartValue(off_rect);
		m_anim.setEndValue(on_rect);
	} else {
		m_anim.setStartValue(on_rect);
		m_anim.setEndValue(off_rect);
	}

	updateOnOffLabels();
	m_anim.start();
}

void CustomSwitch::showEvent(QShowEvent* event)
{
	updateOnOffLabels();
	if (isChecked() ^ m_polarity) {
		m_handle.setGeometry(QRect(0, m_handle.y(), m_handle.width(), m_handle.height()));
	} else {
		m_handle.setGeometry(
			QRect(width() - m_handle.width(), m_handle.y(), m_handle.width(), m_handle.height()));
	}
}
