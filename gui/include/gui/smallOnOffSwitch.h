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

#ifndef SMALL_ON_OFF_SWITCH_HPP
#define SMALL_ON_OFF_SWITCH_HPP

#include "scopy-gui_export.h"

#include <QCheckBox>
#include <QColor>
#include <QMap>
#include <QPainter>
#include <QPropertyAnimation>
#include <QWidget>

class QShowEvent;

namespace scopy {
class SCOPY_GUI_EXPORT SmallOnOffSwitch : public QCheckBox
{
	Q_OBJECT
	Q_PROPERTY(int offset READ offset WRITE setOffset)

public:
	explicit SmallOnOffSwitch(QWidget *parent = nullptr);
	explicit SmallOnOffSwitch(const QString &text, QWidget *parent = nullptr);

	QSize sizeHint() const override;
	void setChecked(bool checked);

protected:
	void resizeEvent(QResizeEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
	void mouseReleaseEvent(QMouseEvent *event) override;
	void enterEvent(QEvent *event) override;
	int offset() const;
	void setOffset(int value);

private:
	int m_spacing;
	int m_btn_width;
	int m_track_radius;
	int m_thumb_radius;
	int m_margin;
	int m_base_offset;
	int m_offset;
	qreal m_track_opacity;

	QMap<bool, QColor> m_track_color;
	QMap<bool, QColor> m_thumb_color;
	QMap<bool, std::function<int()>> m_end_offset;
};
} // namespace scopy

#endif /* SMALL_ON_OFF_SWITCH_HPP */
