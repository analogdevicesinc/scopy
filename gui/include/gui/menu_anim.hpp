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

#ifndef MENU_ANIM_HPP
#define MENU_ANIM_HPP

#include "customanimation.h"
#include "scopy-gui_export.h"

#include <QSize>
#include <QWidget>

namespace scopy {
class SCOPY_GUI_EXPORT MenuAnim : public QWidget
{
	Q_OBJECT

public:
	explicit MenuAnim(QByteArray minAnimationProperty, QByteArray maxAnimationProperty, QWidget *parent = nullptr);
	~MenuAnim() {}

	bool animInProgress() const;

	int getAnimationDuration() const;
	void setAnimationDuration(int newAnimationDuration);

	void setAnimMin(int min);
	void setAnimMax(int max);

protected:
	virtual int getImplicitMin() = 0;
	virtual int getImplicitMax() = 0;
Q_SIGNALS:
	void finished(bool opened);

public Q_SLOTS:
	void toggleMenu(bool open);

private Q_SLOTS:
	void closeAnimFinished();
	void openAnimFinished();

private:
	CustomAnimation close_anim_max, close_anim_min, open_anim_max, open_anim_min;
	int min_val;
	int max_val;
	int animationDuration;
	bool animInProg;
};

class SCOPY_GUI_EXPORT MenuVAnim : public MenuAnim
{
	Q_OBJECT
public:
	explicit MenuVAnim(QWidget *parent = nullptr);

protected:
	int getImplicitMin() override;
	int getImplicitMax() override;
};

class SCOPY_GUI_EXPORT MenuHAnim : public MenuAnim
{
	Q_OBJECT
public:
	explicit MenuHAnim(QWidget *parent = nullptr);

protected:
	int getImplicitMin() override;
	int getImplicitMax() override;
};
} // namespace scopy

#endif /* MENU_ANIM_HPP */
