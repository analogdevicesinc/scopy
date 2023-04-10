#include "application_restarter.h"

#include <QApplication>
#include <QProcess>
#include <QDir>

#ifdef __ANDROID__
	#include <QtAndroidExtras/QtAndroid>
#endif

using namespace adiscope;

ApplicationRestarter::ApplicationRestarter(const QString &executable)
        : m_executable(executable)
        , m_currentPath(QDir::currentPath())
{

}

void ApplicationRestarter::setArguments(const QStringList &arguments)
{
        m_arguments = arguments;
}

QStringList ApplicationRestarter::getArguments() const
{
        return m_arguments;
}

int ApplicationRestarter::restart(int exitCode)
{
	if (qApp->property("restart").toBool()) {
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
	qApp->setProperty("restart", QVariant(true));
	qApp->exit();
}
