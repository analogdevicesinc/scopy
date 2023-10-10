#include "application_restarter.h"

#include <QApplication>
#include <QDir>
#include <QProcess>

#ifdef __ANDROID__
#include <QtAndroidExtras/QtAndroid>
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
#ifdef __ANDROID__
		QAndroidJniObject activity = QtAndroid::androidActivity();
		activity.callMethod<void>("restart");
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
