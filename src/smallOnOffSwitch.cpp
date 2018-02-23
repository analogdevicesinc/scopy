/*
 * Copyright 2017 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "smallOnOffSwitch.hpp"

#include <QDebug>
#include <QResizeEvent>

using namespace adiscope;

SmallOnOffSwitch::SmallOnOffSwitch(QWidget *parent) : QPushButton(parent),
	color_start("grey"), color_end("blue"),
	handle(this), anim(&handle, "geometry"), color_anim(this, "color")
{
	handle.setObjectName("handle");

	setFlat(true);
	setCheckable(true);
	setDuration(100);

	QFile file(":stylesheets/stylesheets/smallOnOffSwitch.qss");
	file.open(QFile::ReadOnly);
	stylesheet = QString::fromLatin1(file.readAll());
	this->setStyleSheet(stylesheet);

	connect(this, SIGNAL(toggled(bool)), SLOT(toggleAnim(bool)));
}

SmallOnOffSwitch::~SmallOnOffSwitch()
{
}

void SmallOnOffSwitch::setDuration(int ms)
{
	duration_ms = ms;
	anim.setDuration(ms);
	color_anim.setDuration(ms);
}

void SmallOnOffSwitch::setHandleColor(const QColor& color)
{
	QString ss(stylesheet + QString("QWidget#handle { background-color: %1; }")
			.arg(color.name()));
	this->setStyleSheet(ss);
}

void SmallOnOffSwitch::toggleAnim(bool enabled)
{
	if (!isVisible())
		return;

	QRect off_rect(0, handle.y(), handle.width(), handle.height());
	QRect on_rect(width() - handle.width(), handle.y(),
			handle.width(), handle.height());

	anim.stop();
	color_anim.stop();

	if (!enabled) {
		anim.setStartValue(off_rect);
		anim.setEndValue(on_rect);
		color_anim.setStartValue(color_start);
		color_anim.setEndValue(color_end);
	} else {
		anim.setStartValue(on_rect);
		anim.setEndValue(off_rect);
		color_anim.setStartValue(color_end);
		color_anim.setEndValue(color_start);
	}

	anim.start();
	color_anim.start();
}

void SmallOnOffSwitch::showEvent(QShowEvent *event)
{
	if (!isChecked()) {
		handle.setGeometry(QRect(width() - handle.width(), handle.y(),
					handle.width(), handle.height()));
		setHandleColor(color_end);
	} else {
		setHandleColor(color_start);
	}
}
