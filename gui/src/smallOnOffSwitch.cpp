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
#include "style.h"
#include <QDebug>
#include <QFile>
#include <QResizeEvent>
#include <QStylePainter>

using namespace scopy;

SmallOnOffSwitch::SmallOnOffSwitch(QWidget *parent)
	: QPushButton(parent)
{
	setCheckable(true);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_track_radius = Style::getAttribute(json::global::unit_int_1).toInt();
	m_thumb_radius = Style::getAttribute(json::global::unit_int_1).toInt() - 2;
	m_margin = std::max(0, m_thumb_radius - m_track_radius);
	m_base_offset = std::max(m_thumb_radius, m_track_radius);
	m_end_offset[true] = [this]() { return width() - m_base_offset; };
	m_end_offset[false] = [this]() { return m_base_offset; };
	m_offset = m_base_offset;

	m_track_color[true] = Style::getColor(json::theme::color_highlight);
	m_track_color[false] = Style::getColor(json::theme::highlight_disabled);
	m_thumb_color[true] = Style::getColor(json::theme::focus_item);
	m_thumb_color[false] = Style::getColor(json::theme::focus_item);
	m_text_color[true] = Style::getColor(json::theme::highlight);
	m_text_color[false] = Style::getColor(json::theme::highlight);
	m_track_opacity = 1.0;
}

int SmallOnOffSwitch::offset() const { return m_offset; }

void SmallOnOffSwitch::setOffset(int value)
{
	m_offset = value;
	update();
}

QSize SmallOnOffSwitch::sizeHint() const
{
	return QSize(4 * m_track_radius + 2 * m_margin, 2 * m_track_radius + 2 * m_margin);
}

void SmallOnOffSwitch::setChecked(bool checked)
{
	QAbstractButton::setChecked(checked);
	setOffset(m_end_offset[checked]());
}

void SmallOnOffSwitch::resizeEvent(QResizeEvent *event)
{
	QAbstractButton::resizeEvent(event);
	setOffset(m_end_offset[isChecked()]());
}

void SmallOnOffSwitch::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::NoPen);

	qreal track_opacity = m_track_opacity;
	qreal thumb_opacity = 1.0;
	qreal text_opacity = 1.0;

	QColor track_brush, thumb_brush, text_color;
	if(isEnabled()) {
		track_brush = m_track_color[isChecked()];
		thumb_brush = m_thumb_color[isChecked()];
		text_color = m_text_color[isChecked()];
	} else {
		track_opacity *= 0.8;
		track_brush = palette().shadow().color();
		thumb_brush = palette().mid().color();
		text_color = palette().shadow().color();
	}

	p.setBrush(track_brush);
	p.setOpacity(track_opacity);
	p.drawRoundedRect(m_margin, m_margin, width() - 2 * m_margin, height() - 2 * m_margin, m_track_radius,
			  m_track_radius);

	p.setBrush(thumb_brush);
	p.setOpacity(thumb_opacity);
	p.drawEllipse(m_offset - m_thumb_radius, m_base_offset - m_thumb_radius, 2 * m_thumb_radius,
		      2 * m_thumb_radius);

	p.setPen(text_color);
	p.setOpacity(text_opacity);
	QFont font = p.font();
	font.setPixelSize(static_cast<int>(1.5 * m_thumb_radius));
	p.setFont(font);
	//	p.drawText(QRectF(m_offset - m_thumb_radius, m_base_offset - m_thumb_radius, 2 * m_thumb_radius,
	//			  2 * m_thumb_radius),
	//		   Qt::AlignCenter);
}

void SmallOnOffSwitch::mouseReleaseEvent(QMouseEvent *event)
{
	QAbstractButton::mouseReleaseEvent(event);
	if(event->button() == Qt::LeftButton) {
		auto *anim = new QPropertyAnimation(this, "offset");
		anim->setDuration(120);
		anim->setStartValue(m_offset);
		anim->setEndValue(m_end_offset[isChecked()]());
		anim->start(QAbstractAnimation::DeleteWhenStopped);
	}
}

void SmallOnOffSwitch::enterEvent(QEvent *event)
{
	setCursor(Qt::PointingHandCursor);
	QAbstractButton::enterEvent(event);
}

#include "moc_smallOnOffSwitch.cpp"
