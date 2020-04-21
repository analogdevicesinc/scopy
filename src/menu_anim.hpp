/*
 * Copyright 2016 Analog Devices, Inc.
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

#ifndef MENU_ANIM_HPP
#define MENU_ANIM_HPP

#include "coloredQWidget.hpp"
#include "customanimation.h"

#include <QSize>
#include <QWidget>

namespace adiscope {
class MenuAnim : public ColoredQWidget {
	Q_OBJECT

public:
	explicit MenuAnim(QWidget *parent = nullptr);
	~MenuAnim() {}

	void setMinimumSize(QSize size);
	bool animInProgress() const;

Q_SIGNALS:
	void finished(bool opened);

public Q_SLOTS:
	void toggleMenu(bool open);

private Q_SLOTS:
	void closeAnimFinished();
	void openAnimFinished();

private:
	CustomAnimation close_anim_max, close_anim_min, open_anim_max,
		open_anim_min;
	int min_width;
	bool animInProg;
};
} // namespace adiscope

#endif /* MENU_ANIM_HPP */
