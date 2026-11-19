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

#include "application_restarter.h"

#include <QApplication>
#include <QDir>
#include <QProcess>

#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QCoreApplication>
#endif

using namespace scopy;

ApplicationRestarter *ApplicationRestarter::pinstance_{nullptr};

ApplicationRestarter::ApplicationRestarter(const QString &executable)
	: m_executable(executable)
	, m_currentPath(QDir::currentPath())
	, m_restart(false)
{
	pinstance_ = this;
}

ApplicationRestarter *ApplicationRestarter::GetInstance() { return pinstance_; }

void ApplicationRestarter::setArguments(const QStringList &arguments) { m_arguments = arguments; }

QStringList ApplicationRestarter::getArguments() const { return m_arguments; }

int ApplicationRestarter::restart(int exitCode)
{
	if(m_restart) {
#ifdef Q_OS_ANDROID
		// Qt6: QtAndroidExtras is gone. The Android context is the QtActivity, which the
		// bundled ScopyActivity extends; call its restart() method through JNI.
		QJniObject activity = QNativeInterface::QAndroidApplication::context();
		if(activity.isValid()) {
			activity.callMethod<void>("restart");
		}
#else
		QProcess::startDetached(m_executable, m_arguments, m_currentPath);
#endif
	}

	return exitCode;
}

void ApplicationRestarter::triggerRestart()
{
	GetInstance()->m_restart = true;
	qApp->exit();
}
