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

#include "scopysplashscreen.h"
#include <QLoggingCategory>
#include <QApplication>

using namespace scopy;

Q_LOGGING_CATEGORY(CAT_SPLASHSCREEN, "ScopySplashscreen");

ScopySplashscreen *ScopySplashscreen::pinstance_{nullptr};
ScopySplashscreen::ScopySplashscreen()
{
	QPixmap pixmap(":/gui/scopy.png");
	m_splash = new QSplashScreen(pixmap);

	qInfo(CAT_SPLASHSCREEN) << "Created splashscreen";
	m_splash->show();
	m_prefix = "";
	QCoreApplication::processEvents();
	finished = false;
}

ScopySplashscreen::~ScopySplashscreen() {}

void ScopySplashscreen::init() { ScopySplashscreen::GetInstance(); }

ScopySplashscreen *ScopySplashscreen::GetInstance()
{
	if(pinstance_ == nullptr) {
		pinstance_ = new ScopySplashscreen(); // singleton has the app as parent
	}
	return pinstance_;
}

void ScopySplashscreen::showMessage(QString s)
{
	ScopySplashscreen *inst = ScopySplashscreen::GetInstance();
	if(!inst->finished) {
		inst->m_splash->showMessage(inst->m_prefix + s, Qt::AlignBottom | Qt::AlignHCenter, Qt::white);
		QCoreApplication::processEvents();
		qInfo(CAT_SPLASHSCREEN) << "Splashscreen message:" << s;
	} else {
		qWarning(CAT_SPLASHSCREEN) << "Splashscreen finished but message requested: :" << s << "";
	}
}

void ScopySplashscreen::setPrefix(QString pre)
{
	ScopySplashscreen *inst = ScopySplashscreen::GetInstance();
	inst->m_prefix = pre;
}

QString ScopySplashscreen::message() { return ScopySplashscreen::GetInstance()->message(); }

void ScopySplashscreen::finish(QWidget *w)
{
	ScopySplashscreen *inst = ScopySplashscreen::GetInstance();
	inst->m_splash->finish(w);
	qInfo(CAT_SPLASHSCREEN) << "Finished splashscreen";
	inst->finished = true;
}

void ScopySplashscreen::setMessage(QString a) { showMessage(a); }

#include "moc_scopysplashscreen.cpp"
