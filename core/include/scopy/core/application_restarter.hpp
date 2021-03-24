#ifndef APPLICATIONRESTARTER_H
#define APPLICATIONRESTARTER_H

#include <QString>
#include <QStringList>

namespace scopy {
namespace core {

class ApplicationRestarter
{
public:
	ApplicationRestarter(const QString& executable);

	void setArguments(const QStringList& arguments);
	QStringList getArguments() const;

	int restart(int exitCode);
	static void triggerRestart();

private:
	QString m_executable;
	QStringList m_arguments;
	QString m_currentPath;
};
} // namespace core
} // namespace scopy

#endif // APPLICATIONRESTARTER_H
