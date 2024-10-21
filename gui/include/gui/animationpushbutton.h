/*
 * Copyright (c) 2024 Analog Devices Inc.
 *
 * This file is part of Scopy
 * (see https://www.github.com/analogdevicesinc/scopy).
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
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 */

#ifndef ANIMATIONPUSHBUTTON_H
#define ANIMATIONPUSHBUTTON_H

#include "scopy-gui_export.h"

#include <QMovie>
#include <QPushButton>

namespace scopy {
class SCOPY_GUI_EXPORT AnimationPushButton : public QPushButton
{
	Q_OBJECT
public:
	explicit AnimationPushButton(QWidget *parent = Q_NULLPTR);
	~AnimationPushButton();

	void setAnimation(QMovie *animation, int maxRunningTimeMsec = 10000);

public Q_SLOTS:
	void startAnimation();
	void stopAnimation();

private Q_SLOTS:
	void setBtnIcon();

private:
	QTimer *m_timer;
	QMovie *m_animation;
	QString m_currentText;
	QIcon m_currentIcon;
};
} // namespace scopy

#endif // ANIMATIONPUSHBUTTON_H
