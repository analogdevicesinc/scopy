/*
 * Copyright (c) 2025 Analog Devices Inc.
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
 */

#ifndef SCOPYSPLASHSCREEN_H
#define SCOPYSPLASHSCREEN_H

#include <QSplashScreen>
#include <QObject>
#include <scopy-gui_export.h>

namespace scopy {
class SCOPY_GUI_EXPORT ScopySplashscreen : public QObject
{
	Q_OBJECT
protected:
	explicit ScopySplashscreen();
	~ScopySplashscreen();

public:
	// singleton
	ScopySplashscreen(ScopySplashscreen &other) = delete;
	void operator=(const ScopySplashscreen &) = delete;
	static ScopySplashscreen *GetInstance();

	static void init();
	static void showMessage(QString s);
	static void setPrefix(QString pre);
	static QString message();
	static void finish(QWidget *w);

public Q_SLOTS:
	void setMessage(QString a);

private:
	static ScopySplashscreen *pinstance_;
	QSplashScreen *m_splash;
	QString m_prefix;
	bool finished;
};
} // namespace scopy

#endif // SCOPYSPLASHSCREEN_H
