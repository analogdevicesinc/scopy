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

#include "customSwitch.h"
#include "qevent.h"
#include "qpainterpath.h"
#include "qstyleoption.h"

#include <QDebug>
#include <QFile>
#include <QPropertyAnimation>
#include <style.h>
#include <QPen>
#include <QPainter>

using namespace scopy;

CustomSwitch::CustomSwitch(QWidget *parent)
	: QPushButton(parent)
	, onLabel(new QLabel("On", this))
	, offLabel(new QLabel("Off", this))
	, layout(new QHBoxLayout(this))
{
	setCheckable(true);
	setObjectName("toggleButton");

	onLabel->setAlignment(Qt::AlignCenter);
	offLabel->setAlignment(Qt::AlignCenter);

	layout->addWidget(offLabel);
	layout->addWidget(onLabel);
	layout->setContentsMargins(0, 0, 0, 0);
	layout->setSpacing(0);

	setLayout(layout);

	connect(this, &QPushButton::toggled, this, [this](bool checked) { update(); });

	// Initialize the state to unchecked
	setChecked(false);
	setMinimumWidth(80);
	setMinimumHeight(40);
}

void CustomSwitch::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	QPen pen = QPen();
	pen.setBrush(QColor(Qt::transparent));
	pen.setColor(Style::getColor(json::theme::color_highlight));
	pen.setWidth(2);
	p.setPen(pen);

	// Position of shape
	qreal x = 0;
	qreal y = 0;
	// Size of shape
	qreal width = this->width() / 2;
	qreal height = this->height();
	// Radius of corners
	qreal corner_radius = 8;

	QPainterPath path = QPainterPath();
	path.moveTo(x + corner_radius, y);
	path.arcTo(x, y, 2 * corner_radius, 2 * corner_radius, 90.0, 90.0);
	path.lineTo(x, y + (height - 2 * corner_radius));
	path.arcTo(x, y + (height - 2 * corner_radius), 2 * corner_radius, 2 * corner_radius, 180.0, 90.0);
	path.lineTo(x + (width - 2 * corner_radius), y + height);
	path.lineTo(x + (width - 2 * corner_radius), y);
	path.lineTo(x + corner_radius, y);
	p.drawPath(path);

	p.translate(0,this->height());
	p.scale(1.0, -1.0);
	pen.setColor(Style::getColor(json::theme::highlight));
	p.drawPath(path);
}

// int CustomSwitch::offset() const { return m_offset; }

// void CustomSwitch::setOffset(int value)
//{
//	m_offset = value;
//	update();
//}

// QSize CustomSwitch::sizeHint() const
//{
//	return QSize(m_btn_width * 4, 2 * m_track_radius);
//}

// void CustomSwitch::setChecked(bool checked)
//{
//	QPushButton::setChecked(checked);
//	setOffset(m_end_offset[checked]());
//}

void CustomSwitch::setOnText(QString text) { m_text[true] = text; }

void CustomSwitch::setOffText(QString text) { m_text[false] = text; }

void CustomSwitch::setOn(const QPixmap &pixmap) {}

void CustomSwitch::setOff(const QPixmap &pixmap) {}

// void CustomSwitch::resizeEvent(QResizeEvent *event)
//{
//	QPushButton::resizeEvent(event);
//	setOffset(m_end_offset[isChecked()]());
//}

// void CustomSwitch::paintEvent(QPaintEvent *event)
//{
//	Qp p(this);
//	p.setRenderHint(Qp::Antialiasing);
//	p.setPen(Qt::NoPen);

//	qreal track_opacity = m_track_opacity;
//	qreal thumb_opacity = 1.0;
//	qreal text_opacity = 1.0;

//	QColor track_brush, thumb_brush, text_color;
//	if(isEnabled()) {
//		track_brush = m_track_color[isChecked()];
//		thumb_brush = m_thumb_color[isChecked()];
//		text_color = m_text_color[isChecked()];
//	} else {
//		track_opacity *= 0.8;
//		track_brush = palette().shadow().color();
//		thumb_brush = palette().mid().color();
//		text_color = palette().shadow().color();
//	}

//	p.setBrush(track_brush);
//	p.setOpacity(track_opacity);
////	QpPath path;
////	path.addRoundedRect(m_margin, m_margin, m_btn_width - 2 * m_margin, m_track_radius * 2 - 2 * m_margin,
/// m_track_radius, m_track_radius); /	p.drawPath(path); /	p.fillPath(path, Qt::transparent); /
/// p.drawRoundedRect(m_margin, m_margin, m_btn_width - 2 * m_margin, m_track_radius * 2 - 2 * m_margin, m_track_radius,
////			  m_track_radius);

//	// Define colors
//	QColor offColor = thumb_brush; // Light gray color
//	QColor onColor = track_brush;  // Light blue color

//	// Get widget size
//	int width = size().width();
//	int height = size().height();

//	// Draw "Off" rectangle (left side)
//	p.setPen(QColor(160, 160, 160)); // Gray border color
//	p.setBrush(offColor);
//	p.drawRect(0, 0, width / 2, height);

//	// Draw "On" rectangle (right side)
//	p.setPen(QColor(100, 100, 255)); // Blue border color
//	p.setBrush(onColor);
//	p.drawRect(width / 2, 0, width / 2, height);

//	p.setBrush(thumb_brush);
//	p.setOpacity(thumb_opacity);
//	p.drawEllipse(m_offset - m_thumb_radius, m_base_offset - m_thumb_radius, 2 * m_thumb_radius,
//		      2 * m_thumb_radius);

//	p.drawText(QRectF(m_offset - m_thumb_radius, m_base_offset - m_thumb_radius, 2 * m_thumb_radius, 2 *
// m_thumb_radius), Qt::AlignCenter, m_text[isChecked()]);
//}

// void CustomSwitch::mouseReleaseEvent(QMouseEvent *event)
//{
//	if(event->button() == Qt::LeftButton && rect().contains(event->pos())) {
//		QPushButton::toggle();
//		auto *anim = new QPropertyAnimation(this, "offset");
//		anim->setDuration(120);
//		anim->setStartValue(m_offset);
//		anim->setEndValue(m_end_offset[isChecked()]());
//		anim->start(QAbstractAnimation::DeleteWhenStopped);
//	}
//}

// void CustomSwitch::enterEvent(QEvent *event)
//{
//	setCursor(Qt::PointingHandCursor);
//	QPushButton::enterEvent(event);
//}

#include "moc_customSwitch.cpp"
