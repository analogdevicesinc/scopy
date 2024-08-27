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
	initDimensions();

	connect(this, &QCheckBox::toggled, this, &SmallOnOffSwitch::toggleAnim);
}

SmallOnOffSwitch::SmallOnOffSwitch(const QString &text, QWidget *parent)
	: SmallOnOffSwitch(parent)
{
	QCheckBox::setText(text);
}

SmallOnOffSwitch::~SmallOnOffSwitch() {}

void SmallOnOffSwitch::initDimensions()
{
	m_is_entered = false;
	m_track_radius = Style::getDimension(json::global::unit_1) / 2;
	m_thumb_radius = Style::getDimension(json::global::unit_1) / 2 - 1;
	m_btn_width = Style::getDimension(json::global::unit_1) * 2.2;
	m_spacing = Style::getDimension(json::global::unit_1) / 2;
	m_margin = std::max(0, m_thumb_radius - m_track_radius);
	m_base_offset = std::max(m_thumb_radius, m_track_radius);
	m_end_offset[true] = [this]() { return m_btn_width - m_base_offset; };
	m_end_offset[false] = [this]() { return m_base_offset; };
	m_offset = m_base_offset;

	m_track_color[true] = Style::getColor(json::theme::interactive_primary_idle);
	m_track_color[false] = Style::getColor(json::theme::interactive_subtle_idle);
	m_track_color_disabled[true] = Style::getColor(json::theme::interactive_primary_disabled);
	m_track_color_disabled[false] = Style::getColor(json::theme::interactive_subtle_disabled);
	m_thumb_color[true] = Style::getColor(json::theme::content_inverse);
	m_thumb_color[false] = Style::getColor(json::theme::content_inverse);
	m_track_opacity = 1.0;
}

int SmallOnOffSwitch::offset() const { return m_offset; }

void SmallOnOffSwitch::setSpacing(int spacing)
{
	m_spacing = spacing;
	update();
}

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

void SmallOnOffSwitch::toggleAnim()
{
	auto *anim = new QPropertyAnimation(this, "offset");
	anim->setDuration(120);
	anim->setStartValue(m_offset);
	anim->setEndValue(m_end_offset[isChecked()]());
	anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void SmallOnOffSwitch::resizeEvent(QResizeEvent *event)
{
	QCheckBox::resizeEvent(event);
	setOffset(m_end_offset[isChecked()]());
}

void SmallOnOffSwitch::paintEvent(QPaintEvent *event)
{
	update();
	QPainter p(this);
	p.setRenderHint(QPainter::Antialiasing);
	p.setPen(Qt::NoPen);

	QColor track_brush, thumb_brush;
	track_brush = m_track_color[isChecked()];
	thumb_brush = m_thumb_color[isChecked()];

	if(!isEnabled()) {
		track_brush = m_track_color_disabled[isChecked()];
	}

	p.setBrush(track_brush);
	int track_radius = !getDynamicProperty(this, "use_icon") ? m_track_radius : m_track_radius / 2;
	p.drawRoundedRect(m_margin, m_margin, m_btn_width - 2 * m_margin, m_track_radius * 2 - 2 * m_margin,
			  track_radius, track_radius);

	// use icon (this is used in m2k)
	if(getDynamicProperty(this, "use_icon")) {
		QPixmap pixmap;
		if(isChecked()) {
			pixmap = Style::getPixmap(":/gui/icons/unlocked.svg",
						  Style::getColor(json::theme::content_inverse));
		} else {
			pixmap = Style::getPixmap(":/gui/icons/locked.svg",
						  Style::getColor(json::theme::content_inverse));
		}

		//		p.drawPixmap(QRect(m_offset - m_thumb_radius, m_base_offset - m_thumb_radius, 2 *
		//m_thumb_radius, 2 * m_thumb_radius), pixmap);
		p.drawPixmap(QRect(m_offset - m_thumb_radius, m_base_offset - m_thumb_radius, pixmap.width(),
				   pixmap.height()),
			     pixmap);
	} else {
		p.setBrush(thumb_brush);
		p.drawEllipse(m_offset - m_thumb_radius, m_base_offset - m_thumb_radius, 2 * m_thumb_radius,
			      2 * m_thumb_radius);
	}

	if(!QCheckBox::text().isEmpty()) {
		QStylePainter sp(this);
		QStyleOptionButton opt;
		initStyleOption(&opt);
		opt.rect.setLeft(opt.rect.left() + m_btn_width + m_spacing);
		opt.rect.setTop(opt.rect.top() - (opt.rect.height() - m_track_radius * 2));
		sp.drawControl(QStyle::CE_CheckBoxLabel, opt);
	}
}

void SmallOnOffSwitch::enterEvent(QEvent *event)
{
	setCursor(Qt::PointingHandCursor);
	m_is_entered = true;
	QCheckBox::enterEvent(event);
}

void SmallOnOffSwitch::leaveEvent(QEvent *event)
{
	m_is_entered = false;
	QCheckBox::enterEvent(event);
}

void SmallOnOffSwitch::mousePressEvent(QMouseEvent *event)
{
	if(event->button() == Qt::LeftButton && m_is_entered) {
		toggle();
	}
}

#include "moc_smallOnOffSwitch.cpp"
