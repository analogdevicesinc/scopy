#include <QApplication>
#include <QDir>
#include <QProcess>

#include <scopy/core/application_restarter.hpp>

using namespace scopy::core;

ApplicationRestarter::ApplicationRestarter(const QString& executable)
	: m_executable(executable)
	, m_currentPath(QDir::currentPath())
{}

void ApplicationRestarter::setArguments(const QStringList& arguments) { m_arguments = arguments; }

QStringList ApplicationRestarter::getArguments() const { return m_arguments; }

int ApplicationRestarter::restart(int exitCode)
{
	if (qApp->property("restart").toBool()) {
		QProcess::startDetached(m_executable, m_arguments, m_currentPath);
	}

	return exitCode;
}

void ApplicationRestarter::triggerRestart()
{
	qApp->setProperty("restart", QVariant(true));
	qApp->exit();
}
