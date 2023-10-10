#ifndef COMMAND_H
#define COMMAND_H

#include "scopy-iioutil_export.h"

#include <QDebug>
#include <QObject>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT CommandResult
{
public:
	ssize_t errorCode;
	void *results = nullptr;
};

class SCOPY_IIOUTIL_EXPORT Command : public QObject
{
	Q_OBJECT
public:
	virtual ~Command()
	{
		qDebug() << "Command deleted";
		delete m_cmdResult;
		m_cmdResult = nullptr;
	};
	virtual void execute() = 0;
	virtual ssize_t getReturnCode() { return m_cmdResult->errorCode; }
	virtual bool isOverwrite() { return m_overwrite; }
Q_SIGNALS:
	void started(scopy::Command *command = nullptr);
	void finished(scopy::Command *command = nullptr);

protected:
	void setOverwrite(bool overwrite) { m_overwrite = overwrite; }

protected:
	CommandResult *m_cmdResult = nullptr;
	bool m_overwrite = false;
};
} // namespace scopy

Q_DECLARE_METATYPE(scopy::Command *)
#endif // COMMAND_H
