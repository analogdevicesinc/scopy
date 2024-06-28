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

#ifndef CUSTOM_SWITCH_HPP
#define CUSTOM_SWITCH_HPP

#include "scopy-gui_export.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QMap>
#include <QPushButton>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT CustomSwitch : public QPushButton
{
	Q_OBJECT

public:
	explicit CustomSwitch(QWidget *parent = nullptr);
//	explicit CustomSwitch(const QString &text, QWidget *parent = nullptr);

//	QSize sizeHint() const override;
//	void setChecked(bool checked);

	void setOnText(const QString text);
	void setOffText(const QString text);

	void setOn(const QPixmap &pixmap);
	void setOff(const QPixmap &pixmap);

protected:
//	void resizeEvent(QResizeEvent *event) override;
	void paintEvent(QPaintEvent *event) override;
//	void mouseReleaseEvent(QMouseEvent *event) override;
//	void enterEvent(QEvent *event) override;
//	int offset() const;
//	void setOffset(int value);

private:
	QLabel *onLabel;
	QLabel *offLabel;
	QHBoxLayout *layout;




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
	QMap<bool, QColor> m_text_color;
	QMap<bool, QString> m_text;
	QMap<bool, std::function<int()>> m_end_offset;
};
} // namespace scopy

#endif /* CUSTOM_SWITCH_HPP */
