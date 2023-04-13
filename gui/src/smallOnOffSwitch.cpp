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

#include "smallOnOffSwitch.h"
#include "dynamicWidget.h"

#include <QDebug>
#include <QResizeEvent>
#include <QStylePainter>
#include <QFile>

using namespace scopy;

SmallOnOffSwitch::SmallOnOffSwitch(QWidget *parent) : QPushButton(parent),
	color_start("grey"), color_end("blue"), on(this), off(this),
	handle(this), anim(&handle, "geometry"), color_anim(this, "color"),
	show_icon(false),bothValid(false)
{
	handle.setObjectName("handle");
	on.setObjectName("on");
	off.setObjectName("off");

	setFlat(true);
	setCheckable(true);
	setDuration(100);

	QFile file(":/gui/stylesheets/smallOnOffSwitch.qss");
	file.open(QFile::ReadOnly);
	stylesheet = QString::fromLatin1(file.readAll());
	this->setStyleSheet(stylesheet);

	on.raise();
	off.raise();
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
		if(bothValid)
			color_anim.setEndValue(color_start);
		else
			color_anim.setEndValue(color_end);
	} else {
		anim.setStartValue(on_rect);
		anim.setEndValue(off_rect);
		if(bothValid)
			color_anim.setStartValue(color_start);
		else
			color_anim.setStartValue(color_end);

		color_anim.setEndValue(color_start);
	}

	anim.start();
	color_anim.start();
}

bool SmallOnOffSwitch::event(QEvent* e)
{
	if (e->type() == QEvent::DynamicPropertyChange)
	{
		QDynamicPropertyChangeEvent *const propEvent = static_cast<QDynamicPropertyChangeEvent*>(e);
		QString propName = propEvent->propertyName();
		if(propName=="leftText" && property("leftText").isValid())
			on.setText(property("leftText").toString());
		if(propName=="rightText" && property("rightText").isValid())
			off.setText(property("rightText").toString());
		if(propName=="bothValid" && property("bothValid").isValid())
			bothValid = property("bothValid").toBool();
		if(propName=="duration" && property("duration").isValid())
			setDuration(property("duration").toInt());


	}
	return QPushButton::event(e);
}
void SmallOnOffSwitch::paintEvent(QPaintEvent *e)
{
        QPushButton::paintEvent(e);
        show_icon = getDynamicProperty(this, "use_icon");

	if (!show_icon) {
                return;
        }

        QIcon locked = QIcon::fromTheme("locked");
        QIcon unlocked = QIcon::fromTheme("unlocked");
        QPixmap pixmap;

        QStylePainter p(this);
        int w, h;
        int left = 4, top = 4;

        if (isChecked()) {
                w = 8; h = 12;
                pixmap =  locked.pixmap(w, h);
                p.drawPixmap(left + handle.x() + handle.width(),
                             handle.y() + top, w, h, pixmap);
        } else {
                w = 10; h = 12;
                pixmap = unlocked.pixmap(w, h);
                p.drawPixmap(left, handle.y() + top, w, h, pixmap);
        }
}

void SmallOnOffSwitch::showEvent(QShowEvent *event)
{
	if (!isChecked()) {
		handle.setGeometry(QRect(width() - handle.width(), handle.y(),
					handle.width(), handle.height()));		
		if(bothValid)
		{
			setHandleColor(color_start);
		}
		else
		{
			setHandleColor(color_end);
		}
	} else {
		setHandleColor(color_start);
		handle.setGeometry(0, handle.y(), handle.width(), handle.height());
	}

}

void SmallOnOffSwitch::updateOnOffLabels()
{
	if(!bothValid)
	{
		on.setEnabled(isChecked());
		off.setEnabled(!isChecked());
	}
}

#include "moc_smallOnOffSwitch.cpp"
