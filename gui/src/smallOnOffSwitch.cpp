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
#include <QStyleOptionButton>
#include <QStylePainter>

using namespace scopy;

SmallOnOffSwitch::SmallOnOffSwitch(QWidget *parent)
	: QCheckBox(parent)
{
	setCheckable(true);
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	m_track_radius = Style::getAttribute(json::global::unit_int_1).toInt() / 2;
	m_thumb_radius = Style::getAttribute(json::global::unit_int_1).toInt() / 2 - 1;
	m_btn_width = Style::getAttribute(json::global::unit_int_1).toInt() * 2.2;
	m_spacing = Style::getAttribute(json::global::unit_int_1).toInt() / 2;
	m_margin = std::max(0, m_thumb_radius - m_track_radius);
	m_base_offset = std::max(m_thumb_radius, m_track_radius);
	m_end_offset[true] = [this]() { return m_btn_width - m_base_offset; };
	m_end_offset[false] = [this]() { return m_base_offset; };
	m_offset = m_base_offset;

	m_track_color[true] = Style::getColor(json::theme::color_highlight);
	m_track_color[false] = Style::getColor(json::theme::highlight_disabled);
	m_thumb_color[true] = Style::getColor(json::theme::focus_item);
	m_thumb_color[false] = Style::getColor(json::theme::focus_item);
	m_track_opacity = 1.0;
}

SmallOnOffSwitch::SmallOnOffSwitch(const QString &text, QWidget *parent)
	: SmallOnOffSwitch(parent)
{
	QCheckBox::setText(text);
}

int SmallOnOffSwitch::offset() const { return m_offset; }

void SmallOnOffSwitch::setOffset(int value)
{
	m_offset = value;
	update();
}

QSize SmallOnOffSwitch::sizeHint() const
{
	QSize size;
	if(QCheckBox::text().isEmpty()) {
		size = QSize(m_btn_width, 2 * m_track_radius);
	} else {
		size = QCheckBox::sizeHint();
		size.rwidth() += m_btn_width + m_spacing;
	}

	return size;
}

void SmallOnOffSwitch::setChecked(bool checked)
{
	QCheckBox::setChecked(checked);
	setOffset(m_end_offset[checked]());
}

void SmallOnOffSwitch::resizeEvent(QResizeEvent *event)
{
	QCheckBox::resizeEvent(event);
	setOffset(m_end_offset[isChecked()]());
}

void SmallOnOffSwitch::paintEvent(QPaintEvent *event)
{
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::NoPen);

	qreal track_opacity = m_track_opacity;
	qreal thumb_opacity = 1.0;

	QColor track_brush, thumb_brush;
	if(isEnabled()) {
		track_brush = m_track_color[isChecked()];
		thumb_brush = m_thumb_color[isChecked()];
	} else {
		track_opacity *= 0.8;
		track_brush = palette().shadow().color();
		thumb_brush = palette().mid().color();
	}

	p.setBrush(track_brush);
	p.setOpacity(track_opacity);
	p.drawRoundedRect(m_margin, m_margin, m_btn_width - 2 * m_margin, m_track_radius * 2 - 2 * m_margin, m_track_radius,
			  m_track_radius);

	p.setBrush(thumb_brush);
	p.setOpacity(thumb_opacity);
	p.drawEllipse(m_offset - m_thumb_radius, m_base_offset - m_thumb_radius, 2 * m_thumb_radius,
		      2 * m_thumb_radius);

	if(!QCheckBox::text().isEmpty()) {
		QStylePainter sp(this);
		QStyleOptionButton opt;
		initStyleOption(&opt);
		opt.rect.setLeft(opt.rect.left() + m_btn_width + m_spacing);
		opt.rect.setTop(opt.rect.top() - (opt.rect.height() - m_track_radius * 2));
		sp.drawControl(QStyle::CE_CheckBoxLabel, opt);
	}
}

void SmallOnOffSwitch::mouseReleaseEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton && rect().contains(event->pos())) {
		QCheckBox::toggle();
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
	QCheckBox::enterEvent(event);
}

#include "moc_smallOnOffSwitch.cpp"
