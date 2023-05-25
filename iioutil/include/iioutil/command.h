#ifndef COMMAND_H
#define COMMAND_H

#include "scopy-iioutil_export.h"
#include <QObject>
#include <QDebug>

namespace scopy {
class SCOPY_IIOUTIL_EXPORT CommandResult {
public:
	ssize_t errorCode;
	void *results = nullptr;
};

class SCOPY_IIOUTIL_EXPORT Command : public QObject {
	Q_OBJECT
public:
	virtual ~Command() {
		qDebug() << "Command deleted";
		delete m_cmdResult;
		m_cmdResult = nullptr;

	};
	virtual void execute() = 0;
	virtual ssize_t getReturnCode() {
		return m_cmdResult->errorCode;
	}
Q_SIGNALS:
	void started(scopy::Command *command = nullptr);
	void finished(scopy::Command *command = nullptr);
protected:
	CommandResult *m_cmdResult = nullptr;
};
}

Q_DECLARE_METATYPE(scopy::Command *)
#endif // COMMAND_H
